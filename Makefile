runWalkBoth : parallel_random_walk parallel_random_walk.cpu
	time ./parallel_random_walk
	time ./parallel_random_walk.cpu

runWalk : parallel_random_walk 
	time ./parallel_random_walk

parallel_random_walk : parallel_random_walk.c pcg.c pcg.h
	gcc -fopenmp -o $@ parallel_random_walk.c pcg.c  -fno-stack-protector -foffload=nvptx-none="-misa=sm_35"

parallel_random_walk.cpu : parallel_random_walk.c pcg.c pcg.h	
	gcc -fopenmp -o $@ parallel_random_walk.c pcg.c  -fno-stack-protector -foffload=disable
