
<table>
 <tr>
   <td align="center"><img src="https://www.xilinx.com/content/dam/xilinx/imgs/press/media-kits/corporate/xilinx-logo.png" width="30%"/><h1>2019.2 Vitis™ Application Acceleration Development Flow Tutorials</h1>
   <a href="https://github.com/Xilinx/SDAccel-Tutorials/branches/all">See other versions</a>
   </td>
 </tr>
 <tr>
 <td align="center"><h1>Methodology for Optimizing Accelerated FPGA Applications
 </td>
 </tr>
</table>

# 7. Running the Accelerator in Hardware

Until now, the results of all the previous labs have been run in hardware emulation mode to give you an idea of how the optimization improves performance, while reducing the compilation time needed to build the system. In this section, you will build and run each of the previous optimizations in hardware on an Alveo Data Center accelerator card.

After each run is finished, record the performance data printed out by host application, and fill in the table at the end of the section. Your numbers might vary.  
Note the following data:

* **Total Data**: Calculated by (frame number x frame size).
* **Total Time**: Measured in hardware Timeline Trace report. For a fair comparison, this will include the data transfer and kernel execution time.
* **Throughput**: Calculated by Total Data Processed(MB)/Total Time(s)

>**IMPORTANT**: Each of the steps in this lab compiles the hardware kernel and can take significant time to complete.

## Run the Baseline Application on Hardware

>**IMPORTANT**: Make sure the `nk` option in design.cfg was set to 1.

Use the following command to build and run the baseline design on hardware. At the end of execution, host application will print out the kernel execution time and throughput number. The numbers will be filled into the table for benchmarking comparison.

```
make run TARGET=hw STEP=baseline SOLUTION=1
```

The printed out messages will be something like:

```
FPGA Time:       701.888 s
FPGA Throughput: 1.48762 MB/s


Processed 7.91 MB in 703.580s (1.48 MBps)

```

## Run the Memory Transfer Lab on Hardware

>**IMPORTANT**: Make sure the `nk` option in design.cfg was set to 1.

Use the following commands to run the localbuf version design on hardware.

```
make run TARGET=hw STEP=localbuf SOLUTION=1
```

From the performance number printed out by host application, the execution time was improved a lot.
```
FPGA Time:       86.7961 s
FPGA Throughput: 12.0298 MB/s


Processed 7.91 MB in 91.883s (11.36 MBps)

````

## Run Fixed Point Lab on Hardware

>**IMPORTANT**: Make sure the `nk` option in design.cfg was set to 1.

Build and run the fixedpoint version design on hardware and record the performance number.

```
make run TARGET=hw STEP=fixedpoint SOLUTION=1
```

```
FPGA Time:       27.2872 s
FPGA Throughput: 38.2649 MB/s


Processed 7.91 MB in 32.290s (32.34 MBps)

```

## Run Dataflow Lab on Hardware

>**IMPORTANT**: Make sure the `nk` option in design.cfg was set to 1.

Build and run the dataflow version design on hardware and record the performance number.

```
make run TARGET=hw STEP=dataflow SOLUTION=1
```

```
FPGA Time:       3.66991 s
FPGA Throughput: 284.514 MB/s


Processed 7.91 MB in 7.996s (130.58 MBps)

```

### Run Multiple Compute Units Lab on Hardware

Before building the hardware target, you need to set the CU number as 4. To do that, open the `design.cfg` and modify the `nk` option as below:

```
nk=convolve_fpga:4
```

Use the following commands to run the multiple compute units version design on hardware.

```
make run TARGET=hw STEP=multicu SOLUTION=1
```

You should see similar numbers when host program finishes running.

```
FPGA Time:       2.35359 s
FPGA Throughput: 443.637 MB/s


Processed 7.91 MB in 7.246s (144.10 MBps)

```

### Performance Table

The final performance benchmarking table displays as follows.

| Step                            | Image Size   | Number of Frames  | Time (Hardware) (s) | Throughput (MBps) |
| :-----------------------        | :----------- | ------------: | ------------------: | ----------------: |
| baseline                        |     1920x1080 |           132 |              701.8 | 1.48              |
| localbuf                        |     1920x1080 |           132 |                86.8 | 12 (8.1x)         |
| fixed-point data                |     1920x1080 |           132 |                27.3 | 38.2 (3.2x)        |
| dataflow                        |     1920x1080 |           132 |                3.6 | 284 (7.4x)        |
| multi-CU                        |     1920x1080 |           132 |                2.35 | 443 (1.5x)       |

---------------------------------------

## Conclusion

Congratulations! You have successfully completed all the modules of this lab to convert a standard CPU-based application into an FPGA accelerated application, running with nearly 300X the throughput when running on the Alveo U200 Data Center accelerator card. You set performance objectives, and then you employed a series of optimizations to achieve your objectives.

1. You created a Vitis core development kit application from a basic C application.
1. You familiarized yourself with the reports generated during software and hardware emulation.
1. You explored various methods of optimizing your HLS kernel.
1. You learned how to set an OpenCL API command queue to execute out-of-order for improved performance.
1. You enabled your kernel to run on multiple CUs.
1. You used the HLS dataflow directive and explored how it affected your application.
1. You ran the optimized application on the Alveo Data Center accelerator card to see the actual performance gains.

</br>
<hr/>
<p align="center"><b><a href="/docs/vitis-getting-started/">Return to Getting Started Pathway</a> — <a href="./README.md">Return to Start of Tutorial</a></b></p>

<p align="center"><sup>Copyright&copy; 2019 Xilinx</sup></p>
