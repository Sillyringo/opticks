#!/bin/bash -l 

usage(){ cat << EOU
CSGGenstepTest.sh
===================

CSGGenstepTest checks the center-extent-gensteps used in CSGOptiX/cxs.sh 
by generating some photons on CPU from them and loading into python. 




2022-01-03 15:04:33.973 INFO  [5323710] [CSGGenstep::locate@115]  rc 0 MOI.ce (-10093.4 11882.9 11467.3 365.057)
2022-01-03 15:04:33.973 INFO  [5323710] [CSGGenstep::locate@118] 
qt( 0.384,-0.452, 0.806, 0.000) (-0.762,-0.647, 0.000, 0.000) ( 0.522,-0.614,-0.593, 0.000) (-10135.104,11931.969,11514.693, 1.000) 

EOU
}


#geom=HamaXZ_0
#geom=HamaXZ_1000
geom=XJfixtureConstruction_0

export GEOM=${GEOM:-$geom}
ce_offset=0

if [ "$GEOM" == "HamaXZ_0" ]; then

    moi=Hama
    cegs=16:0:9:500   
    gridscale=0.10
    ce_offset=0

elif [ "$GEOM" == "HamaXZ_1000" ]; then

    moi=Hama:0:1000
    cegs=16:0:9:500   
    gridscale=0.10
    ce_offset=0

elif [ "$GEOM" == "XJfixtureConstruction_0" ]; then


    ## see CSGTarget::getCenterExtent

    #iidx=0       # default 
    #iidx=-1
    #iidx=-2
    iidx=-3       # model2world_rtpw = translate * scale * rotate 
    #iidx=-4
    #iidx=-5

    moi="solidXJfixture:10:$iidx"

    #cegs=16:0:9:100                # XZ/RP     (XYZ)->(RTP) 
    cegs=0:16:9:100                 # YZ/TP
    gridscale=0.05

    #ce_offset=1   # already in the transform
    ce_offset=0
fi 


export MOI=${MOI:-$moi}
export CXS_CEGS=${CXS_CEGS:-$cegs}
export CE_OFFSET=${CE_OFFSET:-$ce_offset}
export GRIDSCALE=${GRIDSCALE:-$gridscale}


arg=${1:-run_ana}

if [ "${arg/run}" != "$arg" ]; then  
    if [ "$(uname)" == "Darwin" ]; then
        lldb__ CSGGenstepTest
    else
        CSGGenstepTest
    fi 
    [ $? -ne 0 ] && echo $msg runtime error && exit 1 
fi

if [ "${arg/ana}" != "$arg" ]; then  
    ${IPYTHON:-ipython} -i CSGGenstepTest.py 
fi

exit 0 


