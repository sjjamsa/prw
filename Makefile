SHELL := /bin/bash


# for MI100
CFLAGS_CPU_CLANG_AOMP=-O3 -fopenmp=libiomp5 -L/opt/rocm/llvm/lib/ -Wl,-rpath,$(LIB_OMP_DIR)
CFLAGS_GPU_CLANG_AOMP=-O3 -target $(AOMP_CPUTARGET) -fopenmp -fopenmp-version=50 -fopenmp-targets=$(AOMP_GPUTARGET) -Xopenmp-target=$(AOMP_GPUTARGET) -march=$(AOMP_GPU) -Wall
AOMP_GPUTARGET=amdgcn-amd-amdhsa
AOMP_CPUTARGET=x86_64-pc-linux-gnu
AOMP_GPU=gfx908
LLVM_DIR=/opt/rocm/llvm
LIB_OMP_DIR=$(LLVM_DIR)/lib/
#CC=$(LLVM_DIR)/bin/clang





#For Nvidia on triton
CFLAGS_GPU_GCC=-fno-stack-protector -foffload=nvptx-none="-misa=sm_35" -fopenmp -O3
CFLAGS_GPU_NVC=-mp -target=gpu -gpu=cc35 -fopenmp -Minfo
CFLAGS_CPU_CLANG=-fopenmp=libiomp5

CFLAGS_CPU_GCC=-fno-stack-protector -fopenmp -foffload=disable  -O3
#CFLAGS_CPU_GCC=-fno-stack-protector -O3
CFLAGS_CPU_NVC=-fopenmp
CFLAGS_GPU_CLANG=-fopenmp=libiomp5 -fopenmp-targets=nvptx64-nvidia-cuda  -Xopenmp-target -march=$(GPU_ARCH)

#K80
#GPU_ARCH=sm_37 
#P100  
GPU_ARCH=sm_60 
#V100
#GPU_ARCH=sm_70 


# on triton
# module load gcc/9.2.0-cuda-nvptx
#CC=gcc

#CC=nvc

# on triton
# module purge; module load llvm/11.0.1-cuda-python3 
CC=clang







ifeq ($(CC),clang)
	CFLAGS_GPU=$(CFLAGS_GPU_CLANG)
	CFLAGS_CPU=$(CFLAGS_CPU_CLANG)
endif
ifeq ($(CC),gcc)
	CFLAGS_GPU=$(CFLAGS_GPU_GCC)
	CFLAGS_CPU=$(CFLAGS_CPU_GCC)
endif
ifeq ($(CC),nvcc)
	CFLAGS_GPU=$(CFLAGS_GPU_NVCC)
	CFLAGS_CPU=$(CFLAGS_CPU_NVCC)
endif


runWalkBoth : parallel_random_walk parallel_random_walk.cpu
	time ./parallel_random_walk
	time ./parallel_random_walk.cpu

runWalk : parallel_random_walk 
	time ./parallel_random_walk

runWalk.cpu : parallel_random_walk.cpu 
	time ./parallel_random_walk.cpu

%.o : %.c
	$(CC) -c -o $@ $<  $(CFLAGS_GPU)

parallel_random_walk.o pcg.o : pcg.h Makefile

parallel_random_walk     : parallel_random_walk.o pcg.o 
	$(CC) -o $@ $^  $(CFLAGS_GPU)

parallel_random_walk.cpu : parallel_random_walk.c pcg.c pcg.h Makefile
	$(CC) -o $@ parallel_random_walk.c pcg.c  $(CFLAGS_CPU)


scalingTest: parallel_random_walk parallel_random_walk.cpu
	time ./parallel_random_walk     1024     > gpu.1024.txt     
	time ./parallel_random_walk.cpu 1024     > cpu.1024.txt  
	time ./parallel_random_walk     10240    > gpu.10240.txt    
	time ./parallel_random_walk.cpu 10240    > cpu.10240.txt  
	time ./parallel_random_walk     102400   > gpu.102400.txt   
	time ./parallel_random_walk.cpu 102400   > cpu.102400.txt  
	time ./parallel_random_walk     1024000  > gpu.1024000.txt  
	time ./parallel_random_walk.cpu 1024000  > cpu.1024000.txt  
	time ./parallel_random_walk     10240000 > gpu.10240000.txt 
	time ./parallel_random_walk.cpu 10240000 > cpu.10240000.txt  
	time ./parallel_random_walk     102400000 > gpu.102400000.txt 
	time ./parallel_random_walk.cpu 102400000 > cpu.102400000.txt  
