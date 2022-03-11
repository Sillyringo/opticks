#!/bin/bash -l 
usage(){ cat << EOU
CSGPrimTest.sh
=================

The arguments are passed to geocache_hookup to select the geometry to use. 

old
    some old reference geometry 
new
    recent addition
last
    latest development version  


EOU
}


source $OPTICKS_HOME/bin/geocache_hookup.sh ${1:-new}

${IPYTHON:-ipython} -i --pdb -- tests/CSGPrimTest.py 
