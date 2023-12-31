#!/bin/bash -l 

msg="=== $BASH_SOURCE :"

name=SCenterExtentFrameTest 

gcc \
    $name.cc \
    ../SCenterExtentFrame.cc \
    \
    -std=c++11 \
    -lstdc++ \
    -lm \
    -I.. \
    -Wall \
    -Wnarrowing \
    -I$OPTICKS_PREFIX/externals/glm/glm \
    -o $OPTICKS_PREFIX/lib/$name 

[ $? -ne 0 ] && echo $msg compile error && exit 1 


$name

[ $? -ne 0 ] && echo $msg run error && exit 2 



exit 0 


