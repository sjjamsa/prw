#module load CrayEnv
#module load PrgEnv-cray
#module load craype-accel-amd-gfx90a
#module load rocm

CC=cc -fopenmp -O3


CFLAGS_GPU=$(CFLAGS_GPU_CLANG)
CFLAGS_CPU=$(CFLAGS_CPU_CLANG)


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
