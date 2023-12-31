#!/bin/bash  -l

g4-
clhep-

# clhep- overrides  as clhep-prefix/ver do not match what the geant4 was built against 
clhep-prefix(){  echo /usr/local/opticks_externals/clhep ; }
clhep-ver(){    echo 2.4.1.0 ; }


usage(){ cat << EOU

In addition to geant4 and clhep this also uses 
the NP.hh header from https://github.com/simoncblyth/np/ 

EOU
}

compile(){ 
    local name=$1 ; 
    mkdir -p /tmp/$name
    cat << EOC
    gcc $name.cc -std=c++11 \
       -I. \
       -g \
       -I$HOME/np \
       -I$(g4-prefix)/include/Geant4 \
       -I$(clhep-prefix)/include \
       -L$(g4-prefix)/lib \
       -L$(clhep-prefix)/lib \
       -lstdc++ \
       -lG4global \
       -lG4materials \
       -lCLHEP-$(clhep-ver) \
       -o /tmp/$name/$name 
EOC
}

run(){  
    local name=$1 ; 
    local var 
    case $(uname) in 
       Darwin) var=DYLD_LIBRARY_PATH ;;
        Linux) var=LD_LIBRARY_PATH ;;
    esac

    case $(uname) in 
       Darwin) DEBUG=lldb__ ;;
        Linux) DEBUG=gdb    ;;
    esac
    DEBUG=

    cat << EOC
$var=$(g4-prefix)/lib:$(clhep-prefix)/lib $DEBUG /tmp/$name/$name 
EOC

}


name=L4CerenkovTest

if [ -n "$SCAN" ]; then 
    docompile=0
    interactive=0
else
    docompile=1
    interactive=1
fi 


if [ $docompile -eq 1 ]; then
    compile $name
    eval $(compile $name)
    [ $? -ne 0 ] && echo compile FAIL && exit 1 
fi 


run $name 
eval $(run $name) $*
[ $? -ne 0 ] && echo run FAIL && exit 2
echo run succeeds


if [ $interactive -eq 1 ]; then 
    ipython -i $name.py
    [ $? -ne 0 ] && echo analysis FAIL && exit 3
else
    python $name.py
    [ $? -ne 0 ] && echo analysis FAIL && exit 3
fi



exit 0 

