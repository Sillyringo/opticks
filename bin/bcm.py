#!/usr/bin/env python
"""
bcm.py
========

Attempt to parse the CMake/BCM tealeaves in order to 
provide the config information that CMake would 
without using CMake.



* hmm this works fine for the lib64/cmake exported targets, 
  but extracting things from the hand written cmake/Modules
  which are "installed" by okdist--
  is going to be painful and fragile

* maybe can used bcm_deploy to cast those 
  into the standard exported form and parse that ?

  * seems confusing and overcomplicated to do this, 
    plus its depending on my fork of BCM a bit too much 


Actually the critical parts of the FindXXX.cmake are mostly the same::

     20 find_path(
     21     PLog_INCLUDE_DIR 
     22     NAMES "plog/Log.h"
     23     PATHS "${OPTICKS_PREFIX}/externals/plog/include"
     24 )

* implementing a python equivalent that finds the path seems easy enough, 
  but some are complicated like XercesC




The exported targets are all generated by BCM, so they 
should strictly follow patterns.


Inclusion tree of BCM exported targets::

    lib64/cmake/extg4/extg4-config-version.cmake
        just version setup

    lib64/cmake/extg4/extg4-config.cmake
           
        lib64/cmake/extg4/extg4-targets.cmake
             lib64/cmake/extg4/extg4-targets-debug.cmake

        lib64/cmake/extg4/properties-extg4-targets.cmake


properties all have same pattern
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

    [blyth@lxslc701 lib64]$ find . -name properties-*.cmake -exec cat {} \;

    set_target_properties(Opticks::OKOP PROPERTIES INTERFACE_PKG_CONFIG_NAME okop)

    set_target_properties(Opticks::YoctoGLRap PROPERTIES INTERFACE_PKG_CONFIG_NAME yoctoglrap)

    ...


"""

import os, re, logging, sys 
log = logging.getLogger(__name__)


class CMakeTargets(object):
    """
    # Create imported target Opticks::SysRap
    add_library(Opticks::SysRap SHARED IMPORTED)

    set_target_properties(Opticks::SysRap PROPERTIES
      INTERFACE_COMPILE_DEFINITIONS "OPTICKS_SYSRAP"
      INTERFACE_INCLUDE_DIRECTORIES "${_IMPORT_PREFIX}/include/SysRap"
      INTERFACE_LINK_LIBRARIES "Opticks::PLog;Opticks::OKConf;ssl;crypto"
    )
    """ 
    pass

    HEAD_STP = re.compile("^set_target_properties\((?P<lib>\S*)\s*PROPERTIES\s*$")
    BODY_STP = re.compile("\s*(?P<key>\S*)\s*\"(?P<val>\S*)\"\s*$") 
    TAIL_STP = re.compile("^\s*\)\s*$")

    HEAD_PFX = 'get_filename_component(_IMPORT_PREFIX "${CMAKE_CURRENT_LIST_FILE}" PATH)'
    BODY_PFX = 'get_filename_component(_IMPORT_PREFIX "${_IMPORT_PREFIX}" PATH)' 


    def __init__(self, paths ):
        self.paths = paths
        self.targets = {}
        for path in paths:
            self.parse_targets(path) 
        pass 

    def parse_targets(self, path):
        print(path)
        lib = None
        props = {}
        prefix = None
 
        for line in file(path, "r").readlines():

            mhpfx = line.startswith(self.HEAD_PFX)
            mbpfx = line.startswith(self.BODY_PFX)

            if mhpfx:
                prefix = os.path.dirname(path)
            elif mbpfx:
                assert not prefix is None
                prefix = os.path.dirname(prefix)
            pass    

            mhead = self.HEAD_STP.match(line)  
            mbody = self.BODY_STP.match(line)  
            mtail = self.TAIL_STP.match(line)  

            if mhead:
                lib = mhead.groupdict()["lib"]
                if not prefix is None:
                    self.prefix = prefix 
                pass
                #print("lib  %s prefix %s  " % (lib, self.prefix))
            elif mbody and not lib is None:
                d = mbody.groupdict()
                key, val = d["key"], d["val"]
                #print( "key %s val %s prefix %s " % (key, val, prefix))
                if not self.prefix is None:
                    val = val.replace("${_IMPORT_PREFIX}", self.prefix) 
                pass
                props[key] = val 
            elif mtail:
                assert not lib is None
                print( "lib %s props %s " % (lib, repr(props)))
                self.targets[lib] =  props.copy() 
                props.clear()
                lib = None
            else:
                pass
            pass 
        pass

    def __repr__(self):
        return self.targets 
 

class CMakeConfig(object):

    LIB = re.compile("^# Library: (?P<lib>\S*)\s*$")
    DEP = re.compile("^find_dependency\((?P<dep>.*)\)\s*$")

    def __init__(self, path, pfx):
        assert os.path.exists(path) 
        targets_path = path.replace("-config.cmake","-targets.cmake")
        targets_debug_path = path.replace("-config.cmake","-targets-debug.cmake")

        assert os.path.exists(targets_path) 
        assert os.path.exists(targets_debug_path) 

        self.path = path 
        self.pfx = pfx
        self.parse_config(path)
        self.targets = CMakeTargets([targets_path, targets_debug_path])

    def parse_config(self, path):
        print(path) 
        for line in file(path, "r").readlines():
            mlib = self.LIB.match(line)
            mdep = self.DEP.match(line)
            if mlib:
                lib = mlib.groupdict()["lib"]
                print("lib %s " % lib) 
            if mdep:
                dep = mdep.groupdict()["dep"]
                print("dep %s " % dep) 
            pass 
        pass   

    def __repr__(self):
        return self.path 
 

class CMakeExport(object):
    CONFIG = re.compile("(?P<pfx>\S*)-config.cmake")
    def __init__(self, base):
        self.base = base 
        self.cfg = {}   
        self.find_config() 

    def find_config(self):
        self.find_config_r(self.base, 0) 
 
    def find_config_r(self, base, depth ):
        assert os.path.isdir(base), "expected directory %s does not exist " % base
        log.info("base %s depth %s " % (base, depth))
        names = os.listdir(base)
        for name in names:
            path = os.path.join(base, name)
            if os.path.isdir(path):
                self.find_config_r(path, depth+1)
            else:
                m = self.CONFIG.match(name)
                if not m: continue
                pfx = m.groupdict()['pfx']
                cfg = CMakeConfig(path, pfx) 
                self.cfg[pfx] = cfg
            pass
        pass


if __name__ == '__main__':

    base = sys.argv[1] if len(sys.argv) > 1 else os.path.expandvars("$OPTICKS_INSTALL_PREFIX/lib64/cmake")
    cx = CMakeExport(base)    

