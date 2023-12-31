#!/bin/bash -l 

usage(){ cat << EOU
sysrap/tests/S4MTRandGaussQTest.sh
====================================

Related scripts:

sysrap/tests/gaussQTables.sh 
   get familiar with Geant4 gaussTable 

sysrap/tests/S4MTRandGaussQTest.sh
   using the table to implement S4MTRandGaussQTest::transformQuick 

sysrap/tests/erfcinvf_Test.sh 
   getting good match to S4MTRandGaussQTest::transformQuick on device with erfcinvf 

EOU
}


cd $(dirname $BASH_SOURCE)

name=S4MTRandGaussQTest

clhep-
g4-

defarg="info_build_run_ana"
arg=${1:-$defarg}

export FOLD=/tmp/$name
mkdir -p $FOLD
bin=$FOLD/$name

vars="BASH_SOURCE name FOLD bin"

if [ "${arg/info}" != "$arg" ]; then 
    for var in $vars ; do printf "%20s : %s \n" "$var" "${!var}" ; done 
fi 

if [ "${arg/build}" != "$arg" ]; then 
    gcc $name.cc \
       S4MTRandGaussQ.cc \
       -I.. \
       -I$(clhep-prefix)/include \
       -I$(g4-prefix)/include/Geant4  \
       -L$(clhep-prefix)/lib \
       -lCLHEP \
       -std=c++11 -lstdc++ \
       -o $bin
    [ $? -ne 0 ] && echo $BASH_SOURCE : build error && exit 1 
fi

if [ "${arg/run}" != "$arg" ]; then 
    $bin
    [ $? -ne 0 ] && echo $BASH_SOURCE : build error && exit 2
fi

if [ "${arg/ana}" != "$arg" ]; then 
    ${IPYTHON:-ipython} --pdb -i $name.py 
    [ $? -ne 0 ] && echo $BASH_SOURCE : ana error && exit 3
fi

exit 0 


