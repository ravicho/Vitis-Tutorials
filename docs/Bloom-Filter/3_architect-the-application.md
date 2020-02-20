# Methodology for Architecting a Device Accelerated Application
  
## Establish a Baseline Application Performance and Establish Goals
The algorithm can be divided in two sections:  
* Compute the hash function : This is performed on the number of the input words and creates output flags for each word.
* Compute document score : Uses the output flags from previous hash function and creates the score of each document.

Under Bloom-filter/cpu_src, main funcion calls runOnCPU function. This function is implemented in Bloom-filter/cpu_src/compute_score_host.cpp file.

### Measure Running Time

1. Navigate to the `cpu_src` directory and run the following command.

    ```bash 
    cd ~/SDAccel-AWS-F1-Developer-Labs/modules/module_02/cpu_src
    make run
    ```

2. The output is as follows.
    ```
    Total execution time of CPU                        |  4112.5895 ms
    Compute Hash & Output Flags processing time        |  3660.4433 ms
    Compute Score processing time                      |   452.1462 ms
    --------------------------------------------------------------------
    ```

The above command computes the score for 100,000 documents, amounting to 1.39 GBytes of data. The execution time is 4.112 seconds and throughput is computed as follows:

Throughput = Total data/Total time = 1.39 GB/4.112s = 338 MB/s

3. It is estimated that in 2012, all the data in the American Library of Congress amounted to 15 TB. Based on the above numbers, we can estimate that run processing the entire American Library of Congress on the host CPU would take about 12.3 hours (15TB / 338MB/s).

### Profiling the Application

To improve the performance, you need to identify bottlenecks where the application is spending the majority of the total running time.

As we can see from the execution times in the previous step, the applications spends 89% of time in calculating the hash function and 11 % of the time in computing the document score.

Because you only have the function `runOnCPU` to accelerate, you will run the executable and evaluate the execution time of the function. We also need to recompile the host with necessary flags to enable for gprof. 

1. Recompile the host and extract the profile result.

   ```
   make host_gprof; 
   gprof host gmon.out> gprofresult.txt
   ```

2. To view the gprof report, open the `gprofresult.txt` file in a text editor. You should see results similar to the following table.

   Each sample counts as 0.01 seconds.

   The flat profile report of the individual sub-functions is as follows.

   | % Time | Cumulative Seconds | Self Seconds | Total Calls  | ms/Call  | ms/Call  | Name                         |  
   |--------:|-----------:|----------:|----------:|----------:|----------:|:------------------------------|  
   | 43.19  |     7.55  |   7.55   |  699484226   |  0.00  |  0.00   | MurmurHash2                 |
   | 28.30  |     12.49 |   4.94   |    1   |   4.94   |   12.49 | runOnCPU              |
   | 17.81 | 15.60 | 3.11 | 1 | 3.11 | 4.99 | setupData |

   You can see that the application spends almost half of time in the sub-function `MurmurHash2`, which computes the hash values of words in each document. The `MurmurHash2` sub-function is called by `runOnCPU` and `setupData` functions as part of the `main` function.

   The following table shows a high-level view of the functions inside the `main` function as part of the call graph for the previous report `gprofresults.txt`.

    | % Time |         Self              |  Children | Called | Name
   |--------:|-----------:|----------:|----------:|:------------|
   | 72.9 | 4.54 | 8.76 | 1 | runOnCPU |
   | 26.8  | 3.85 | 1.03 | 1 | setupData |

   From a `main` function standpoint, you can see that the CPU spends almost 73% of time in the `runOnCPU` function. In the `runOnCPU` function, there is only one child call, which is the `MurmurHash2` function. Based on execution times from table, you can deduce that around 66% **((8.76/(8.76+4.54))** of time is spent by calls to `MurmurHash2`. Therefore, accelerating the `runOnCPU` function would significantly increase the performance of the application.


## Create budget for Computation on Device

For creating budget of the kernels, let's say we want to use 10k docs with approx 3500 words in each doc. This results into 35M words. Each word is 4Bytes which is equivalent to 35M * 4B = 140 MB. 
Each "RAVI" function creates flag of unsigned char which results into 35M * 1B = 35 MB.  
We also need to take into account for moving the data from Host to Device and Device to Host back.

s/w Version is about 358ms which is equivalent to 140 MB/ 358ms = 388MBps 

Let's say we want to accelerate this Applications to 2GBps. To achieve this, all 35M words needs to be processed in 140MB/2GBps = 140MB/2000MBps = 0.07s or 70 ms. 

For 70ms, we have to budget for hash compute and profile score calculation. Assuming, both functions take about half the time. Then we must compute hash function within 35 ms. 

The whole application time should be split and budgeted conceptually based on following
1. Transferring document data of size 140MB from Host to device DDR using PCIe
2. Compute the Hash and craete the flags
3. Transferring flags data of size 35MB from Device to Host using PCIe.

Run PCIe BW : 11 GB/sec

For 1, Using PCIe BW of 11GBps, approximate time for transfer = 140MB/11G = 12ms
For 3, Using PCIe BW of 11GBps, approximate time for transfer = 35MB/11G = 3ms

This leaves budget of 35ms - 12ms - 3ms = 20ms for Kernel Computation. 

In 20ms, we need to compute 35MB words. Using 300MHz, there are 6M Cycles in 20 ms. If one word is processed in every cycle, then we will need 35M cycles at the best resulting into 35M/300MHz = approx 120ms. To compute processing of 35M in 6M cycles, we will need to process at least 6 words in parallel at the best. 

If we could create kernel to process say 8 words in parallel, then we can be confident to achieve finally performance of 2GBps. 




## Identify Functions to Accelerate on FPGA

The algorithm can be divided into two sections
* Computing output flags created from the hash function of every word in all documents
* Computing document score based on output flags created above
   
Let's evaluate which of these sections are a good fit for FPGA.

### Evaluating the Hash Function 

1. Open `MurmurHash2.c` file with a file editor.

2. The `MurmurHash2` hash function code is as follows:

```cpp
unsigned int MurmurHash2 ( const void * key, int len, unsigned int seed )
{
  // 'm' and 'r' are mixing constants generated offline.
  // They're not really 'magic', they just happen to work well.

  const unsigned int m = 0x5bd1e995;
  //	const int r = 24;

  // Initialize the hash to a 'random' value
  unsigned int h = seed ^ len;

  // Mix 4 bytes at a time into the hash
  const unsigned char * data = (const unsigned char *)key;

  switch(len)
  {
    case 3: h ^= data[2] << 16;
    case 2: h ^= data[1] << 8;
    case 1: h ^= data[0];
    h *= m;
  };

  // Do a few final mixes of the hash to ensure the last few
  // bytes are well-incorporated.
  h ^= h >> 13;
  h *= m;
  h ^= h >> 15;
  
  return h;
}   
```

**Computational Complexity** is the number of basic computing operations requried to execute the funciton. From the above code, 

* The compute of hash for a single word ID consists of four XOR, 3 arithmetic shifts, and two multiplication operations.
* A shift of 1-bit in an arithmetic shift operation takes one clock cycle on the CPU. 

The three arithmetic operations shift a total of 44-bits (when`len=3` in the above code) to compute the hash which requires 44 clock cycles just to shift the bits on the host CPU. On the FPGA, it is possible to create custom architectures and therefore create an accelerator that will shift data by an arbitrary number of bits in a single clock cycle.

* The FPGA also has dedicated DSP units, which perform multiplications faster than the CPU. Even though the CPU runs at a frequency 8 times higher than the FPGA, the arithmetic shift and multiplication operations can perform faster on the FPGA because of its customizable hardware architecture.







* Therefore this function is a good candidate for FPGA acceleration.

3. Close the file.

### Evaluating the "Compute Output Flags from Hash" code section
### Identify Acceleration Potential of Hash Function
1. Open `compute_score_host.cpp` file in the file editor. 

2. The code at lines 32-58 which computes output flags is shown below.

```cpp
// Compute output flags based on hash function output for the words in all documents
for(unsigned int doc=0;doc<total_num_docs;doc++) 
{
  profile_score[doc] = 0.0;
  unsigned int size = doc_sizes[doc];
  
  for (unsigned i = 0; i < size ; i++)
  { 
    unsigned curr_entry = input_doc_words[size_offset+i];
    unsigned word_id = curr_entry >> 8;
    unsigned hash_pu =  MurmurHash2( &word_id , 3,1);
    unsigned hash_lu =  MurmurHash2( &word_id , 3,5);
    bool doc_end = (word_id==docTag);
    unsigned hash1 = hash_pu&hash_bloom;
    bool inh1 = (!doc_end) && (bloom_filter[ hash1 >> 5 ] & ( 1 << (hash1 & 0x1f)));
    unsigned hash2 = (hash_pu+hash_lu)&hash_bloom;
    bool inh2 = (!doc_end) && (bloom_filter[ hash2 >> 5 ] & ( 1 << (hash2 & 0x1f)));
    
    if (inh1 && inh2) {
      inh_flags[size_offset+i]=1;
    } else {
      inh_flags[size_offset+i]=0;
    }
  }
  
  size_offset+=size;
}

```

* From the above code, we see that we are computing two hash outputs for each word in all the documents and creating output flags accordingly.

* We already determined that the Hash function(MurmurHash2()) is a good candidate for acceleration on FPGA.

* Computation of the hash (`MurmurHash2()`) of one word is independent of other words and can be done in parallel thereby improving the execution time.

* The algorithm makes sequential access to the `input_doc_words` array. This is an important property as it allows very efficient accesses to DDR when implemented in the FPGA.  



 4. Close the file. 
 
This code section is a a good candidate for FPGA as the hash function can run faster on FPGA and we can compute hashes for multiple words in parallel by reading multiple words from DDR in burst mode. 




### ### Identify Acceleration Potential of "Compute Document Score" function

The code for computing the document score is shown below:

```cpp
for(unsigned int doc=0, n=0; doc<total_num_docs;doc++)
{
  profile_score[doc] = 0.0;
  unsigned int size = doc_sizes[doc];

  for (unsigned i = 0; i < size ; i++,n++)
  {
    if (inh_flags[n])
    {
      unsigned curr_entry = input_doc_words[n];
      unsigned frequency = curr_entry & 0x00ff;
      unsigned word_id = curr_entry >> 8;
      profile_score[doc]+= profile_weights[word_id] * (unsigned long)frequency;
    }
  }
}
```

* You can see that the compute score requires one memory access to `profile_weights`, one accumulation and one multiplication operation.

* The memory accesses are random, since they depend on the word ID and therefore the content of each document. 

* The size of the `profile_weights` array is 128 MB and has to be stored in DDR memory connected to the FPGA. Non-sequential accesses to DDR are big performance bottlenecks. Since accesses to the `profile_weights` array are random, implementing this function on the FPGA wouldn't provide much performance benefit, And since this function takes only about 11% of the total running time, we can keep this function on the host CPU. 

Based on this analysis, it is only beneficial to accelerate the "Compute Output Flags from Hash" section on the FPGA. Execution of the "Compute Document Score" section can be kept on the host CPU.



## Identify Device Parallelization Needs

Now that we have analyzed that "Hash" function has potential of accleration on FPGA and overall acceleration goal of (??) has been established, we can also determine what level of parallelization is needed to meet the goals. We need to determine the throughput of hardware function without parallization

Running Original s/w run in cpu_run directory, 
    Bloom-Filter/cpu_run
    make run NUM_DOCS=10000    produces following 
    
     Total execution time of CPU          |   345.8297 ms
     Compute Hash processing time         |   308.2708 ms
     Compute Score processing time        |    37.5588 ms
 
Software Version is about 345ms which is equivalent to 140 MB/ 345ms = approx 400MBps 

We have decided to keep only Compute Hash function in FPGA. This function takes about 308ms in Sotware. 

Let's say we want to accelerate this Applications to 2GBps. To achieve this, all 35M words needs to be processed in 140MB/2GBps = 140MB/2000MBps = 0.07s or 70 ms. 

For 70ms, we have to budget for hash compute and profile score calculation. Since we decided to keep the Compute Score function on software side, it will take about 37ms like during pure software run. This leaves about 70ms - 37ms = approx 33ms time for everything else. 

The whole application time should be split and budgeted based on following
1. Transferring document data of size 140MB from Host to device DDR using PCIe
2. Compute the Hash and craete the flags
3. Transferring flags data of size 35MB from Device to Host using PCIe.

Run PCIe BW : 11 GB/sec

For 1, Using PCIe BW of 11GBps, approximate time for transfer = 140MB/11G = 12ms
For 3, Using PCIe BW of 11GBps, approximate time for transfer = 35MB/11G = 3ms

This leaves budget of 33ms - 12ms - 3ms = 18ms for Kernel Computation. This is equivalent of 140MB/18ms, about 8GBps. Thus Tgoal = 8GBps 

In 20ms, we need to compute 35MB words. Using 300MHz, there are 5.4M Cycles in 18 ms. If one word is processed in every cycle, then we will need 35M cycles at the best resulting into 35M/300MHz = approx 120ms. To compute processing of 35M words in 5.4M cycles, we will need to process at least 7 words in parallel at the best. 

If we could create kernel to process say 8 words in parallel, then we can be confident to achieve finally performance of 2GBps. 




### Estimate Hardware Throughput without Parallelization

The throughput achievable from kernel can be approximated as:

**Thw = (Frequency / Computational Intensity) = (Frequency * max(VINPUT, VOUTPUT) / VOPS)**

where:

* **VINPUT**, **VOUTPUT** represent the volume of input and output data processed respectively.
* **VOPS** represents the volume of operations processed on the input and output data.
* **Computational Intensity** of a function is the ratio of the total number of operations to the maximum amount of input and output data.  

**Thw = (Frequency\*1)samples**

Because each sample is 4 bytes of data and computational intensity is 1, the maximum throughput of kernel is: 
**Thw = (300MHz)\*4B = 1.2GB/s**.

Each word in "Hash" function can be computed in parallel so muliple words can be computed in parallel to improve the acceleration.


### Determine How Much Parallelism is Needed

Throuput potential for hash function computing one word is 1.2GB/sec. The following function can determine how much of the parallelism needed to achieve the performance goal. 

Speed-Up = Thw/Tsw = Fmax * Running Time/Vops 

Speed-Up = 1.2GBps/308Mps = approx 4 times

For Hash function, this parallization can be achieved in either by widening the datapath or by using multiple kernel instances. 

### Determine How Many Samples the Datapath Should be Processing in Parallel

Budget for computing Hash function is 18ms. This is equivalent of 140MB/18ms, about 8GBps. Thus Tgoal = 8GBps 

Tgoal = 8Gpps

##  Identify Software Application Parallelization Needs

### Minimize CPU Idle Time While the Device Kernels are Running

### Keep the Device Kernels Utilized

### Optimize Data Transfers to and from the Device


### Computational Intensity 

## Architectural spec for Kenel, TArget Performance, Interface Widths, Datapath Widths etc. (TODO)

## Conclusion

In this lab, you have seen how to profile an application and determine which parts are best suited for FPGA acceleration. We should have good understanding of architectural spec for Kenel, Target Performance, Interface Widths etc. 

In the next section, you will use "Methodology for Developing C/C++ Kernels" to create optimized kernel to meet the requirements of the Kenel spec.

---------------------------------------

<p align="center"><b>
Start the next step: <a href="./implement-kernel.md">2: Implement the Kernel</a>
</b></p>
