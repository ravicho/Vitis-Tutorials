#include <iostream>
#include <ctime>
#include <utility>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "hls_stream_utils.h"
#include "sizes.h"

#define PARALLELISATION 8

typedef ap_uint<sizeof(int )*8*PARALLELISATION> parallel_words_t; 
typedef ap_uint<sizeof(char)*8*PARALLELISATION> parallel_flags_t; 

const unsigned int bloom_filter_size = 1<<bloom_size;

unsigned int MurmurHash2(unsigned int key, int len, unsigned int seed)
{
  const unsigned char* data = (const unsigned char *)&key;
  const unsigned int m = 0x5bd1e995;
  unsigned int h = seed ^ len;
  switch(len) {
    case 3: h ^= data[2] << 16;
    case 2: h ^= data[1] << 8;
    case 1: h ^= data[0];
            h *= m;
  };
  h ^= h >> 13;
  h *= m;
  h ^= h >> 15;
  return h;
} 

void compute_hash_flags (
        hls::stream<parallel_flags_t>& flag_stream,
        hls::stream<parallel_words_t>& word_stream,
        unsigned int                   bloom_filter_local[PARALLELISATION][bloom_filter_size],
        unsigned int                   total_size) 
{
  compute_flags: for(int i=0; i<total_size/PARALLELISATION; i++)
  {
    parallel_words_t parallel_entries = word_stream.read();
    parallel_flags_t inh_flags = 0;

    for (unsigned int j=0; j<PARALLELISATION; j++)
    {
#pragma HLS UNROLL

      unsigned int curr_entry = parallel_entries(31+j*32, j*32);
      unsigned int frequency = curr_entry & 0x00ff;
      unsigned int word_id = curr_entry >> 8;
      unsigned hash_pu = MurmurHash2(word_id, 3, 1);
      unsigned hash_lu = MurmurHash2(word_id, 3, 5);
      bool doc_end= (word_id==docTag); 
      unsigned hash1 = hash_pu&hash_bloom; 
      bool inh1 = (!doc_end) && (bloom_filter_local[j][ hash1 >> 5 ] & ( 1 << (hash1 & 0x1f)));
      unsigned hash2=(hash_pu+hash_lu)&hash_bloom;
      bool inh2 = (!doc_end) && (bloom_filter_local[j][ hash2 >> 5 ] & ( 1 << (hash2 & 0x1f)));

      inh_flags(7+j*8, j*8) = (inh1 && inh2) ? 1 : 0;
    }

    flag_stream.write(inh_flags); 
  } 
}

void compute_hash_flags_dataflow(
        ap_uint<512>*   output_flags,
        ap_uint<512>*   input_words,
        unsigned int    bloom_filter[PARALLELISATION][bloom_filter_size],
        unsigned int    total_size)
{
    hls::stream<ap_uint<512> >    data_from_gmem;
    hls::stream<parallel_words_t> word_stream;
    hls::stream<parallel_flags_t> flag_stream;
    hls::stream<ap_uint<512> >    data_to_gmem;
    ap_uint<512> tmp_din;
    ap_uint<512> tmp_dout;

#pragma HLS DATAFLOW

  // Burst read 512-bit values from global memory over AXI interface
  axiToStreamLoop: for(int i=0; i<total_size/(512/32); i++) { 
	data_from_gmem.write( input_words[i] ); 
  }
  // Form a stream of parallel words from stream of 512-bit values
  resizeStreamW2NLoop: for(int i=0; i<total_size/(512/32); i++) {
     for (int j=0; j<(512/256); j++) {
        if (j==0) tmp_din = data_from_gmem.read();
        word_stream.write( tmp_din(255+256*j, 256*j) );
     }
  }

  // Process stream of parallel word : word_stream is of 2k (32*64)
  compute_hash_flags(flag_stream, word_stream, bloom_filter, total_size);

  // Form a stream of 512-bit values from stream of parallel flags
  resizeStreamN2WLoop: for(int i=0; i<total_size/(512/8); i++) {
     for (int j=0; j<(512/64); j++) {
        tmp_dout(63+64*j, 64*j) = flag_stream.read();
        if (j==(512/64-1)) data_to_gmem.write(tmp_dout);
     }
  }

  // Burst write 512-bit values to global memory over AXI interface
  streamToAxiLoop: for(int i=0; i<total_size/(512/8); i++) { 
	output_flags[i] = data_to_gmem.read(); 
  }

}

extern "C" 
{

  void runOnfpga (
          ap_uint<512>*  output_flags,
          ap_uint<512>*  input_words,
          unsigned int*  bloom_filter,
          unsigned int   total_size,
          bool           load_filter)
  {
  #pragma HLS INTERFACE ap_ctrl_chain port=return            bundle=control
  #pragma HLS INTERFACE s_axilite     port=return            bundle=control
  #pragma HLS INTERFACE s_axilite     port=output_flags      bundle=control
  #pragma HLS INTERFACE s_axilite     port=input_words       bundle=control
  #pragma HLS INTERFACE s_axilite     port=bloom_filter      bundle=control
  #pragma HLS INTERFACE s_axilite     port=total_size        bundle=control
  #pragma HLS INTERFACE s_axilite     port=load_filter       bundle=control

  #pragma HLS INTERFACE m_axi         port=output_flags      bundle=maxiport0   offset=slave 
  #pragma HLS INTERFACE m_axi         port=input_words       bundle=maxiport0   offset=slave 
  #pragma HLS INTERFACE m_axi         port=bloom_filter      bundle=maxiport1   offset=slave 

    static unsigned int bloom_filter_local[PARALLELISATION][bloom_filter_size];
  #pragma HLS ARRAY_PARTITION variable=bloom_filter_local complete dim=1
printf("From runOnfpga : Total_size = %d\n", total_size);

    if(load_filter==true) 
    {
      read_bloom_filter: for(int index=0; index<bloom_filter_size; index++) {
  #pragma HLS PIPELINE II=1
        unsigned int tmp = bloom_filter[index];
        for (int j=0; j<PARALLELISATION; j++) {
          bloom_filter_local[j][index] = tmp;
        }
      }
    }

    compute_hash_flags_dataflow(
      output_flags,
      input_words,
      bloom_filter_local,
      total_size);
  }
}
