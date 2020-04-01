
# Kernel Implementation using 8 words in Parallel

Based on previous "Architecting the Application" module, we have established following Kernel requirements to achieve the acceleration goal.

- Input Data width for receiving input words = 512 bits
- Hash Function pipeline requirements, II=1

## Macro Architecture Implementation

The original algorithm used for running on CPU was processing word by word. This algorithm needs to be updated to receive 512 bit of words from DDR. Based on the "Methodology for Accelerating Applications with the Vitis Software, the algorithm needs to be converted to partition the code into Load, Compute and Store blocks. 

 ![](./images/Methodology_HLS_1.PNG)


1. The first step of kernel development methodology requires structuring the kernel code into the Load-Compute-Store pattern. This means creating top level function, runOnfpga with:

    -   Adding Interface Pragmas <- Need to be rmoeved (RAVI)
    -   Added sub-functions in compute_hash_flags_dataflow for Load, Compute and Store.
    -   Local arrays or hls::stream variables to pass data between these functions.

"runOnfpga" top level function is configured with the following arguments 
    - input words of 512-bit input words data.
    - output flags of 512-bit output flags data.
    - bloom_filter for loading coefficients.
    - total number of words to be computed. 

Function "runOnfpga" loads the bloom filter coefficients and calls "compute_hash_flags_dataflow" function which has main functionaly of Load, Compute and Store functions.

- Also added #pragmas for HLS to enable task-level pipelining also known as HLS dataflow. This pragma will instruct the HLS compiler to run all sub-functions of Load-Compute-Store simultaneously, creating a pipeline of concurrently running tasks. 

- For Load part, buffer and resize functions are implemented in "hls_stream_utils.h"
    - Function "buffer" function receives 512-bit input words from memory and creates streams of 512-bit words.
    - Function "resize" creates stream of 512-bit input data to feed function "compute_hash_flags" that requries 8*32-bit intput words = 256 bit input data
- For Compute part, Original "Hash" function is renamed into function "compute_hash_flags". Since we plan to compute 8 words in parallel, this function will require 8*32-bit intput words = 256 bit input data. 
    -   For loop is also restructured to input 512-bit of values and capability to compute parallel words by adding nested loop. This loop can be unrolled completely. Since we require 8 words to be computed in parallel, PARALLISATION is set to 8. 
- For Save part, similar buffer and resize functions are implemented in "hls_stream_utils.h"
    - Function "resize" reads the output flags, output of function "compute_hash_flags" and creates stream of 512-bit data.
    - Function "buffer" function receives 512-bit data from resize functiona and creates a burst write of 512-bit value to send to global memoy over AXI interfaces.

2. Here is implemeted code put together for Load, Compute and Store functons.

    ```cpp
    hls_stream::buffer(data_from_gmem, input_words, total_size/(512/32));
    ```
- Form a stream of parallel words, word_stream (32-bit each) from stream of 512-bit values of flag_stream.

    ```cpp
    hls_stream::resize(word_stream, data_from_gmem, total_size/(512/32));
    ```
- Renamed original function Hash to compute_hash_flags function
    ```cpp
    compute_hash_flags(flag_stream, word_stream, bloom_filter, total_size);
    ```
- Form a stream of 512-bit values, data_to_gmem from stream of 8-bit words in stream of flag_stream.
    ```cpp
    hls_stream::resize(data_to_gmem, flag_stream, total_size/(512/8));
    ```
- Writes 512-bit values to gloabl memory over AXI interfaces.
    ```cpp
    hls_stream::buffer(output_flags, data_to_gmem, total_size/(512/8));
    ```

3. Add the following pragma in the code to implement the dataflow for the functions. 
    ```cpp 
    # pragma DATAFLOW
    ```
This pragma will let function run as indepndent processes and ability to run these in parallel and overlapping fashion. 


## Micro Architecture Implementation

Now we have the top level function, runOnfpga function updated with proper datawidths and interface type, we need to identify the loops for optimizaion for improving latency and throughput. 

1. Function "runOnfpga" has read_bloom_filter nested for loop that reads the bloom filter values and save into bloom_filter_local array. 

For the following code, "#pragma HLS PIPELINE II=1" is added to receive  bloom filter data and make 8 copies to be used for 8 parallel computation in function "compute_hash_flags"

The following code will be executed in about 64k cycles as bloom_filter_size is fixed to 64k and the code is pipelined with initition interval of 1. 

```cpp

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

```

2. Function "compute_hash_flags" for loop is rearchitected into nested loops to input 512-bit of data from DDR and process 8 words of data equal to 256-bits.

You don't need to update the code manually to create 8 copies of compute_hash_flags function. Vitis HLS compiler can perform this functionality by making use of UNROLL pragma. 

For the following code, #pragma HLS LOOP_TRIPCOUNT" is added for reporting purposes. When this function is synthesized, you can determine the latency of this function if this matches the expectations. Since "Compute_hash_flags" function is part of the "#pragma HLS DATAFLOW" so this function should honor initiation interval of 1. 

The inner for loop will have some latency about 10-12 cycles and outer loop is being invoked every cycle. So to match the expectations of the function, expected latency reported should be close to 10000. This way we can be confident that we have achieved the desiered performance of the kernel.


```cpp
void compute_hash_flags (
        hls::stream<parallel_flags_t>& flag_stream,
        hls::stream<parallel_words_t>& word_stream,
        unsigned int                   bloom_filter_local[PARALLELISATION][bloom_filter_size],
        unsigned int                   total_size)
{
  compute_flags: for(int i=0; i<total_size/PARALLELISATION; i++)
  {
    #pragma HLS LOOP_TRIPCOUNT min=1 max=10000
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

```

### Run SW Emulation 

Let's ensure that with our changes, the application passes SW Emulation by running following command 

``` make run STEP=kernel_basic TARGET=sw_emu SOLUTION=1 ```

### Build Kernel using HW Emulation 

We don't need to run the HW Emulation during developing kernel as turnaround time for running HW emulation for all the words could be significantly high. 

However, we can just build the HW to find out if the Kernel successfully was able to create 8 copies of the compute_hash_flags function.

``` make build STEP=kernel TARGET=hw_emu SOLUTION=1 ```

1. Reviewing Syntheis Report results 

RAVI-> VitisAnalyzer?
Vitis HLS generates the vitis_hls.log file available at 
    ../build/kernel_basic/log_dir/runOnfpga_hw/runOnfpga_vitis_hls.log

-   Looking at the HLS reports, we can see that DATAFLOW Pragma has been successfully applied for compute_hash_flags_dataflow 

INFO: [XFORM 203-712] Applying dataflow to function 'compute_hash_flags_dataflow', detected/extracted 8 process function(s):

-   Burst reads must be inferred for compute_hash_flags function for accesssing DDR. The following is confirmation of burst inferring for reading input_doc_words from DDR 0.
INFO: [HLS 214-115] Burst read of variable length and bit width 512 has been inferred on port 'maxiport0' (/wrk/xsjhdnobkup5/ravic/git_ravicho/Vitis-Tutorials/docs/Bloom-Filter/reference_files/hls_stream_utils.h:12:22)
INFO: [HLS 214-115] Burst write of variable length and bit width 512 has been inferred on port 'maxiport0' (/wrk/xsjhdnobkup5/ravic/git_ravicho/Vitis-Tutorials/docs/Bloom-Filter/reference_files/hls_stream_utils.h:22:22)

    INFO: [HLS 214-115] Burst read of length 1024 and bit width 512 has been inferred on port 'maxiport1' (/wrk/xsjhdnobkup5/ravic/git_ravicho/Vitis-Tutorials/docs/Bloom-Filter/reference_files/compute_score_fpga.cpp:134:26)

### Run HW Emulation 

You should run HW emulation to verify the functionality is intact by running the following command. Please note that the number of input words used are only 100 here as it will take long time to run the HW Emulation. 

``` make run STEP=kernel_basic TARGET=hw SOLUTION=1 ```

The above commands shows that the SIMULATION is PASSED. This ensures that Hardware generated should be functionallity correct. But we haven't run the HW on FPGA yet. Let's run the application on hardware to ensure that application can be run on hardware. 

### Run HW on FPGA

Run the following step for executing appliction on HW. We are using 100,000 documents to be computed on the hardware.

``` make run STEP=kernel_basic TARGET=hw SOLUTION=1 ```

```
Loading runOnfpga_hw.xclbin
 Processing 1398.903 MBytes of data
    Running with a single buffer of 1398.903 MBytes for FPGA processing
--------------------------------------------------------------------
 Executed FPGA accelerated version  |   797.8023 ms   ( FPGA 304.580 ms )
 Executed Software-Only version     |   3167.8370 ms
--------------------------------------------------------------------
 Verification: PASS
```

We have also added functionality in ../referemce_files/run_single_buffer.cpp to track total FPGA Time as well as total application time consumed. 
- Total FPGA time includes Host to DDR transfer, Compute on FPGA and DDR to host tranfer. This can be achieved in 304 ms.
- Total time of computing 100k documents is about 797 ms.

### Throughput Acheived - Compute 8 words in Parallel
- Based on the results, Throughput of the Application is 1399MB/797ms = approx 1.75GBs. This is our first attempt to run application on hardware and we have 4x performance results compared to Software Only.

### Resources Utlized 


# Kernel Implementation using 16 words in Parallel

In the previous step, you are reading 512-bit input values from DDR and computing 8 words in parallel that uses only 256-bit input values. As you may have noticed, we can also compute 16 words in parallel to make use of all of 512-bits at the same time. 

We can achieve this by using PARALLELISATION=16 in the code. You can use the following steps to compute 16 workds in parallel.

## Run HW on FPGA 

``` make run STEP=kernel_basic TARGET=hw SOLUTION=1 ```

```
Loading runOnfpga_hw.xclbin
 Processing 1398.903 MBytes of data
    Running with a single buffer of 1398.903 MBytes for FPGA processing
--------------------------------------------------------------------
 Executed FPGA accelerated version  |   724.0923 ms   ( FPGA 268.346 ms )
 Executed Software-Only version     |   3086.9720 ms
--------------------------------------------------------------------
 Verification: PASS

```

- Total FPGA time includes Host to DDR transfer, Compute on FPGA and DDR to host tranfer. This can be achieved in 304 ms.
- As expected computing 16 words in parallel, FPGA Time has reduced from 304 ms to 268 ms. 


### Throughput Acheived - Compute 8 words in Parallel
- Based on the results, Throughput of the Application is 1399MB/797ms = approx 1.75GBs. This is our first attempt to run application on hardware and we have 4x performance results compared to Software Only.

### Resources Utlized 
- Indicate increase in the resources here.

We are going to use 16 words in parallel for computing in the next section since this has better FPGA performance and there are available resources to build the hardware on FPGA.
 
So far, we have focused on building kernel based on recommnended methodology. We will look into some of the host code based optimization in the next section



<p align="center"><b>
Start the next step: <a href="./5_data-movement.md"> 5. Data movement between Host and FPGA </a>
</b></p>

</br>
<hr/>
<p align="center"><b><a href="/docs/vitis-getting-started/">Return to Getting Started Pathway</a> — <a href="./README.md">Return to Start of Tutorial</a></b></p>

<p align="center"><sup>Copyright&copy; 2019 Xilinx</sup></p>
