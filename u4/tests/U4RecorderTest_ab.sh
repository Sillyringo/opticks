#!/bin/bash -l 

usage(){ cat << EOU
U4RecorderTest_ab.sh
=======================

::

    u4t
    ./U4RecorderTest_ab.sh


EOU
}

#export FOLD_MODE=TMP
export FOLD_MODE=KEEP

source ../../bin/AB_FOLD.sh 

${IPYTHON:-ipython} --pdb -i U4RecorderTest_ab.py $*  




