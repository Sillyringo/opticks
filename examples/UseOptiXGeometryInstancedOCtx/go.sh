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

echo "## opticks- "
opticks-
#oe-
echo "##[ om- : all output redirected in oe- to stderr:2  "
om- 2> /dev/null
echo "##] om- "

sdir=$(pwd)
name=$(basename $sdir)
bdir=/tmp/$USER/opticks/$name/build 
msg="=== $0 :"

echo $msg bdir $bdir name $name

remake=0

# Note that on macOS some auto (or reboot) cleanup seems to delete 
# all files within /tmp directories without deleting the directories
# so check for Makefile existance, not bdir, to trigger reconfig.

if [ $remake -eq 1 -o ! -f "$bdir/Makefile" ]; then  
    rm -rf $bdir && mkdir -p $bdir 
    cd $bdir && pwd 
    om-cmake $sdir

    make
    [ ! $? -eq 0 ] && echo build error && exit 1
    make install   
else
    echo $msg not reconfig : proceed to bdir $bdir
    cd $bdir && pwd 
fi 

earth=$HOME/opticks_refs/Earth_Albedo_8192_4096.ppm
gradient=/tmp/SPPMTest_MakeTestImage.ppm
if [ -f "$earth" ]; then 
    path=$earth  
else
    path=$gradient
fi
exe=$(which $name)

[ -z "$exe" ] && echo $msg no $name in path && exit 1 


runpath=/tmp/octx.sh

cd $sdir

cat << EOR > $runpath
#!/bin/bash -l
## $(date)
## this script $runpath script was generated by go.sh from $PWD 

opticks-
echo "##[ oe- : all output redirected in oe- to stderr:2  "
oe- 2> /dev/null
echo "##] oe- "

cmdline="$exe $path \$*"
echo \$cmdline
eval \$cmdline

[ ! \$? -eq 0 ] && echo \$0 : runtime error && exit 1

outdir=/tmp/$USER/opticks/$name
outpath=\$outdir/pixels.ppm
ls -l \$outdir
date

if [ -n "\$SSH_TTY" ]; then 
    echo remote running : outpath $outpath
else 
    echo local running : open outpath $outpath
    open \$outpath
fi
echo lldb_ \$cmdline

# needs numpy 
python3 intersect_sdf_test.py --tmpdir \$outdir
#python3 intersect_sdf_test.py --tmpdir \$outdir --level debug

which IntersectSDFTest  ## from okc-
IntersectSDFTest \$outdir


rc=\$?
echo rc \$rc

EOR

chmod ugo+x $runpath
ls -l $runpath
#cat $runpath

#$runpath
#$runpath textured,tex0
#$runpath textured,tex1
$runpath textured,tex2


