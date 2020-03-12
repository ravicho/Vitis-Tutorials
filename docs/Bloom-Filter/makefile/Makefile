TARGET := hw

ITER   := 
ifeq ($(STEP), sw_overlap)
ITER   := 16
endif
PF     := 8
CURRENT_DIR = $(PWD)



ifeq ($(SOLUTION),1)
	SRCDIR := ./../reference_files
else
	SRCDIR := ./../src/$(STEP)
endif

# Default Settings here ..
HOST_SRC_CPP := $(SRCDIR)/compute_score_host.cpp
BUILDDIR := ./../build/$(STEP)

ifeq ($(STEP), $(filter $(STEP), single_buffer , split_buffer , generic_buffer , sw_overlap ))
HOST_SRC_FPGA := $(SRCDIR)/compute_score_fpga_kernel.cpp
BUILDDIR := ./dataMovement
endif

ifeq ($(STEP), $(filter $(STEP), kernel , kernel_16 , stream , multiDDR ))
HOST_SRC_FPGA := $(SRCDIR)/compute_score_fpga_$(STEP).cpp
endif

ifeq ($(STEP), $(filter $(STEP), localbuf ))
BUILDDIR := ./../build/kernel
endif

# Common Source Code for Host
HOST_SRC_CPP += $(SRCDIR)/MurmurHash2.c
HOST_SRC_CPP += $(SRCDIR)/xcl2.cpp
HOST_SRC_CPP += $(SRCDIR)/main.cpp 

HOST_SRC_RUN_CPP = $(SRCDIR)/run_single_buffer.cpp
ifeq ($(STEP),multiDDR)
HOST_SRC_RUN_CPP = $(SRCDIR)/run_sw_overlap_multiDDR.cpp
endif
ifeq ($(STEP),split_buffer)
HOST_SRC_RUN_CPP = $(SRCDIR)/run_split_buffer.cpp
endif
ifeq ($(STEP),generic_buffer)
HOST_SRC_RUN_CPP = $(SRCDIR)/run_generic_buffer.cpp
endif
ifeq ($(STEP),sw_overlap)
HOST_SRC_RUN_CPP = $(SRCDIR)/run_sw_overlap.cpp
endif

HOST_SRC_CPP += $(HOST_SRC_RUN_CPP)
include common.mk

setup:
	if ! [ -f temp.txt ]; \
  	then \
		touch temp.txt; \
		sudo -E -- bash -c 'source ~/aws-fpga/sdaccel_setup.sh'; \
	fi;



build: host

#run: build 
run: build ./$(BUILDDIR)/runOnfpga_$(TARGET).xclbin
	mkdir -p $(BUILDDIR)
	if [ -f runOnfpga_$(TARGET).xclbin ]; \
	then mv runOnfpga_$(TARGET).* $(BUILDDIR) ; \
	fi
	cp xrt.ini $(BUILDDIR)
ifeq ($(TARGET), hw)
	echo "************  Use Command Line to run application!  ************"
	cd $(BUILDDIR) && ./host 100000 $(ITER) ;
else
	echo "Running $(TAGET) mode"; 
	cd $(BUILDDIR) && XCL_EMULATION_MODE=$(TARGET) ./host 100 $(ITER) ;
endif

	 
all :
	cd $(CURRENT_DIR); make run STEP=kernel TARGET=hw SOLUTION=1
	cd $(CURRENT_DIR); make run STEP=kernel_16 TARGET=hw SOLUTION=1
	cd $(CURRENT_DIR); make run STEP=single_buffer SOLUTION=1
	cd $(CURRENT_DIR); make run STEP=split_buffer SOLUTION=1
	cd $(CURRENT_DIR); make run STEP=generic_buffer ITER=16 SOLUTION=1
	cd $(CURRENT_DIR); make run STEP=sw_overlap ITER=16 SOLUTION=1
	#cd $(CURRENT_DIR); make run STEP=stream TARGET=hw SOLUTION=1

help:
	@echo  " "
	@echo  " "
	@echo  " Makefile Usage:"
	@echo  " "
	@echo  "  Run Part 2 : "  - 
	@echo  "     Step 1 : make run STEP=kernel TARGET=hw SOLUTION=1"
	@echo  "     Step 1 : make run STEP=kernel_16 TARGET=hw SOLUTION=1"
	@echo  "     Step 1 : make run STEP=stream TARGET=hw SOLUTION=1"
	@echo  "     Step 1 : make run STEP=single_buffer SOLUTION=1"
	@echo  "     Step 2 : make run STEP=split_buffer SOLUTION=1"
	@echo  "     Step 3 : make run STEP=generic_buffer ITER=16 SOLUTION=1"
	@echo  "     Step 4 : make run STEP=sw_overlap ITER=16 SOLUTION=1"
	@echo  " "
	@echo  "  Generate and View Profile Repprt:"
	@echo  "  sdx_analyze  profile  –f html -i ./profile_summary.csv; firefox ./profile_summary;"
	@echo  " "
	@echo  "  Generate and view timeline Trace:"
	@echo  "  sdx_analyze trace –f wdb -I ./timeline_trace.csv; sdx –workspace workspace –report timeline_trace.wdb"
	@echo  ""
