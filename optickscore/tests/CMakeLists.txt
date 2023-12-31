cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
set(name OpticksCoreTest)
project(${name} VERSION 0.1.0)
include(OpticksBuildOptions)


set(TEST_SOURCES

    OpticksQueryTest.cc
    Opticks_getFoundryBase.cc
    IndexerTest.cc
    CameraTest.cc
    CameraSwiftTest.cc
    AnimatorTest.cc
    TrackballTest.cc
    OpticksEventSpecTest.cc
    OpticksFlagsTest.cc
    OpticksPhotonFlagsTest.cc
    OpticksColorsTest.cc
    OpticksCfgTest.cc
    OpticksCfg2Test.cc
    OpticksTest.cc
    OpticksTwoTest.cc
    OpticksResourceTest.cc
    OpticksBufferControlTest.cc
    OpticksBufferSpecTest.cc
    OpticksActionControlTest.cc

    OpticksProfileTest.cc
    OK_PROFILE_Test.cc
    OpticksIDPATH.cc
    OpticksAnaTest.cc
    OpticksDbgTest.cc
    OpticksSwitchesTest.cc

    CompositionTest.cc
    Composition_vs_SGLM_Test.cc
    ViewTest.cc
    CtrlTest.cc

    EvtLoadTest.cc
    OpticksEventAnaTest.cc
    OpticksEventCompareTest.cc
    OpticksEventDumpTest.cc
    OpticksGenstepTest.cc 

    IntersectSDFTest.cc
    OpticksIdentityTest.cc
    OpticksShapeTest.cc

    SphereOfTransformsTest.cc
    CfgTest.cc

    SensorLibTest.cc
    MockSensorAngularEfficiencyTableTest.cc
    MockSensorLibTest.cc
    OpticksEventTest.cc
    OpticksEventLeakTest.cc
    OpticksRunTest.cc

    FlightPathTest.cc
    Opticks_getOutPathTest.cc

)


set(TEST_SOURCES_NOK 


    OpticksAppTest.cc

    NGunConfigTest.cc

    BookmarksTest.cc
    InterpolatedViewTest.cc
)

foreach(TEST_SOURCE ${TEST_SOURCES})
    get_filename_component(TGT ${TEST_SOURCE} NAME_WE)
    add_executable(${TGT} ${TEST_SOURCE})

    if (${TGT} MATCHES ".*Test$")  
       set(testname ${name}.${TGT})
       add_test(${testname} ${TGT})
       #add_dependencies(check ${TGT})
    else()
       #message(" skip non-test ${TGT} ")
    endif()

    target_link_libraries(${TGT} OpticksCore)
    install(TARGETS ${TGT} DESTINATION lib)
endforeach()


# cmakex-
#set_property(TEST ${name}.OpticksResourceTest PROPERTY ENVIRONMENT 
#   "OPTICKS_GEOKEY=DAE_NAME_SPANNER"
#   "DAE_NAME_SPANNER=/tmp/spanner_in_works.dae"
# )

