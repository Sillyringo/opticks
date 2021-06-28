#!/bin/bash -l 

usage(){ cat << EOU
ab.sh
======

::

   ab.sh 1 --nocompare   # early stage debugging 

   ab.sh 1 
   ab.sh 2

   PFX=tds3ip ab.sh 1 


EOU
}


cat="g4live"
det="det"
src="natural"
pfx=${PFX:-source}

tag=${1:-1}
shift 

export OPTICKS_ANA_DEFAULTS="src=$src,cat=$cat,det=$det,tag=$tag,pfx=$pfx"

#script=evt.py
#script=ab.py 
script=abs.py 
#script=profile_.py 

cd ~/opticks/ana

ipython -i --pdb $(which $script) -- $*


