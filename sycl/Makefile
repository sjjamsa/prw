# Remember to enable oneapi, eg:
# source /opt/intel/oneapi/setvars.sh

# and disable hangcheck on intel hardware for > 4s runs.
#    Open the grub file in /etc/default
#    In the grub file, find the line GRUB_CMDLINE_LINUX_DEFAULT=""
#    Enter this text between the quotes (""):
#    i915.enable_hangcheck=0
#
#    Run this command:
#    sudo update-grub
#
#    Reboot the system. Hangcheck remains disabled.
#
# And increase  i915.request_timeout_ms to a large number
# https://github.com/intel/compute-runtime/issues/497
# 



# Equivivalent openmp: ../parallel_random_walk 500 10000 20

# https://www.intel.com/content/www/us/en/developer/articles/technical/compiling-sycl-with-different-gpus.html
# https://github.com/intel/llvm/issues/5943 "make sycl-toolchain"
# https://github.com/pytorch/pytorch/issues/63480  missing libtinfo.so.6 >>>> module load ncurses/6.2

# cd to the build dir
# export DPCPP_HIP_COMPILER_ROOTDIR=$PWD/llvm/build
# export PATH=$DPCPP_HIP_COMPILER_ROOTDIR/bin:$PATH
# export LD_LIBRARY_PATH=$DPCPP_HIP_COMPILER_ROOTDIR/lib:$LD_LIBRARY_PATH



CPP=clang++ -fsycl -fsycl-targets=amdgcn-amd-amdhsa -Xsycl-target-backend=amdgcn-amd-amdhsa --offload-arch=gfx906 

prw_sycl : prw_sycl.o
	${CPP} $^ -o $@


prw_sycl.o : prw_sycl.cpp
	${CPP} -c $^ -o $@

scalingTest: prw_sycl
	time ./$^ 1024      320000 80000 gpu > gpu.1024.txt
	time ./$^ 1024      320000 80000 cpu > cpu.1024.txt
	time ./$^ 10240     320000 80000 gpu > gpu.10240.txt
	time ./$^ 10240     320000 80000 cpu > cpu.10240.txt
	time ./$^ 102400    320000 80000 gpu > gpu.102400.txt
	time ./$^ 102400    320000 80000 cpu > cpu.102400.txt
	time ./$^ 1024000   320000 80000 gpu > gpu.1024000.txt
	time ./$^ 1024000   320000 80000 cpu > cpu.1024000.txt
	time ./$^ 10240000  320000 80000 gpu > gpu.10240000.txt
	time ./$^ 10240000  320000 80000 cpu > cpu.10240000.txt
	time ./$^ 102400000 320000 80000 gpu > gpu.102400000.txt
	time ./$^ 102400000 320000 80000 cpu > cpu.102400000.txt
