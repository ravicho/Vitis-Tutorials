#pragma once

#include<ap_int.h>
#include<hls_stream.h>

namespace hls_stream 
{
  template<typename T>
  void buffer(hls::stream<T> &stream_o, T* axi_i, unsigned int nvals)
  {
//std::cout << "Axi2Stream: "<< std::endl;
    axiToStreamLoop: for(int i=0; i<nvals; i++) {
//std::cout << "Axi2Stream: "<< i << std::endl;
      stream_o.write( axi_i[i] );
    }  
  }

  template<typename T>
  void buffer(T* axi_o, hls::stream<T> &stream_i, unsigned int nvals)
  {
//std::cout << "Stream2Axi: "<< std::endl;
    streamToAxiLoop: for(int i=0; i<nvals; i++) {
//std::cout << "Stream2Axi: "<< i << std::endl;
      axi_o[i] = stream_i.read();
    }  
  }

  template<int Wo, int Wi>
  void resize(hls::stream<ap_uint<Wo> > &stream_o, hls::stream<ap_uint<Wi> > &stream_i, unsigned int nvals)
  {
//std::cout << "int Size" << sizeof(int) << std::endl;
//std::cout << "Unsigned int Size" << sizeof(unsigned int) << std::endl;
    if (Wo<Wi) { // Wide to Narrow
//std::cout << "Calling Resize 1: "<< Wi << " "  << Wo << std::endl;
      ap_uint<Wi> tmp;
      int nwrites = Wi/Wo;
      int nreads  = nvals;
      resizeStreamW2NLoop: for(int i=0; i<nreads; i++) {
//std::cout << "Reads: " << i << std::endl;
        for (int j=0; j<nwrites; j++) {
//std::cout << "Writes: " << j << std::endl;
          if (j==0) tmp = stream_i.read();
          stream_o.write( tmp(Wo-1+Wo*j, Wo*j) );
        }
      }
    }

    if (Wo>Wi) { // Narrow to Wide
//std::cout << "Calling Resize 2: "<< Wi << " "  << Wo << std::endl;
      ap_uint<Wo> tmp;
      int nwrites = nvals;
      int nreads  = Wo/Wi;

      resizeStreamN2WLoop: for(int i=0; i<nwrites; i++) {
//std::cout << ">>Writes: " << i << std::endl;
        for (int j=0; j<nreads; j++) {
//std::cout << ">>Reads: " << j << std::endl;
          tmp(Wi-1+Wi*j, Wi*j) = stream_i.read();
          if (j==(Wo/Wi-1)) stream_o.write(tmp);
        }
      }
    }

    if (Wo==Wi) { // Equal sizes
      resizeStreamEqLoop: for(int i=0; i<nvals; i++) {
        stream_o.write( stream_i.read() );      
      }
    }
  }
}
