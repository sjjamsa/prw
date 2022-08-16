# Remember to enable oneapi, eg:
# source /opt/intel/oneapi/setvars.sh

# Equivivalent openmp: ../parallel_random_walk 500 10000 20

CPP=icpx -fsycl

prw_sycl : prw_sycl.o
	${CPP} $^ -o $@


prw_sycl.o : prw_sycl.cpp
	${CPP} -c $^ -o $@
