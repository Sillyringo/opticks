#!/bin/bash -l 
usage(){ cat << EOU
ct.sh : using CSGSimtraceTest
===============================


1. load single solid GEOM from CSGFoundry folder, normally created by GeoChain/mtranslate.sh 
2. load center-extent gensteps, just like X4SimtraceTest.cc does 
3. use CSG intersection headers to get intersect positions using CUDA compatible code on the CPU 


::

   MPPLT_SIMTRACE_SELECTION_LINE=o2i ./ct.sh ana


EOU
}

export CSGSimtrace=INFO


arg=${1:-run_ana}

bin=CSGSimtraceTest
log=$bin.log

source $(dirname $BASH_SOURCE)/../bin/COMMON.sh
unset OPTICKS_INPUT_PHOTON 


export FOLD=/tmp/$USER/opticks/$GEOM/$bin/ALL

if [ "$GEOM" == "nmskSolidMaskTail__U1" ]; then 
   export SELECTION=495871
   #export SELECTION=495871,512880
   #export FOCUS=257,-39,7
fi 

export TOPLINE="CSG/ct.sh GEOM $GEOM FOCUS $FOCUS"


if [ "${arg/info}" != "$arg" ]; then 
    vars="BASH_SOURCE arg bin GEOM FOLD"
    for var in $vars ; do printf "%30s : %s \n" $var ${!var} ; done 
fi

if [ "${arg/run}" != "$arg" ]; then
    [ -f "$log" ] && rm $log 
    $bin
    [ $? -ne 0 ] && echo $BASH_SOURCE run error && exit 1 
fi  

if [ "${arg/dbg}" != "$arg" ]; then
    case $(uname) in
        Darwin) lldb__ $bin ;;
        Linux)  gdb__ $bin ;;
    esac
    [ $? -ne 0 ] && echo $BASH_SOURCE dbg error && exit 2
fi

if [ "${arg/ana}"  != "$arg" ]; then
    ${IPYTHON:-ipython} --pdb -i $(dirname $BASH_SOURCE)/tests/$bin.py
    [ $? -ne 0 ] && echo $BASH_SOURCE ana interactive error && exit 3
fi

if [ "$arg" == "mpcap" -o "$arg" == "mppub" ]; then
    export CAP_BASE=$FOLD/figs
    export CAP_REL=ct
    export CAP_STEM=${GEOM}
    case $arg in  
       mpcap) source mpcap.sh cap  ;;  
       mppub) source mpcap.sh env  ;;  
    esac

    if [ "$arg" == "mppub" ]; then 
        source epub.sh 
    fi  
fi 

exit 0
