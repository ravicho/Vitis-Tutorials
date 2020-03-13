﻿
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

# Introduction

The methodology for accelerating applications on FPGA comprises of two phases: architecting the application and developing the accelerator to meet your desired performance goals.

* In the first phase, the developer makes key decisions about the architecture of the application and determines factors, such as what software functions should be mapped to FPGA kernels, how much parallelism is needed, and how it should be delivered.
* In the second phase, the developer implements the kernel by modifying the source code and applying pragmas to create kernel architecture that can achieve the desired performance goals.

You begin this tutorial with a baseline application and profile to examine the potential for hardware acceleration. The tutorial application involves searching an incoming stream of documents to find the documents that closely match a user’s interest based on a search profile using a bloom filter.

In general, this application has use cases in data analytics, such as browsing through unstructured data of emails and text files to identify the documents that closely associate with a specific user and send notifications accordingly.


# Before You Begin

The labs in this tutorial use:

* BASH Linux shell commands.
* 2019.2 Vitis core development kit release and the *xilinx_u200_xdma_201830_2* platform. If necessary, it can be easily ported to other versions and platforms.
* A `Makefile` that is detailed and contains many steps and variables. For a discussion of the `Makefile` structure and contents, refer to [Understanding the Makefile](./HowToRunTutorial.md).

>**IMPORTANT:**  
>
> * Before running any of the examples, make sure you have installed the Vitis core development kit as described in [Installation](https://www.xilinx.com/html_docs/xilinx2019_2/vitis_doc/vhc1571429852245.html).
>* If you run applications on Xilinx® Alveo™ Data Center accelerator cards, ensure the card and software drivers have been correctly installed by following the instructions in the *Getting Started with Alveo Data Center Accelerator Cards Guide* ([UG1301](https://www.xilinx.com/support/documentation/boards_and_kits/accelerator-cards/ug1301-getting-started-guide-alveo-accelerator-cards.pdf)).

## Accessing the Tutorial Reference Files

1. To access the reference files, enter the following in a terminal: `git clone http://github.com/Xilinx/Vitis-Tutorials`.
2. Navigate to the `bloom` directory and then access the `design` directory.

## Tutorial Overview

1. [Brief overview of the Application](1_overview.md): This modules provides the brief overview of Bloom filter application and some examples how this application is being used in real world.
2. [Experience Acceleration performance vs Software-only version](2_experience-acceleration.md): You will profile the Bloom filter application and evaluate which sections are best suited for FPGA acceleration. You will also experience the acceleration potential of by running the application first as a software-only version and then as an optimized FPGA-accelerated version.
3. [Arhitecting the Appplication](3_architect-the-application.md)():In this lab, the original C++ based application computes scores for the documents using a bloom filter. This lab also discusses setting realistic performance goals for an accelerated application. At the end of this lab, you will have specification of kernel created based on Vitis HLS methodology recommendations.
4. [Implementing the Kernel](4_implement-kernel.md): You will implement the kernel based on specification from previous lab and will run the compute part of the algorithm on the FPGA
5. [Data movement between Host and Kernel](5_data-movement.md): You will analyze the performance results of previously generated kernel. Working with a predefined FPGA accelerator, you will experience how to optimize data movements between host and FPGA, how to efficiently invoke the FPGA kernel and how to overlap computation on the CPU and the FPGA to maximize application performance.

<p align="center"><b>
Start the next step: <a href="./2_experience-acceleration.md">2: Experience FPGA Acceleration</a>
</b></p>

</br>
<hr/>
<p align= center><b><a href="/README.md">Return to Main Page</a> — <a href="/docs/vitis-getting-started/">Return to Getting Started Pathway</a></b></p>
</br>
<p align="center"><sup>Copyright&copy; 2019 Xilinx</sup></p>

