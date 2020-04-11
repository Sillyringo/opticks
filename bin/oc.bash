oc-source(){ echo $BASH_SOURCE ; }
oc-vi(){ vi $(oc-source) ; }
oc-env(){  olocal- ; opticks- ; }
oc-help(){ cat << EOH

opticks-config
===============

Provides cflags, libs and lippath for all opticks externals
and sub-packages based on underlying pkg-config .pc files
which are generated from the Opticks CMake/bash infrastructure.  

This allows C++ code using any of the Opticks packages to 
be compiled, linked and executed without using CMake.  
Many demonstrations of usage are in examples/Use*NoCMake/go.sh  

Usage examples:

    opticks-config --cflags NPY
    opticks-config --libs   NPY
    opticks-config --libpath NPY

Both opticks-config and oc are symbolic links to oc.bash::

    oc -cflags NPY
    oc -f NPY
    oc -f npy 
    oc --help

The pkg arguments are names such as NPY, optix, OptiXRap, PLog, G4
which are intepreted case insensitively.
If the pkg name is not recognized an error message from 
pkg-config is returned.

EOH
}

oc-usage(){ cat << EOU

opticks-config notes for developers
===========================================================

Overview for Developers
-------------------------

Note that this file acts as both a sourced collection 
of bash functions ~/opticks/bin/oc.bash for development oc-vi
and as a deployed bash script when used via the installed 
\$(opticks-prefix)/bin/oc OR \$(opticks-prefix)/bin/opticks-config 
scripts which are actually symbolic links to the installed 
version of this file \$(opticks-prefix)/bin/oc.bash

Updating and Usage
---------------------

Update::
 
    oc-
    oc-vi

    cd ~/opticks/bin
    om-
    om--  

The last command installs oc.bash into the CMAKE_INSTALL_PREFIX/bin 
and plants symbolic links.

Developer Usage via bash functions
-------------------------------------

Use as bash functions::

   oc-
   oc-cflags NPY 
   oc-vi         # editing from the source tree 

Contrast with enduser usage via script::

   oc -cflags NPY
   oc --cflags NPY


TODO : Avoid manual pc edits 
-----------------------------

Decided that a kludge with bin/pc.py following installation 
is best way to handle this allowing a common approach to the 
three afflicted externals : assimp, gflw3, glew 

Edits move the "externals" from the prefix into the libdir and includedir,
in order for prefix to have the same meaning for both internals and externals, 
aiming for ease of relocatability by changing prefix when operating from 
distributions::

   /usr/local/opticks/externals/lib/pkgconfig/assimp.pc
   /usr/local/opticks/externals/lib/pkgconfig/glfw3.pc
   /usr/local/opticks/externals/lib/pkgconfig/glew.pc

Overriding of INCLUDEDIR and LIBDIR are demonstated in the CMakeLists.txt 
if odcs- and oyoctogl- with::

    include(GNUInstallDirs)
    set(CMAKE_INSTALL_INCLUDEDIR "externals/include/${name}")
    set(CMAKE_INSTALL_LIBDIR     "externals/lib")
    set(CMAKE_INSTALL_BINDIR     "lib")

BUT it Looks difficult to induce the standard generation to create what want, 
so need an after the install fix in a assimp-pc glfw-pc glew-pc 
invoked at install from  assimp-- glfw-- glew--

Probaby a python script that parses the pc variables would be 
the obvious way to kludge it.


assimp
~~~~~~~~

::

    epsilon:assimp-fork blyth$ cat assimp.pc.in
    prefix=@CMAKE_INSTALL_PREFIX@
    exec_prefix=@CMAKE_INSTALL_PREFIX@/@ASSIMP_BIN_INSTALL_DIR@
    libdir=@CMAKE_INSTALL_PREFIX@/@ASSIMP_LIB_INSTALL_DIR@
    includedir=@CMAKE_INSTALL_PREFIX@/@ASSIMP_INCLUDE_INSTALL_DIR@/assimp

    Name: @CMAKE_PROJECT_NAME@
    Description: Import various well-known 3D model formats in an uniform manner.
    Version: @PROJECT_VERSION@
    Libs: -L${libdir} -lassimp@ASSIMP_LIBRARY_SUFFIX@
    Libs.private: @LIBSTDC++_LIBRARIES@ @ZLIB_LIBRARIES_LINKED@
    Cflags: -I${includedir}

glew
~~~~~

build/cmake/CMakeLists.txt::

    107 set (requireslib glu)
    108 configure_file (${GLEW_DIR}/glew.pc.in ${GLEW_DIR}/glew.pc @ONLY)

::

    epsilon:glew-1.13.0 blyth$ cat glew.pc.in 
    prefix=@prefix@
    exec_prefix=${prefix}
    libdir=@libdir@
    includedir=${prefix}/include

    Name: glew
    Description: The OpenGL Extension Wrangler library
    Version: @version@
    Cflags: -I${includedir} @cflags@
    Libs: -L${libdir} -l@libname@
    Requires: @requireslib@


glfw : pc via CMakePackageConfigHelpers
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

* https://cmake.org/cmake/help/latest/module/CMakePackageConfigHelpers.html

Despite the name that is not for pkg-config

::

    514 include(CMakePackageConfigHelpers)
    515 
    516 if (UNIX)
    517     set(GLFW_CONFIG_PATH "${CMAKE_INSTALL_PREFIX}/lib/cmake/glfw3/")
    518 else()
    519     set(GLFW_CONFIG_PATH "${CMAKE_INSTALL_PREFIX}/")
    520 endif()
    521 
    522 configure_package_config_file("${GLFW_SOURCE_DIR}/src/glfw3Config.cmake.in"
    523                               "${GLFW_BINARY_DIR}/src/glfw3Config.cmake"
    524                               INSTALL_DESTINATION "${GLFW_CONFIG_PATH}"
    525                               PATH_VARS CMAKE_INSTALL_PREFIX
    526                               NO_CHECK_REQUIRED_COMPONENTS_MACRO)
    527 
    528 write_basic_package_version_file("${GLFW_BINARY_DIR}/src/glfw3ConfigVersion.cmake"
    529                                  VERSION ${GLFW_VERSION_FULL}
    530                                  COMPATIBILITY SameMajorVersion)
    531 
    532 if (GLFW_BUILD_DOCS)
    533     configure_file("${GLFW_SOURCE_DIR}/docs/Doxyfile.in"
    534                    "${GLFW_BINARY_DIR}/docs/Doxyfile" @ONLY)
    535 endif()
    536 
    537 configure_file("${GLFW_SOURCE_DIR}/src/glfw_config.h.in"
    538                "${GLFW_BINARY_DIR}/src/glfw_config.h" @ONLY)
    539 
    540 configure_file("${GLFW_SOURCE_DIR}/src/glfw3.pc.in"
    541                "${GLFW_BINARY_DIR}/src/glfw3.pc" @ONLY)

    ## @ONLY means only substitute @VAR@ 

::

    epsilon:glfw-3.1.1 blyth$ cat src/glfw3.pc.in
    prefix=@CMAKE_INSTALL_PREFIX@
    exec_prefix=${prefix}
    includedir=${prefix}/include
    libdir=${exec_prefix}/lib@LIB_SUFFIX@

    Name: GLFW
    Description: A multi-platform library for OpenGL, window and input
    Version: @GLFW_VERSION_FULL@
    URL: http://www.glfw.org/
    Requires.private: @GLFW_PKG_DEPS@
    Libs: -L${libdir} -l@GLFW_LIB_NAME@
    Libs.private: @GLFW_PKG_LIBS@
    Cflags: -I${includedir}



TODO : OptiXRap Linux linker warning
---------------------------------------

::

    /usr/bin/ld: warning: liboptix_prime.so.6.5.0, needed by /home/blyth/local/opticks/lib64/libOptiXRap.so, not found (try using -rpath or -rpath-link)


TODO : operation with non-Opticks controlled boost and G4 
------------------------------------------------------------



TODO : regularize imgui CMakeLists.txt its not using bcm_deploy forcing manual pc
-----------------------------------------------------------------------------------

TODO : pc Libs.private ?
--------------------------

NPY : had to make all libs PUBLIC for UseNPYNoCMake to work on Linux 


* https://stackoverflow.com/questions/45334645/which-cmake-property-should-hold-privately-linked-shared-libraries-for-imported
* https://stackoverflow.com/questions/32756195/recursive-list-of-link-libraries-in-cmake
* https://gitlab.kitware.com/cmake/cmake/issues/12435


TODO : get relocatable operation from a distributed installation to work
-------------------------------------------------------------------------- 

Cannot use "--define-prefix" for this as that demands a 
newer version of pkg-config that is commonly available.


pkg-config versions
---------------------

* https://www.freedesktop.org/wiki/Software/pkg-config/
* https://pkg-config.freedesktop.org/releases/

::

    [blyth@localhost UseSysRapNoCMake]$ pkg-config --version  ## from pkgconfig-0.27.1-4.el7.i686 
    0.27.1
    ## arghh : from 2012 : 0.27.1 doesnt have --define-prefix

    epsilon:cfg4 blyth$ pkg-config --version
    0.29.2


Requirements for pkg-config hookup
-------------------------------------

1. all packages (internal and external) need a .pc file to be 
   installed into lib/pkgconfig or externals/lib/pkgconfig : often 
   the simplest way to do that is via the bash functions that install
   the package 
  

2. CMake machinery needs to be informed for externals by addition 
   of the INTERFACE_PKG_CONFIG_NAME property to found targets, this 
   is most conveniently done in cmake/modules/FindName.cmake 

   The name corresponding to pkg-config pc file eg glm.pc, assimp.pc 

::

     39     set_target_properties(${_tgt} PROPERTIES
     40         INTERFACE_INCLUDE_DIRECTORIES "${GLM_INCLUDE_DIR}"
     41         INTERFACE_PKG_CONFIG_NAME "glm"
     42     )


   Despite pkg-config being usable without CMake the Opticks 
   build remains CMake based and most of the .pc files are generated
   by the BCMPkgConfig CMake machinery. It is because of this that 
   the CMake build needs to the INTERFACE_PKG_CONFIG_NAME for 
   inclusion into the generated pc files.

   After adding that property, need to rebuild and install 
   packages that use the dependency in order to re-generate the pc files.


Typical Usage
----------------

::

    opticks-
    oc-

    pkg=OpenMeshRap

    gcc -c $sdir/Use$pkg.cc $(oc-cflags $pkg)
    gcc Use$pkg.o -o Use$pkg $(oc-libs $pkg) 
    LD_LIBRARY_PATH=$(oc-libpath $pkg) ./Use$pkg


Hmm always using define-prefix means have to get rid of prefix var in below ?
------------------------------------------------------------------------------

* suppose use of macports xerces-c means have to add /opt/local/lib/pkgconfig 
  to PKG_CONFIG_PATH so --define-prefix then can operate correctly without changes
  to the pc

* hmm but its problematic from a cross platform point of view

::

    epsilon:UseOpticksXercesC blyth$ cat /opt/local/lib/pkgconfig/xerces-c.pc
    prefix=/opt/local
    exec_prefix=${prefix}
    libdir=${exec_prefix}/lib
    includedir=${prefix}/include

    Name: Xerces-C++
    Description: Validating XML parser library for C++
    Version: 3.2.1
    Libs: -L${libdir} -lxerces-c
    Libs.private: -lcurl
    Cflags: -I${includedir}


FIXED : define-prefix is scrubbing the CUDA include dir ?
-----------------------------------------------------------

define-prefix assumes the prefix can be obtained from the grandparent
dir of the /usr/local/opticks/xlib/pkgconfig/optickscuda.pc 
which yeilds /usr/local/opticks. This replaces the prefix variable 
if there is one defined in the pc file.

/usr/local/opticks/xlib/pkgconfig/optickscuda.pc::

    prefix=/usr/local/cuda
    includedir=${prefix}/include
    libdir=${prefix}/lib

    Name: CUDA
    Description: 
    Version: 9.1 
    Libs: -L${libdir} -lcudart -lcurand
    Cflags: -I${includedir}

The result is the wrong prefix.::

    epsilon:UseCUDARapNoCMake blyth$ oc-pkg-config optickscuda --cflags
    -I/usr/local/cuda/include

    epsilon:UseCUDARapNoCMake blyth$ oc-pkg-config optickscuda --cflags --define-prefix
    -I/usr/local/opticks/include

One solution is to exclude the prefix variable in pc files
of packages that are not going to be part of the distribution.  
This prevents --define-prefix from having any effect.

EOU
} 


#oc-extra(){ echo --define-prefix ; }  ## --define-prefix is a "recent" pkg-config addition, so better not to use it 
oc-extra(){ echo ; }
oc-lower(){ 
   local arg
   local larg
   for arg in "$@"
   do
      larg=$(echo $arg | tr "[A-Z]" "[a-z]")
      printf "%s " $larg
   done
}

oc-args-(){
   local arg
   for arg in "$@"
   do
      printf "%s\n" $arg
   done
}

oc-args(){ oc-args- $(oc-lower $*) ; }

oc-libdir-(){ oc-pkg-config $(oc-lower $*) --libs-only-L $(oc-extra) | tr -- "-L" " " ; }
oc-cflags-(){ oc-pkg-config $(oc-lower $*) --cflags $(oc-extra) ; }


oc-cflags(){ echo $(oc-cflags- $*) -std=c++11 ; }
oc-libs(){   oc-pkg-config $(oc-lower $*) --libs   $(oc-extra) ; }
oc-libsl(){  oc-pkg-config $(oc-lower $*) --libs-only-L $(oc-extra) ; }
oc-deps(){   oc-pkg-config $(oc-lower $*) --print-requires  ; }
oc-dump(){   oc-pkg-config-dump $(oc-lower $*) ; }
oc-check(){  oc-pkg-config-check-dirs $(oc-lower $*) ; }
oc-find(){   oc-pkg-config-find $(oc-lower $*) ; }

oc-libdir(){  oc-libdir- $* | tr " " "\n" | sort | uniq ; }
oc-libpath(){ local dirs=$(oc-libdir $*) ; echo $dirs | tr " " ":" ; }

oc-cat(){    
   local pc=$(oc-find $*) 
   [ -n "$pc" -a -f "$pc" ] && cat $pc
}
oc-edit(){    
   local pc=$(oc-find $*) 
   [ -n "$pc" -a -f "$pc" ] && vi $pc
}

oc-setup()
{
   : TODO eliminate this transient  
   local iwd=$pwd
   cd $(opticks-prefix)
   [ ! -x xlib ] && ln -s externals/lib xlib    ## now that cannot use --define-prefix not needed ?
   cd $iwd 

   ## the below is transient, they should be done on installing  
 
   plog-
   plog-pc
   glm-
   glm-pc 
   openmesh-
   openmesh-pc
   imgui-
   imgui-pc
   optix-
   optix-pc
   cuda-
   cuda-pc
}



oc-prefix-notes(){ cat << EON

opticks-prefix is only defined when sourced 
when run from a script like ./go.sh that becomes $0

When using these functions from oc or opticks-config plucked
off the PATH the prefix should be determined from the path 
to this script, otherwise rely on having opticks-prefix function.

EON
}

oc-prefix(){
   local arg=$0
   if [ "${arg:(-2)}" == "oc" -o "${arg:(-14)}" == "opticks-config" ]
   then
       echo $(dirname $(dirname $0)) 
   else
       opticks-prefix
   fi
}


# "private" interface
oc-pkg-config-path--(){ cat << EOP
$(oc-prefix)/lib/pkgconfig
$(oc-prefix)/lib64/pkgconfig
$(oc-prefix)/xlib/pkgconfig
/opt/local/lib/pkgconfig
EOP
}
oc-pkg-config-path-(){
   $FUNCNAME- | while read dir ; do
      [ -d "$dir" ] && echo $dir
   done
}
oc-pkg-config-path(){ echo $(oc-pkg-config-path-) | tr " " ":" ; }


oc-info(){
   local pkg=${1:-NPY}

   cat << EOI

oc-info $pkg
========================

   \$0                : $0
   oc-prefix          : $(oc-prefix)
   oc-pkg-config-path : $(oc-pkg-config-path)

   oc-find $pkg : $(oc-find $pkg) 
   oc-cat $pkg  : 

   $(oc-cat $pkg) 


EOI

}


oc-pkg-config(){ PKG_CONFIG_PATH=$(oc-pkg-config-path) pkg-config $* ; }
oc-pcfix(){      PKG_CONFIG_PATH=$(oc-pkg-config-path) pc.py $* ; }


oc-pkg-config-find(){
   local pkg=${1:-NPY}
   local lpkg=$(echo $pkg | tr [A-Z] [a-z])

   local dir
   local pc
   oc-pkg-config-path- | while read dir ; do 
      pc=$dir/${lpkg}.pc
      if [ -f "$pc" ]; then
         echo $pc
      fi 
   done
}

oc-pkg-config-dump(){
   local pkg=${1:-NPY}
   local opt
   $FUNCNAME-opts- | while read opt ; do 
       cmd="oc-pkg-config $pkg $opt"  
       printf "\n\n# %s\n\n"  "$cmd"
       $cmd | tr " " "\n"
   done
   oc-info $pkg
}

oc-pkg-config-dump-opts-(){ cat << EOC
--print-requires
--cflags 
--cflags-only-I 
--libs 
EOC

cat << EOO > /dev/null
--print-requires --define-prefix
--cflags --define-prefix
--libs --define-prefix
--cflags-only-I --define-prefix
EOO

}

oc-pkg-config-check-dirs(){
   local pkg=${1:-NPY}
   local line 
   local dir
   local exists 
   oc-pkg-config $pkg --cflags-only-I --define-prefix | tr " " "\n" | while read line ; do 
     dir=${line:2} 
     [ -d "$dir" ] && exists="Y" || exists="N"  
     printf " %s : %s \n" $exists $dir 
   done 
}


oc-main(){
   if [ "$#" == "0" ]; then
       oc-help 
   else 
       local cmd=$1
       shift
       #printf "cmd:%s:\n" $cmd 
       case $cmd in 
         --cflags|-cflags|-f) oc-cflags $* ;; 
             --libs|-libs|-l) oc-libs $* ;; 
       --libpath|-libpath|-p) oc-libpath $* ;; 
             --dump|-dump|-d) oc-dump $* ;; 
             --find|-find|-n) oc-find $* ;; 
               --cat|-cat|-c) oc-cat  $* ;; 
             --info|-info|-i) oc-info  $* ;; 
             --help|-help|-h) oc-help $* ;; 
                           *) oc-dump $cmd $* ;;   ## assume unrecognized command is a pkg name 
       esac
   fi 
}


if [ ! "$0" == "-bash" -o "$0" == "bash" ]; then   ## when used as a script rather than being sourced 
   [ "$(basename $0)" == "opticks-config" -o "$(basename $0)" == "oc"  ] && oc-main $*
fi

