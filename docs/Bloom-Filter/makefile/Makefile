TARGET := hw

ITER   := 
ifeq ($(STEP), $(filter $(STEP), sw_overlap , multiDDR ))
ITER   := 16
endif

PF     := 8
ifeq ($(STEP), kernel_4)
PF   := 4
endif
ifeq ($(STEP), kernel_16)
PF   := 16
endif

CURRENT_DIR = $(PWD)
PLATFORM :=/proj/xbuilds/2020.1_daily_latest/internal_platforms/xilinx_u200_xdma_201830_2/xilinx_u200_xdma_201830_2.xpfm


SRCDIR := ./../reference_files

# Default Settings here ..
HOST_SRC_CPP := $(SRCDIR)/compute_score_host.cpp
BUILDDIR := ./../build/$(STEP)/kernel_$(PF)/$(TARGET)

ifeq ($(STEP), $(filter $(STEP), single_buffer , split_buffer , generic_buffer , sw_overlap , multiDDR ))
HOST_SRC_FPGA := $(SRCDIR)/compute_score_fpga_kernel.cpp
HOST_SRC_CPP := $(SRCDIR)/compute_score_host.cpp
endif

ifeq ($(STEP), $(filter $(STEP), split_buffer , generic_buffer , sw_overlap ))
NOBUILD=1
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



compile_host:  host
	@echo $(HOST_SRC_FPGA) is being used as source for Generating Kernel; \
	mkdir -p $(BUILDDIR)

ifeq ($(NOBUILD),1)
build: compile_host
	if [ -f ../build/single_buffer/kernel_$(PF)/$(TARGET)/runOnfpga_$(TARGET).xclbin ]; \
  	then \
		echo "Copying the xclbin and host files to $(BUILDDIR) "; \
		cp ../build/single_buffer/kernel_$(PF)/$(TARGET)/runOnfpga_$(TARGET).* $(BUILDDIR);  \
	fi;
	if ! [ -f ../build/single_buffer/kernel_$(PF)/$(TARGET)/runOnfpga_$(TARGET).xclbin ]; \
  	then \
		echo ""; \
		echo "***   Please run STEP=single_buffer first! *** "; \
		echo ""; \
	fi;

else
build: compile_host ./$(BUILDDIR)/runOnfpga_$(TARGET).xclbin
endif


run:   build host
	cp xrt.ini $(BUILDDIR)
ifeq ($(TARGET), hw)
	@echo "************  Use Command Line to run application!  ************"
	cd $(BUILDDIR) && ./host 100000 $(ITER) ;
else
	@echo "Running $(TAGET) mode"; 
	cd $(BUILDDIR) && XCL_EMULATION_MODE=$(TARGET) ./host 1 $(ITER) ;
endif

	 
run_all :
	cd $(CURRENT_DIR); make $(GO) STEP=single_buffer TARGET=$(TARGET) SOLUTION=1 PF=4
	cd $(CURRENT_DIR); make $(GO) STEP=single_buffer TARGET=$(TARGET) SOLUTION=1 PF=8
	cd $(CURRENT_DIR); make $(GO) STEP=single_buffer TARGET=$(TARGET) SOLUTION=1 PF=16
	cd $(CURRENT_DIR); make $(GO) STEP=split_buffer TARGET=$(TARGET) SOLUTION=1 PF=4
	cd $(CURRENT_DIR); make $(GO) STEP=split_buffer TARGET=$(TARGET) SOLUTION=1 PF=8
	cd $(CURRENT_DIR); make $(GO) STEP=split_buffer TARGET=$(TARGET) SOLUTION=1 PF=16
	cd $(CURRENT_DIR); make $(GO) STEP=sw_overlap TARGET=$(TARGET) SOLUTION=1 PF=4
	cd $(CURRENT_DIR); make $(GO) STEP=sw_overlap TARGET=$(TARGET) SOLUTION=1 PF=8
	cd $(CURRENT_DIR); make $(GO) STEP=sw_overlap TARGET=$(TARGET) SOLUTION=1 PF=16
	cd $(CURRENT_DIR); make $(GO) STEP=multiDDR TARGET=$(TARGET) SOLUTION=1 PF=4
	cd $(CURRENT_DIR); make $(GO) STEP=multiDDR TARGET=$(TARGET) SOLUTION=1 PF=8
	cd $(CURRENT_DIR); make $(GO) STEP=multiDDR TARGET=$(TARGET) SOLUTION=1 PF=16

build_all:
	for entry in ${STEPS} ; \
	do 			\
		for pf in 4 8 16 ;	\
		do			\
			echo "Running Command -  make build STEP=$${entry} TARGET=sw_emu SOLUTION=1 PF=$${pf} " ; 	\
			make build STEP=$${entry} TARGET=sw_emu SOLUTION=1 PF=$${pf} ; \
		done					\
	done

help:
	@echo  " "
	@echo  " "
	@echo  " Makefile Usage:"
	@echo  " "
	@echo  "  Run Part 2 : "  - 
	@echo  "     Step 1 : make run STEP=kernel TARGET=hw SOLUTION=1 PF=4/8/16"
	@echo  "     Step 1 : make run STEP=kernel_16 TARGET=hw SOLUTION=1 PF=4/8/16"
	@echo  "     Step 1 : make run STEP=stream TARGET=hw SOLUTION=1 PF=4/8/16"
	@echo  "     Step 1 : make run STEP=single_buffer SOLUTION=1 PF=4/8/16"
	@echo  "     Step 2 : make run STEP=split_buffer SOLUTION=1 PF=4/8/16"
	@echo  "     Step 3 : make run STEP=generic_buffer ITER=16 SOLUTION=1 PF=4/8/16"
	@echo  "     Step 4 : make run STEP=sw_overlap ITER=16 SOLUTION=1 PF=4/8/16"
	@echo  " "
	@echo  "  Generate and View Profile Repprt:"
	@echo  "  sdx_analyze  profile  –f html -i ./profile_summary.csv; firefox ./profile_summary;"
	@echo  " "
	@echo  "  Generate and view timeline Trace:"
	@echo  "  sdx_analyze trace –f wdb -I ./timeline_trace.csv; sdx –workspace workspace –report timeline_trace.wdb"
	@echo  ""
