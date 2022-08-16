# Remember to enable oneapi, eg:
# source /opt/intel/oneapi/setvars.sh

CPP=icpx -fsycl

prw_sycl : prw_sycl.o
	${CPP} $^ -o $@


prw_sycl.o : prw_sycl.cpp
	${CPP} -c $^ -o $@
