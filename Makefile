
CFLAGS_GPU_GCC=-fno-stack-protector -foffload=nvptx-none="-misa=sm_35" -fopenmp 
CFLAGS_GPU_NVC=-mp -target=gpu -gpu=cc35 -fopenmp -Minfo
CFLAGS_CPU_CLANG=-fopenmp=libiomp5

CFLAGS_CPU_GCC=-fno-stack-protector -fopenmp -foffload=disable
CFLAGS_CPU_NVC=-fopenmp
CFLAGS_GPU_CLANG=-fopenmp=libiomp5 -fopenmp-targets=nvptx64-nvidia-cuda  -Xopenmp-target -march=$(GPU_ARCH)

#K80
GPU_ARCH=sm_37 
#P100  
#GPU_ARCH=sm_60 
#V100
#GPU_ARCH=sm_70 


# module load gcc/9.2.0-cuda-nvptx
#CC=gcc

#CC=nvc

# module use /share/apps/spack/envs/fgci-centos7-haswell-dev/lmod/linux-centos7-x86_64/all
# module --ignore-cache purge; module --ignore-cache load llvm/11.0.1-cuda-python3-gcc-6.5.0 
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

parallel_random_walk     : parallel_random_walk.c pcg.c pcg.h Makefile
	$(CC) -o $@ parallel_random_walk.c pcg.c  $(CFLAGS_GPU)

parallel_random_walk.cpu : parallel_random_walk.c pcg.c pcg.h Makefile
	$(CC) -o $@ parallel_random_walk.c pcg.c  $(CFLAGS_CPU)


