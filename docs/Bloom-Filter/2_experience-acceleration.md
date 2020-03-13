<table>
 <tr>
   <td align="center"><img src="https://www.xilinx.com/content/dam/xilinx/imgs/press/media-kits/corporate/xilinx-logo.png" width="30%"/><h1>2019.2 Vitis™ Application Acceleration Development Flow Tutorials</h1>
   <a href="https://github.com/Xilinx/SDAccel-Tutorials/branches/all">See SDAccel™ Development Environment 2019.1 Tutorials</a>
   </td>
 </tr>
 <tr>
 <td align="center"><h1>Optimizing Accelerated FPGA Applications: Bloom Filter Example
 </td>
 </tr>
</table>

# Experiencing Acceleration Peformance

In this step, You will profile the Bloom filter application and evaluate which sections are best suited for FPGA acceleration. You will also experience the acceleration potential by running the application first as a software-only version and then as an optimized FPGA-accelerated version. 

Before we go into details of how to architect the applicationn and recommendations to accelerate the application, let's look at the FPGA acceleration that can be observed by a precomplied FPGA accelerator.

## Build the C Application

Navigate to the `cpu_src` directory and run **make** to generate the executable file

  The command compiles the C source code and builds the `host` executable. The executable requires the number of documents as an input argument.
>**TIP:** The `Makefile` used in this lab is detailed and contains many steps and variables. For a discussion of the structure and contents of the Makefile, refer to [Understanding the Makefile](./HowToRunTutorial.md).

## Run the C Application and Generate the Golden Result

Run the original C application with the number of documents as the argument and generate the golden output files for comparison purposes using the following commands.

   ```
   cd Bloom-Filter/cpu_src/build
   ./host 100000
   ```

The generated output compute scores are stored in the `cpu_profile_score` array in the host code, which represents the outputs for the total number of documents specified.

## Run the Application on the FPGA

For the purposes of this lab, we have implemented the FPGA accelerator with an 8x parallelization factor. It processes 8 input words in parallel, producing 8 output flags in parallel each clock cycle. Each output flag is stored as byte and indicates whether the corresponding word is present in the search array. Since each word requires two calls to the MurmurHash2 function, this means that the accelerator performs 16 hash computations in parallel. In addition, we have optimized the host application to efficiently interact with the parallelized FPGA-accelerator. The result is an application which runs significantly faster, thanks to FPGAs and AWS F1 instances.

1. Run the following make command for running optimized application on FPGA

   ```bash
   make run_fpga
   ```

2. The output is as follows:

   ```
   --------------------------------------------------------------------
    Executed FPGA accelerated version  |   552.5344 ms   ( FPGA 528.744 ms )
    Executed Software-Only version     |   3864.4070 ms
   --------------------------------------------------------------------
    Verification: PASS
   ```

Throughput = Total data/Total time = 1.39 GB/552.534ms = 2.516 GB/s

You can see that by efficiently leveraging FPGA acceleration, the throughput of the application has increased by a factor of 7.  

3. With FPGA acceleration, processing the entire American Library of Congress would take about 1.65 hours (15TB/2.52GB/s), as opposed to 12.3 hours with a software-only approach.


## Next Step

You have identified the functions from the original application that are targets for acceleration and established the performance goals. In the following labs, you will create a baseline of the original bloom filter function running in hardware and perform a series of host and kernel code optimizations to meet your performance goals. You will begin by [creating a Vitis core development kit application](./baseline_fpga.md) from the original application.

You will be using hardware emulation runs for measuring performance in each step. As part of the final step, you can run all these steps in hardware to demonstrate how the performance was improved at each step.

<p align="center"><b>
Start the next step: <a href="./3_architect-the-application.md">2: Architect the Application for FPGA</a>
</b></p>

</br>
<hr/>
<p align="center"><b><a href="/docs/vitis-getting-started/">Return to Getting Started Pathway</a> — <a href="./README.md">Return to Start of Tutorial</a></b></p>

<p align="center"><sup>Copyright&copy; 2019 Xilinx</sup></p>



