#!/bin/bash -l 

name=scuda_test 

CUDA_PREFIX=/usr/local/cuda

gcc $name.cc \
      -std=c++11 -lstdc++ \
       -I.. \
       -I${CUDA_PREFIX}/include \
       -o /tmp/$name 

[ $? -ne 0 ] && echo $BASH_SOURCE compile error && exit 1 

/tmp/$name
[ $? -ne 0 ] && echo $BASH_SOURCE run error && exit 2 

exit 0 

