#!/bin/bash -l 
usage(){ cat << EOU
nmskSolidMask.sh
==================

EOU
}

echo $BASH_SOURCE 


#selection=176995,153452,459970
#selection=176995

#selection=153452,459970
selection=153452

GEOM=nmskSolidMask SELECTION=$selection ./CSGSimtraceRerunTest.sh 

