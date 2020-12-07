
CFLAGS_GPU_GCC=-fno-stack-protector -foffload=nvptx-none="-misa=sm_35" -fopenmp -Minfo -O3
CFLAGS_GPU_NVC=-mp -target=gpu -gpu=cc35 -fopenmp -Minfo -O3
CFLAGS_GPU_CLANG=-fopenmp -fopenmp-targets=nvptx64 -O3


CFLAGS_CPU_GCC=-fno-stack-protector -foffload=disable -fopenmp -O3
CFLAGS_CPU_NVC=-fopenmp -O3
CFLAGS_CPU_CLANG=-fopenmp -O3




CC=clang
#CC=gcc
#CC=nvc

CFLAGS_GPU=$(CFLAGS_GPU_CLANG)
CFLAGS_CPU=$(CFLAGS_CPU_CLANG)


all : parallel_random_walk parallel_random_walk.cpu

runWalkBoth : parallel_random_walk parallel_random_walk.cpu
	./parallel_random_walk
	./parallel_random_walk.cpu

runWalk : parallel_random_walk 
	time ./parallel_random_walk

parallel_random_walk     : parallel_random_walk.c pcg.c pcg.h Makefile
	$(CC) -o $@ parallel_random_walk.c pcg.c  $(CFLAGS_GPU)

parallel_random_walk.cpu : parallel_random_walk.c pcg.c pcg.h Makefile
	$(CC) -o $@ parallel_random_walk.c pcg.c  $(CFLAGS_CPU)


