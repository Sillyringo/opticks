##
## Copyright (c) 2019 Opticks Team. All Rights Reserved.
##
## This file is part of Opticks
## (see https://bitbucket.org/simoncblyth/opticks).
##
## Licensed under the Apache License, Version 2.0 (the "License"); 
## you may not use this file except in compliance with the License.  
## You may obtain a copy of the License at
##
##   http://www.apache.org/licenses/LICENSE-2.0
##
## Unless required by applicable law or agreed to in writing, software 
## distributed under the License is distributed on an "AS IS" BASIS, 
## WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  
## See the License for the specific language governing permissions and 
## limitations under the License.
##

cudarap-source(){   echo $BASH_SOURCE ; }
cudarap-vi(){       vi $(cudarap-source) ; }
cudarap-usage(){ cat << "EOU"

CUDAWrap
==========

CentOS7
----------

::

	[blyth@localhost cudarap]$ om--
	=== om-make-one : cudarap         /home/blyth/opticks/cudarap                                  /home/blyth/local/opticks/build/cudarap                      
	[  5%] Building NVCC (Device) object CMakeFiles/CUDARap.dir/CUDARap_generated_CResource_.cu.o
	nvcc fatal   : Unsupported gpu architecture 'compute_'
	CMake Error at CUDARap_generated_CResource_.cu.o.Debug.cmake:219 (message):
	  Error generating
	  /home/blyth/local/opticks/build/cudarap/CMakeFiles/CUDARap.dir//./CUDARap_generated_CResource_.cu.o


	gmake[2]: *** [CMakeFiles/CUDARap.dir/CUDARap_generated_CResource_.cu.o] Error 1
	gmake[1]: *** [CMakeFiles/CUDARap.dir/all] Error 2
	gmake: *** [all] Error 2
	=== om-one-or-all make : non-zero rc 2
	[blyth@localhost cudarap]$ 
	[blyth@localhost cudarap]$ 
	[blyth@localhost cudarap]$ opticks-compute-capability
	opticks-compute-capability   opticks-compute-capability-  
	[blyth@localhost cudarap]$ opticks-compute-capability
	70



High Sierra Error 
--------------------

* http://docs.nvidia.com/cuda/pdf/CUDA_Toolkit_Release_Notes.pdf

NVIDIA CUDA TOOLKIT 9.1.199
RN-06722-001 _v9.1 | March 2018
Release Notes for Windows, Linux, and Mac OS

* Xcode 9.2 is now supported as a host compiler on Mac OS.


::

    delta:opticksnpy blyth$ cudarap--
    [  0%] Building NVCC (Device) object cudarap/CMakeFiles/CUDARap.dir/CUDARap_generated_CResource_.cu.o

    nvcc fatal   : The version ('90100') of the host compiler ('Apple clang') is not supported

    CMake Error at CUDARap_generated_CResource_.cu.o.cmake:203 (message):
      Error generating
      /usr/local/opticks/build/cudarap/CMakeFiles/CUDARap.dir//./CUDARap_generated_CResource_.cu.o


    make[2]: *** [cudarap/CMakeFiles/CUDARap.dir/CUDARap_generated_CResource_.cu.o] Error 1
    make[1]: *** [cudarap/CMakeFiles/CUDARap.dir/all] Error 2
    make: *** [all] Error 2
    delta:opticksnpy blyth$ 


Reproduced the belo with nvcc-hello::

    nvcc-;nvcc-hello

    nvcc fatal   : The version ('90100') of the host compiler ('Apple clang') is not supported

Solved with::

    xcode-;xcode-92
    nvcc-;nvcc-hello

For that to apply to opticks needed to::

    opticks-configure
    cudarap--          
       ## this means that now have a mixed xcode-93 and xcode-92 compiler version build, 
       ## better to start from scratch ? 



* https://stackoverflow.com/questions/36250949/revert-apple-clang-version-for-nvcc



Warnings
---------

::

    simon:cudarap blyth$ cudarap--
    [ 33%] Building NVCC (Device) object cudarap/CMakeFiles/CUDARap.dir/CUDARap_generated_CResource_.cu.o
    [ 33%] Building NVCC (Device) object cudarap/CMakeFiles/CUDARap.dir/CUDARap_generated_cuRANDWrapper_kernel.cu.o
    In file included from /usr/local/cuda/include/curand_poisson.h:231:
    In file included from /usr/local/cuda/include/common_functions.h:219:
    In file included from /usr/local/cuda/include/math_functions.h:9342:
    In file included from /usr/local/cuda/include/device_functions.h:4183:
    /usr/local/cuda/include/device_functions.hpp:283:3: warning: extension used [-Wlanguage-extension-token]
      asm __volatile__ ("{ \n\t"
      ^
    /usr/local/cuda/include/device_functions.hpp:296:3: warning: extension used [-Wlanguage-extension-token]
      asm __volatile__ ("{ \n\t"
      ^

::

     65 # CUDA headers yield many:
     66 #   /usr/local/cuda/include/device_functions.hpp:283:3:   warning: extension used [-Wlanguage-extension-token]
     67 #
     68 # set(XOPT --disable-warnings) 
     69 # BUT too much of a sledgehammer to disable all warnings
     70 #
     71 
     72 CUDA_ADD_LIBRARY( ${name} ${SOURCES} OPTIONS -gencode=arch=compute_30,code=sm_30   )



Tools not cooperating on macOS
-----------------------------------

* https://devtalk.nvidia.com/default/topic/856171/unnecessary-compiler-warning-with-nvcc-/
* http://stackoverflow.com/questions/14831051/how-to-disable-compiler-warnings-with-nvcc

::

    simon:boostrap blyth$ nvcc --display_error_number
    nvcc fatal   : Unknown option '-display_error_number'


    simon:boostrap blyth$ which cudafe
    /Developer/NVIDIA/CUDA-7.0/bin/cudafe

    simon:boostrap blyth$ cudafe --display_error_number
    Command-line error #593: missing source file name


CUDARAP usage
----------------

::

    bogon:env blyth$ find . -name '*.*' -exec grep -H CUDARAP {} \; 
    ./cudarap/cudarap.bash:   CUDARAP_RNG_DIR=$(cudarap-rng-dir) DYLD_LIBRARY_PATH=. WORK=$(( 1024*768 )) ./cuRANDWrapperTest 
    ./cudarap/cudarap.bash:   CUDARAP_RNG_DIR=$(cudarap-rng-dir) DYLD_LIBRARY_PATH=. WORK=$(( 1440*900 )) ./cuRANDWrapperTest 
    ./cudarap/cuRANDWrapperTest.cc:    unsigned int work              = getenvvar("CUDARAP_RNG_MAX", WORK) ;
    ./cudarap/cuRANDWrapperTest.cc:    char* cachedir = getenv("CUDARAP_RNG_DIR") ;
    ./graphics/ggeoview/ggeoview.bash:   CUDARAP_RNG_DIR=$(ggeoview-rng-dir) CUDARAP_RNG_MAX=$(ggeoview-rng-max) $(cudarap-ibin)

    ./graphics/ggeoview/ggeoview.bash:   export CUDARAP_RNG_MAX=$(ggeoview-rng-max)

    ./graphics/ggeoview/ggeoview.bash:   env | grep CUDARAP
    grep: ./offline/tg/OfflineDB/OfflineDB.egg-info: Is a directory
    ./opticks/Opticks.cc:   int rng_max = getenvint("CUDARAP_RNG_MAX",-1); 
    ./opticks/Opticks.cc:                  << " CUDARAP_RNG_MAX " << rng_max 
    ./opticks/Opticks.cc:   assert(rng_max == x_rng_max && "Configured RngMax must match envvar CUDARAP_RNG_MAX and corresponding files, see cudarap- ");    
    ./opticks/OpticksCfg.hh:"Value must match envvar CUDARAP_RNG_MAX and corresponding pre-cooked seeds, see cudarap- for details. "


::

    1964 ggeoview-rng-max()
    1965 {
    1966    # maximal number of photons that can be handled : move to cudarap- ?
    1967     #echo $(( 1000*1000*3 ))
    1968     echo $(( 1000*1000*1 ))
    1969 }




census
-------

::

    bogon:ggeo blyth$ cudarap-ccd
    bogon:rngcache blyth$ l
    total 179840
    -rw-r--r--  1 blyth  staff  57024000 Mar 23  2015 cuRANDWrapper_1296000_0_0.bin
    -rw-r--r--  1 blyth  staff    450560 Mar 23  2015 cuRANDWrapper_10240_0_0.bin
    -rw-r--r--  1 blyth  staff  34603008 Mar 23  2015 cuRANDWrapper_786432_0_0.bin

    bogon:rngcache blyth$ echo $(( 1024*768 ))
    786432
    bogon:rngcache blyth$ echo $(( 1440*900 ))
    1296000



CUDA prior to 7.0 needs libstdc++ how is CUDAWrap working with libc++ ?
--------------------------------------------------------------------------------

Presumbaly because it does not use the STL, just a few simples classes and C string.h 

::

    /usr/local/env/cudarap/lib/libCUDAWrap.dylib:
        @rpath/libCUDAWrap.dylib (compatibility version 0.0.0, current version 0.0.0)
        @rpath/libcudart.5.5.dylib (compatibility version 0.0.0, current version 5.5.28)
        /usr/lib/libc++.1.dylib (compatibility version 1.0.0, current version 120.0.0)
        /usr/lib/libSystem.B.dylib (compatibility version 1.0.0, current version 1197.1.1)


Include arrangment OptiX and CUDA
---------------------------------------

When place include without "using namespace optix" get::

    [100%] Building CXX object CMakeFiles/GGeoView.dir/main.cc.o
    In file included from /Users/blyth/env/graphics/ggeoview/main.cc:48:
    In file included from /usr/local/env/CUDAWrap/include/cuRANDWrapper.hh:8:
    In file included from /usr/local/cuda/include/curand_kernel.h:60:
    In file included from /usr/local/cuda/include/curand.h:59:
    In file included from /usr/local/cuda/include/cuda_runtime.h:68:
    In file included from /usr/local/cuda/include/channel_descriptor.h:62:
    /usr/local/cuda/include/cuda_runtime_api.h:193:17: error: unknown type name 'cudaError_t'; did you mean 'optix::cudaError_t'?
    extern __host__ cudaError_t CUDARTAPI cudaDeviceReset(void);
                    ^~~~~~~~~~~
                    optix::cudaError_t

Correct way to include::

    // cudarap-
    using namespace optix ; 
    #include "cuRANDWrapper.hh"
    #include "curand.h"
    #include "curand_kernel.h"

* TODO: look into avoiding this requirement



Build Warnings
-----------------

Warning::

    delta:2015 blyth$ cudarap-make
    [ 16%] Building NVCC (Device) object CMakeFiles/CUDAEnv.dir//./CUDAEnv_generated_cuRANDWrapper_kernel.cu.o
    ptxas /tmp/tmpxft_00004fb0_00000000-5_cuRANDWrapper_kernel.ptx, line 894; warning : Double is not supported. Demoting to float
    Scanning dependencies of target CUDAEnv

    Created ptx has f64 on the offending line 894
    nvcc -ptx cuRANDWrapper_kernel.cu

    861     .entry _Z8test_rngiiP17curandStateXORWOWPf (
    862         .param .s32 __cudaparm__Z8test_rngiiP17curandStateXORWOWPf_threads_per_launch,
    863         .param .s32 __cudaparm__Z8test_rngiiP17curandStateXORWOWPf_thread_offset,
    864         .param .u64 __cudaparm__Z8test_rngiiP17curandStateXORWOWPf_rng_states,
    865         .param .u64 __cudaparm__Z8test_rngiiP17curandStateXORWOWPf_a)
    866     {
    ...
    887     ld.param.u64    %rd2, [__cudaparm__Z8test_rngiiP17curandStateXORWOWPf_rng_states];
    888     mul.wide.s32    %rd3, %r3, 48;
    889     add.u64     %rd4, %rd2, %rd3;
    890     ld.global.u32   %r5, [%rd4+4];
    891     ld.global.u32   %r6, [%rd4+20];
    892     ld.global.v2.s32    {%r7,%r8}, [%rd4+24];
    893     ld.global.f32   %f1, [%rd4+32];
    894     ld.global.f64   %fd1, [%rd4+40];
    895     .loc    14  125 0


Commenting rng on two lines in the below gets rid of the warning::

    112 __global__ void test_rng(int threads_per_launch, int thread_offset, curandState* rng_states, float *a)
    113 {
    114    //
    115 
    116     int id = blockIdx.x*blockDim.x + threadIdx.x;
    117     if (id >= threads_per_launch) return;
    118 
    119     // NB no id offsetting on rng_states or a, as the offsetting
    120     // was done once in the kernel call 
    121     // this means thread_offset argument not used
    122 
    123     curandState rng = rng_states[id];   
    124 
    125     //a[id] = curand_uniform(&rng);
    126 
    127     //rng_states[id] = rng;   
    128 }


From mdfind curand_kernel.h, /Developer/NVIDIA/CUDA-5.5/include/curand_kernel.h 
see that copying boxmuller_extra_double is causing the warning:: 

     121  * Implementation details not in reference documentation */
     122 struct curandStateXORWOW {
     123     unsigned int d, v[5];
     124     int boxmuller_flag;
     125     int boxmuller_flag_double;
     126     float boxmuller_extra;
     127     double boxmuller_extra_double;
     128 };
     ...
     ...       6*6 + 4 + 8 = 48 
     ...
     275 /*
     276  * Default RNG
     277  */
     278 /** \cond UNHIDE_TYPEDEFS */
     279 typedef struct curandStateXORWOW curandState_t;
     280 typedef struct curandStateXORWOW curandState;
     281 /** \endcond */



    [ 16%] Building NVCC (Device) object CMakeFiles/CUDAEnv.dir//./CUDAEnv_generated_cuRANDWrapper_kernel.cu.o
    /Users/blyth/env/cudarap/cuRANDWrapper_kernel.cu(123): warning: variable "rng" was declared but never referenced

    /Users/blyth/env/cudarap/cuRANDWrapper_kernel.cu(123): warning: variable "rng" was declared but never referenced


* http://stackoverflow.com/questions/19034321/cuda-double-demoted-to-float-and-understanding-ptx-output

* http://lists.tiker.net/pipermail/pycuda/2011-December/003513.html



Usage examples
-----------------

::

    delta:cudarap blyth$ DYLD_LIBRARY_PATH=. WORK=$(( 1024*128 )) ./cuRANDWrapperTest 
    seq workitems  131072  threads_per_block   256  max_blocks    128 nlaunch   4 
     seq sequence_index   0  thread_offset      0  threads_per_launch  32768 blocks_per_launch    128   threads_per_block    256  
     seq sequence_index   1  thread_offset  32768  threads_per_launch  32768 blocks_per_launch    128   threads_per_block    256  
     seq sequence_index   2  thread_offset  65536  threads_per_launch  32768 blocks_per_launch    128   threads_per_block    256  
     seq sequence_index   3  thread_offset  98304  threads_per_launch  32768 blocks_per_launch    128   threads_per_block    256  
     init_rng_wrapper sequence_index   0  thread_offset      0  threads_per_launch  32768 blocks_per_launch    128   threads_per_block    256  
     init_rng_wrapper sequence_index   1  thread_offset  32768  threads_per_launch  32768 blocks_per_launch    128   threads_per_block    256  
     init_rng_wrapper sequence_index   2  thread_offset  65536  threads_per_launch  32768 blocks_per_launch    128   threads_per_block    256  
     init_rng_wrapper sequence_index   3  thread_offset  98304  threads_per_launch  32768 blocks_per_launch    128   threads_per_block    256  
    delta:cudarap blyth$ 



Comparing LaunchSequenceTest with cuda_launch.py 
Need to define all the envvars to get a match, as defaults not aligned::

    delta:cudarap blyth$ ITEMS=$(( 1024*768 )) THREADS_PER_BLOCK=256 MAX_BLOCKS=256 cudarap-lst
    seq workitems  786432  threads_per_block   256  max_blocks    256 nlaunch  12 
     seq sequence_index   0  thread_offset      0  threads_per_launch  65536 blocks_per_launch    256   threads_per_block    256  
     seq sequence_index   1  thread_offset  65536  threads_per_launch  65536 blocks_per_launch    256   threads_per_block    256  
     seq sequence_index   2  thread_offset 131072  threads_per_launch  65536 blocks_per_launch    256   threads_per_block    256  
     seq sequence_index   3  thread_offset 196608  threads_per_launch  65536 blocks_per_launch    256   threads_per_block    256  
     seq sequence_index   4  thread_offset 262144  threads_per_launch  65536 blocks_per_launch    256   threads_per_block    256  
     seq sequence_index   5  thread_offset 327680  threads_per_launch  65536 blocks_per_launch    256   threads_per_block    256  
     seq sequence_index   6  thread_offset 393216  threads_per_launch  65536 blocks_per_launch    256   threads_per_block    256  
     seq sequence_index   7  thread_offset 458752  threads_per_launch  65536 blocks_per_launch    256   threads_per_block    256  
     seq sequence_index   8  thread_offset 524288  threads_per_launch  65536 blocks_per_launch    256   threads_per_block    256  
     seq sequence_index   9  thread_offset 589824  threads_per_launch  65536 blocks_per_launch    256   threads_per_block    256  
     seq sequence_index  10  thread_offset 655360  threads_per_launch  65536 blocks_per_launch    256   threads_per_block    256  
     seq sequence_index  11  thread_offset 720896  threads_per_launch  65536 blocks_per_launch    256   threads_per_block    256  
    Launch1D work [786432] total 786432 max_blocks 256 threads_per_block 256 block (256, 1, 1) 
    offset          0 count 65536 grid (256, 1) block (256, 1, 1) 
    offset      65536 count 65536 grid (256, 1) block (256, 1, 1) 
    offset     131072 count 65536 grid (256, 1) block (256, 1, 1) 
    offset     196608 count 65536 grid (256, 1) block (256, 1, 1) 
    offset     262144 count 65536 grid (256, 1) block (256, 1, 1) 
    offset     327680 count 65536 grid (256, 1) block (256, 1, 1) 
    offset     393216 count 65536 grid (256, 1) block (256, 1, 1) 
    offset     458752 count 65536 grid (256, 1) block (256, 1, 1) 
    offset     524288 count 65536 grid (256, 1) block (256, 1, 1) 
    offset     589824 count 65536 grid (256, 1) block (256, 1, 1) 
    offset     655360 count 65536 grid (256, 1) block (256, 1, 1) 
    offset     720896 count 65536 grid (256, 1) block (256, 1, 1) 
    delta:cudarap blyth$ 



Below demonstrates that curandState caching works in CUDA only running, 
but mysteriously the test is a factor of three slower when curandState 
was loaded from cache as opposed to being curand_init::

    delta:cudarap blyth$ cudarap-test
     init_rng_wrapper sequence_index   0  thread_offset       0  threads_per_launch  32768 blocks_per_launch    128   threads_per_block    256  kernel_time   135.4791 ms 
     init_rng_wrapper sequence_index   1  thread_offset   32768  threads_per_launch  32768 blocks_per_launch    128   threads_per_block    256  kernel_time   139.1703 ms 
     init_rng_wrapper sequence_index   2  thread_offset   65536  threads_per_launch  32768 blocks_per_launch    128   threads_per_block    256  kernel_time   138.1142 ms 
     init_rng_wrapper sequence_index   3  thread_offset   98304  threads_per_launch  32768 blocks_per_launch    128   threads_per_block    256  kernel_time   158.8427 ms 
     init_rng_wrapper sequence_index   4  thread_offset  131072  threads_per_launch  32768 blocks_per_launch    128   threads_per_block    256  kernel_time   138.6337 ms 
     init_rng_wrapper sequence_index   5  thread_offset  163840  threads_per_launch  32768 blocks_per_launch    128   threads_per_block    256  kernel_time   146.0475 ms 
     init_rng_wrapper sequence_index   6  thread_offset  196608  threads_per_launch  32768 blocks_per_launch    128   threads_per_block    256  kernel_time   157.7762 ms 
     init_rng_wrapper sequence_index   7  thread_offset  229376  threads_per_launch  32768 blocks_per_launch    128   threads_per_block    256  kernel_time   170.1313 ms 
     init_rng_wrapper sequence_index   8  thread_offset  262144  threads_per_launch  32768 blocks_per_launch    128   threads_per_block    256  kernel_time  1070.4102 ms 
     init_rng_wrapper sequence_index   9  thread_offset  294912  threads_per_launch  32768 blocks_per_launch    128   threads_per_block    256  kernel_time  1043.7632 ms 
     init_rng_wrapper sequence_index  10  thread_offset  327680  threads_per_launch  32768 blocks_per_launch    128   threads_per_block    256  kernel_time  1057.3168 ms 
     init_rng_wrapper sequence_index  11  thread_offset  360448  threads_per_launch  32768 blocks_per_launch    128   threads_per_block    256  kernel_time  1070.9830 ms 
     init_rng_wrapper sequence_index  12  thread_offset  393216  threads_per_launch  32768 blocks_per_launch    128   threads_per_block    256  kernel_time  1084.1074 ms 
     init_rng_wrapper sequence_index  13  thread_offset  425984  threads_per_launch  32768 blocks_per_launch    128   threads_per_block    256  kernel_time  1098.1005 ms 
     init_rng_wrapper sequence_index  14  thread_offset  458752  threads_per_launch  32768 blocks_per_launch    128   threads_per_block    256  kernel_time  1112.0490 ms 
     init_rng_wrapper sequence_index  15  thread_offset  491520  threads_per_launch  32768 blocks_per_launch    128   threads_per_block    256  kernel_time  1125.6128 ms 
     init_rng_wrapper sequence_index  16  thread_offset  524288  threads_per_launch  32768 blocks_per_launch    128   threads_per_block    256  kernel_time  1037.1678 ms 
     init_rng_wrapper sequence_index  17  thread_offset  557056  threads_per_launch  32768 blocks_per_launch    128   threads_per_block    256  kernel_time  1050.6687 ms 
     init_rng_wrapper sequence_index  18  thread_offset  589824  threads_per_launch  32768 blocks_per_launch    128   threads_per_block    256  kernel_time  1064.1864 ms 
     init_rng_wrapper sequence_index  19  thread_offset  622592  threads_per_launch  32768 blocks_per_launch    128   threads_per_block    256  kernel_time  1077.8341 ms 
     init_rng_wrapper sequence_index  20  thread_offset  655360  threads_per_launch  32768 blocks_per_launch    128   threads_per_block    256  kernel_time  1091.3204 ms 
     init_rng_wrapper sequence_index  21  thread_offset  688128  threads_per_launch  32768 blocks_per_launch    128   threads_per_block    256  kernel_time  1104.9775 ms 
     init_rng_wrapper sequence_index  22  thread_offset  720896  threads_per_launch  32768 blocks_per_launch    128   threads_per_block    256  kernel_time  1118.7540 ms 
     init_rng_wrapper sequence_index  23  thread_offset  753664  threads_per_launch  32768 blocks_per_launch    128   threads_per_block    256  kernel_time  1132.4813 ms 
    init_rng_wrapper tag init workitems  786432  threads_per_block   256  max_blocks    128 reverse 0 nlaunch  24 TotalTime 18523.9277 ms 
    cuRANDWrapper::Save 786432 items to /tmp/env/cuRANDWrapperTest/cachedir/cuRANDWrapper_786432_0_0.bin save_digest 98b4055e7205ed0841d70564dd30895b 
    cuRANDWrapper::Save mkdirp for path /tmp/env/cuRANDWrapperTest/cachedir/cuRANDWrapper_786432_0_0.bin m_cache_dir /tmp/env/cuRANDWrapperTest/cachedir 
    test_0 c98f62f34773d0213c1c641c0c57936e     0.7402     0.9012     0.3494     0.3829     0.4598     0.5884     0.3593     0.4285 
    test_1 466072011433d91d56c3cac671a1b2da     0.4385     0.0628     0.8695     0.0006     0.1884     0.9480     0.1709     0.6487 
    test_2 ff5a848aeeb57cac41c80964d6c0b1ca     0.5170     0.8406     0.2722     0.2081     0.3173     0.9427     0.7670     0.7851 
    test_3 2c911501a3372c3d0476455d005b1ebe     0.1570     0.6885     0.8124     0.5285     0.0230     0.1309     0.2851     0.2959 
    test_4 867f5171ec223d039b1614d3ef3d9887     0.0714     0.9468     0.3954     0.6760     0.5098     0.2924     0.6596     0.1359 
    cuRANDWrapperTest::main tag init workitems  786432  threads_per_block   256  max_blocks    128 reverse 0 nlaunch  24 TotalTime 18523.9277 ms 
    cuRANDWrapperTest::main tag test_0 workitems  786432  threads_per_block   256  max_blocks   4096 reverse 0 nlaunch   1 TotalTime     5.3581 ms 
    cuRANDWrapperTest::main tag test_1 workitems  786432  threads_per_block   256  max_blocks   4096 reverse 0 nlaunch   1 TotalTime     5.3279 ms 
    cuRANDWrapperTest::main tag test_2 workitems  786432  threads_per_block   256  max_blocks   4096 reverse 0 nlaunch   1 TotalTime     5.3420 ms 
    cuRANDWrapperTest::main tag test_3 workitems  786432  threads_per_block   256  max_blocks   4096 reverse 0 nlaunch   1 TotalTime     5.3855 ms 
    cuRANDWrapperTest::main tag test_4 workitems  786432  threads_per_block   256  max_blocks   4096 reverse 0 nlaunch   1 TotalTime     5.3906 ms 
    delta:cudarap blyth$ 
    delta:cudarap blyth$ cudarap-test
    cuRANDWrapper::Load 786432 items from /tmp/env/cuRANDWrapperTest/cachedir/cuRANDWrapper_786432_0_0.bin load_digest 98b4055e7205ed0841d70564dd30895b 
    cuRANDWrapper::Load roundtrip_digest 98b4055e7205ed0841d70564dd30895b 
    test_0 c98f62f34773d0213c1c641c0c57936e     0.7402     0.9012     0.3494     0.3829     0.4598     0.5884     0.3593     0.4285 
    test_1 466072011433d91d56c3cac671a1b2da     0.4385     0.0628     0.8695     0.0006     0.1884     0.9480     0.1709     0.6487 
    test_2 ff5a848aeeb57cac41c80964d6c0b1ca     0.5170     0.8406     0.2722     0.2081     0.3173     0.9427     0.7670     0.7851 
    test_3 2c911501a3372c3d0476455d005b1ebe     0.1570     0.6885     0.8124     0.5285     0.0230     0.1309     0.2851     0.2959 
    test_4 867f5171ec223d039b1614d3ef3d9887     0.0714     0.9468     0.3954     0.6760     0.5098     0.2924     0.6596     0.1359 
    cuRANDWrapperTest::main tag test_0 workitems  786432  threads_per_block   256  max_blocks   4096 reverse 0 nlaunch   1 TotalTime    14.9684 ms 
    cuRANDWrapperTest::main tag test_1 workitems  786432  threads_per_block   256  max_blocks   4096 reverse 0 nlaunch   1 TotalTime    17.5934 ms 
    cuRANDWrapperTest::main tag test_2 workitems  786432  threads_per_block   256  max_blocks   4096 reverse 0 nlaunch   1 TotalTime    16.8187 ms 
    cuRANDWrapperTest::main tag test_3 workitems  786432  threads_per_block   256  max_blocks   4096 reverse 0 nlaunch   1 TotalTime    15.0857 ms 
    cuRANDWrapperTest::main tag test_4 workitems  786432  threads_per_block   256  max_blocks   4096 reverse 0 nlaunch   1 TotalTime    16.4365 ms 
    delta:cudarap blyth$ 
    delta:cudarap blyth$ cudarap-test
    cuRANDWrapper::Load 786432 items from /tmp/env/cuRANDWrapperTest/cachedir/cuRANDWrapper_786432_0_0.bin load_digest 98b4055e7205ed0841d70564dd30895b 
    cuRANDWrapper::Load roundtrip_digest 98b4055e7205ed0841d70564dd30895b 
    test_0 c98f62f34773d0213c1c641c0c57936e     0.7402     0.9012     0.3494     0.3829     0.4598     0.5884     0.3593     0.4285 
    test_1 466072011433d91d56c3cac671a1b2da     0.4385     0.0628     0.8695     0.0006     0.1884     0.9480     0.1709     0.6487 
    test_2 ff5a848aeeb57cac41c80964d6c0b1ca     0.5170     0.8406     0.2722     0.2081     0.3173     0.9427     0.7670     0.7851 
    test_3 2c911501a3372c3d0476455d005b1ebe     0.1570     0.6885     0.8124     0.5285     0.0230     0.1309     0.2851     0.2959 
    test_4 867f5171ec223d039b1614d3ef3d9887     0.0714     0.9468     0.3954     0.6760     0.5098     0.2924     0.6596     0.1359 
    cuRANDWrapperTest::main tag test_0 workitems  786432  threads_per_block   256  max_blocks   4096 reverse 0 nlaunch   1 TotalTime    15.1265 ms 
    cuRANDWrapperTest::main tag test_1 workitems  786432  threads_per_block   256  max_blocks   4096 reverse 0 nlaunch   1 TotalTime    14.8927 ms 
    cuRANDWrapperTest::main tag test_2 workitems  786432  threads_per_block   256  max_blocks   4096 reverse 0 nlaunch   1 TotalTime    16.8397 ms 
    cuRANDWrapperTest::main tag test_3 workitems  786432  threads_per_block   256  max_blocks   4096 reverse 0 nlaunch   1 TotalTime    15.0116 ms 
    cuRANDWrapperTest::main tag test_4 workitems  786432  threads_per_block   256  max_blocks   4096 reverse 0 nlaunch   1 TotalTime    16.8369 ms 
    delta:cudarap blyth$ 




EOU
}


cudarap-env(){      
  olocal-  
  cuda-
  opticks-
}


cudarap-idir(){ echo $(opticks-idir); }
cudarap-bdir(){ echo $(opticks-bdir)/cudarap ; }
cudarap-sdir(){ echo $(opticks-home)/cudarap ; }
cudarap-tdir(){ echo $(opticks-home)/cudarap/tests ; }
cudarap-ibin(){ echo $(cudarap-idir)/lib/cuRANDWrapperTest ; }


cudarap-c(){    cd $(cudarap-sdir)/$1 ; }
cudarap-cd(){   cd $(cudarap-sdir)/$1 ; }
cudarap-scd(){  cd $(cudarap-sdir); }
cudarap-tcd(){  cd $(cudarap-tdir); }
cudarap-bcd(){  cd $(cudarap-bdir); }


cudarap-wipe(){ local bdir=$(cudarap-bdir) ;  rm -rf $bdir ; } 

cudarap-name(){ echo CUDARap ; }
cudarap-tag(){  echo CUDARAP ; }

cudarap-apihh(){  echo $(cudarap-sdir)/$(cudarap-tag)_API_EXPORT.hh ; }
cudarap---(){     touch $(cudarap-apihh) ; cudarap--  ; }


cudarap--(){                   opticks-- $(cudarap-bdir) ; } 
cudarap-t(){                  opticks-t $(cudarap-bdir) $* ; } 
cudarap-clean(){              opticks-make- $(cudarap-bdir) clean ; } 
cudarap-genproj() { cudarap-scd ; opticks-genproj $(cudarap-name) $(cudarap-tag) ; } 
cudarap-gentest() { cudarap-tcd ; opticks-gentest ${1:-CExample} $(cudarap-tag) ; } 
cudarap-txt(){ vi $(cudarap-sdir)/CMakeLists.txt $(cudarap-tdir)/CMakeLists.txt ; } 



cudarap-bin(){ echo $(cudarap-bdir)/$(cudarap-name)Test ; }
cudarap-lst-bin(){ echo $(cudarap-bdir)/LaunchSequenceTest ; }

cudarap-lst(){ 
   local bin=$(cudarap-lst-bin)
   $bin $*

   cuda_launch.py ${ITEMS:-$(( 1024*768 ))}
}


cudarap-export(){
  echo -n
}

cudarap-run(){
  if [ -n "$DEBUG" ]; then
      $DEBUG $(cudarap-bin) -- $*
  else
      $(cudarap-bin) $*
  fi
}

#cudarap-rngmax-M(){ echo 1 ; }
cudarap-rngmax-M(){ echo ${CUDARAP_RNGMAX_M:-3} ; }
#cudarap-rngmax-M(){ echo 10 ; }
#cudarap-rngmax-M(){ echo 100 ; }

cudarap-rngmax(){ echo $(( $(cudarap-rngmax-M)*1000*1000 )) ; } # maximal number of photons that can be handled

cudarap-rngdir(){ echo $(opticks-rngdir)  ; }


cudarap-prepare-installation-notes(){ cat << EON
$FUNCNAME
====================================

Rerunning will load the pre-existing file and check it, 
without changing it or consuming much time.

Tried varying MAX_BLOCKS to do more in a single launch
in cudarap-test-1M but did not have significant effect 
in total kernel time. 

TODO: check again with different GPUs

EON
}



cudarap-prepare-sizes-Linux-(){  echo ${OPTICKS_CUDARAP_RNGMAX:-1,3,10} ; }
cudarap-prepare-sizes-Darwin-(){ echo ${OPTICKS_CUDARAP_RNGMAX:-1,3} ; }
cudarap-prepare-sizes(){ $FUNCNAME-$(uname)- | tr "," "\n"  ; }

cudarap-prepare-installation()
{
   local size
   cudarap-prepare-sizes | while read size ; do    
      CUDARAP_RNGMAX_M=$size  cudarap-prepare-rng-
   done 
}

cudarap-prepare-rng-()
{
   local msg="=== $FUNCNAME :"
   local path=$(cudarap-rngpath)
   [ -f "$path" ] && echo $msg path $path exists already && return 0

   CUDARAP_RNG_DIR=$(cudarap-rngdir) CUDARAP_RNG_MAX=$(cudarap-rngmax) $(cudarap-ibin)
}


cudarap-prepare-rng-400M(){ CUDARAP_RNGMAX_M=400 cudarap-prepare-rng- ; }
cudarap-prepare-rng-200M(){ CUDARAP_RNGMAX_M=200 cudarap-prepare-rng- ; }
cudarap-prepare-rng-100M(){ CUDARAP_RNGMAX_M=100 cudarap-prepare-rng- ; }
cudarap-prepare-rng-10M(){  CUDARAP_RNGMAX_M=10  cudarap-prepare-rng- ; }
cudarap-prepare-rng-5M(){   CUDARAP_RNGMAX_M=5   cudarap-prepare-rng- ; }
cudarap-prepare-rng-2M(){   CUDARAP_RNGMAX_M=2   cudarap-prepare-rng- ; }
cudarap-prepare-rng-1M(){   CUDARAP_RNGMAX_M=1   cudarap-prepare-rng- ; }

cudarap-rngdir-ls(){ ls -l $(cudarap-rngdir) ; }
cudarap-rngdir-du(){ du -h $(cudarap-rngdir)/* ; }

cudarap-rngname(){ echo cuRANDWrapper_$(cudarap-rngmax)_0_0.bin ; }
cudarap-rngname-1M(){  CUDARAP_RNGMAX_M=1 cudarap-rngname ; }
cudarap-rngpath(){ echo $(cudarap-rngdir)/$(cudarap-rngname) ; } 




cudarap-check-installation()
{
   local msg="=== $FUNCNAME :"

   local rc=0
   cudarap-check-rngdir-
   rc=$? ; [ $rc -ne 0 ] && return $rc 

   local size 
   cudarap-prepare-sizes | while read size ; do    
       CUDARAP_RNGMAX_M=$size  cudarap-check-rngpath- 
       rc=$? ; [ $rc -ne 0 ] && return $rc 
   done 

   return $rc
}

cudarap-check-installation-extra()
{
   local msg="=== $FUNCNAME :"
   local rcx=0
   CUDARAP_RNGMAX_M=100 cudarap-check-rngpath-
   rcx=$?  
   if [ $rcx -ne 0 ]; then 
       echo $msg to operate with up to 100M photons you need to run : cudarap-prepare-rng-100M 
   fi 
}



cudarap-check-rngdir-()
{
    local msg="$FUNCNAME :"
    local rc=0
    local rngdir=$(cudarap-rngdir)
    local err=""
    if [ ! -d "$rngdir" ]; then
        rc=201
        err="ERROR MISSING DIR"
    fi 
    echo $msg $rngdir $err rc $rc
    return $rc
}

cudarap-check-rngpath-()
{
    local msg="$FUNCNAME :"
    local rc=0
    local path=$(cudarap-rngpath)
    local err=""
    if [ ! -f $path ]; then
        rc=202
        err="ERROR MISSING PATH"
    fi    
    echo $msg $path $err rc $rc
    return $rc
}



cudarap-test-1M()
{
   local rngdir=$(cudarap-rngdir)
   mkdir -p $rngdir
   cd $rngdir 

   local name=$(cudadap-rngname-1M)
   rm $name

   cuRANDWrapper=INFO MAX_BLOCKS=4096 cudarap-prepare-rng-1M 
}

