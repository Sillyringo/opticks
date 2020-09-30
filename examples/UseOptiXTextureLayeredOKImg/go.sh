#!/bin/bash -l
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


opticks-
oe-
om-

sdir=$(pwd)
name=$(basename $sdir)
bdir=/tmp/$USER/opticks/$name/build 

echo bdir $bdir name $name

rm -rf $bdir && mkdir -p $bdir && cd $bdir && pwd 


om-cmake $sdir


make
[ ! $? -eq 0 ] && exit 1

make install   

$name

npd-(){ cat << EOP
import os, numpy as np 
ipath = os.path.expandvars('$TMP/$1/inp.npy')
opath = os.path.expandvars('$TMP/$1/out.npy')
i = np.load(ipath)
o = np.load(opath)
print("npd inp %s %r " % (ipath,i.shape) )
print(i) 
print("npd out %s %r " % (opath,o.shape) )
print(o) 
EOP
} 
#npd- $name | python


cd $sdir
pwd
ipython -i inp_out.py 


