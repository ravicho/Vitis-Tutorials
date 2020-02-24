
# Kernel Implementation

Based on previous "Architecting the Application" module, we have established following Kernel requirements to achieve the acceleration goal.

input Data width for receiving input words = 512 bits
Hash Function pipeline requirements, II=1


Refer to the following from "Methodology for Accelerating Applications with the Vitis Software, Add Snapshot here :


To Partition the code into Load-Compute-Store pattern, here are the updates to the original algorithm.
- Created runOnfpga top level  function with 512-bit input that can receieve 8 words in parallel and create output flags for each of the words. 
- Original "Hash" function is renamed into function compute_hash_flags. This function is encapsulated in compute_hash_flags_dataflow function along with function that setup the data in and data out of the function to support dataflow. 
- Reads 512-bit values, input_words from global memory over AXI interfaces and creates a stream of 512-bit values. 

Now we have the top level function, runOnfpga function updated with proper datawidths and interface type, we must update the compute_hash_flags to process multiple words in parallel. 
- compute_hash_flags receives stream on 32-bit input values from stream, word_stream. 

You don't need to update the code manually to create 8 copies of compute_hash_flags function. Vitis HLS compiler can perform this functionality by making use of UNROLL pragma. 


Let's ensure that with our changes, the application passes SW Emulation by running following command 

``` make run STEP=kernel TARGET=sw_emu SOLUTION=1 ```

We don't need to run the HW Emulation during developing kernel as turnaround time for running HW emulation for all the words could be significantly high. 

However, we can just build the HW to find out if the Kernel successfully was able to create 8 copies of the compute_hash_flags function.

``` make build STEP=kernel TARGET=hw_emu SOLUTION=1 ```

RAVI-> VitisAnalyzer?

Looking at the HLS reports, we can see that DATAFLOW Pragma has been successfully applied for compute_hash_flags_dataflow 

INFO: [XFORM 203-712] Applying dataflow to function 'compute_hash_flags_dataflow', detected/extracted 8 process function(s):

Burst reads must be inferred for compute_hash_flags function for accesssing DDR. The following is confirmation of burst inferring for reading input_doc_words from DDR 0.
INFO: [HLS 214-115] Burst read of variable length and bit width 512 has been inferred on port 'maxiport0' (/wrk/xsjhdnobkup5/ravic/git_ravicho/Vitis-Tutorials/docs/Bloom-Filter/reference_files/hls_stream_utils.h:12:22)
INFO: [HLS 214-115] Burst write of variable length and bit width 512 has been inferred on port 'maxiport0' (/wrk/xsjhdnobkup5/ravic/git_ravicho/Vitis-Tutorials/docs/Bloom-Filter/reference_files/hls_stream_utils.h:22:22)


INFO: [HLS 214-115] Burst read of length 1024 and bit width 512 has been inferred on port 'maxiport1' (/wrk/xsjhdnobkup5/ravic/git_ravicho/Vitis-Tutorials/docs/Bloom-Filter/reference_files/compute_score_fpga.cpp:134:26)

You can also run the application on HW emulation to verify the functionality is intact by running the following command. Please note that the number of input words used are only 100 here as it will take long time to run the HW Emulation. 

``` make run STEP=kernel TARGET=hw_emu SOLUTION=1 
```

The above commands shows that the SIMULATION is PASSED. This ensures that Hardware generated should be functionallity correct.

Let's run the application on Hardware to observe the acutal performance here. 

``` make run STEP=kernel TARGET=hw SOLUTION=1 
```

Use Vitis Analyzer to analyze the timeline trace:

``` vitis_analyzer ../build/kernel/runOnfpga_hw.xclbin.run_summary
```

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
- Add the following pragma in the code to implement the dataflow for the functions. 
```cpp 
# pragma DATAFLOW
```







<p align="center"><b>
Start the next step: <a href="./5_data-movement.md"> 5. Data movement between Host and FPGA </a>
</b></p>
