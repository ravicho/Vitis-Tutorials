
# Kernel Implementation

Based on previous "Architectint the Application", we have established Throughput Goal of "??", Datawidth = ?? and required BW. 

To achieve BW requirements of above 2GB/sec ??

Refer to the following from "Methodology for Accelerating Applications with the Vitis Software


To Partition the code into Load-Compute-Store pattern, here are the updates to the original algorithm.
- Created runOnfpga top level  function with 512-bit input that can receieve 16 words in parallel and create output flags for each of the words. 
- Original "Hash" function is renamed into function compute_hash_flags. This function is encapsulated in compute_hash_flags_dataflow function along with function that setup the data in and data out of the function to support dataflow. 
- Reads 512-bit values, input_words from global memory over AXI interfaces and creates a stream of 512-bit values. 
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

Now we have the top level function, runOnfpga function updated with proper datawidths and interface type, we must update the compute_hash_flags to process multiple words in parallel. 
- compute_hash_flags receives stream on 32-bit input values from stream, word_stream. 

The input values coming in are 512-bit indicates that we can have 8 hash functions compute 1 word in parallel. You don't need to update the code with manually creating 8 copies of this function. Vitis HLS compiler can perform this functionality by making use of UNROLL pragma. 






<p align="center"><b>
Start the next step: <a href="./data-movement.md"> Data movement between Host and FPGA </a>
</b></p>
