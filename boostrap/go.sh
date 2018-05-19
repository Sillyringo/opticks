#!/bin/bash -l

opticks-

sdir=$(pwd)
name=$(basename $sdir)

#bdir=/tmp/$USER/opticks/$name/build 
bdir=$(opticks-prefix)/build/$name 

echo sdir $sdir
echo bdir $bdir

#rm -rf $bdir 

mkdir -p $bdir && cd $bdir && pwd 

cmake $sdir \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_INSTALL_PREFIX=$(opticks-prefix) \
    -DCMAKE_MODULE_PATH=$(opticks-home)/cmake/Modules

make
make install   

