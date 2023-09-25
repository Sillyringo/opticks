G4CXOpticks/index
==================

.. From copilot
This rst file seems to be a documentation file for a project related to simulating and tracing 3D models using Geant4 and Opticks. The file includes a list of shell scripts and their descriptions, which likely contain commands for running simulations and tracing rays through the models.
g4cx likely refers to Geant4CX, which is a Geant4-based simulation framework that allows for the integration of Geant4 simulations with Opticks ray tracing.

render
--------

gxr.sh
    G4CXRenderTest : G4CXOpticks::SetGeometry then render


simulate
---------

gxs.sh
    G4CXSimulateTest : G4CXOpticks::SetGeometry then simulate

gxs_ab.sh
    maybe replaced by ab.sh 

ab.sh
    python comparison of gxs fold


simtrace
---------

gxt.sh
    G4CXSimtraceTest : G4CXOpticks::SetGeometry then simtrace

mgxt.sh
    loop over geomlist setting GEOM and invoking gxt.sh 

cf_gxt.sh
    python comparison of simtrace from three geometries 

hama.sh nnvt.sh
    gxt.sh ana with view settings


collective
------------

gx.sh
    invokes gxs.sh gxt.sh gxr.sh one after the other 
 

