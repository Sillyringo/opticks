cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
set(name CSGTest)
project(${name} VERSION 0.1.0)


set(TEST_SOURCES
    CSGNodeTest.cc
    CSGNodeImpTest.cc
    CSGPrimImpTest.cc
    CSGPrimSpecTest.cc
    CSGPrimTest.cc
    CSGSolidTest.cc
    CSGFoundryTest.cc

    CSGFoundry_addPrimNodes_Test.cc
    CSGFoundry_findSolidIdx_Test.cc

    CSGFoundry_CreateFromSimTest.cc

    CSGNameTest.cc
    CSGTargetTest.cc
    CSGTargetGlobalTest.cc
    CSGFoundry_MakeCenterExtentGensteps_Test.cc
    CSGFoundry_getFrame_Test.cc
    CSGFoundry_getMeshName_Test.cc
    CSGFoundry_SGeo_SEvt_Test.cc

    CSGFoundry_ResolveCFBase_Test.cc
    CSGFoundryLoadTest.cc 

    CSGScanTest.cc
    CUTest.cc
    CSGLogTest.cc
    CSGMakerTest.cc
    CSGQueryTest.cc

    CSGSimtraceTest.cc
    CSGSimtraceRerunTest.cc
    CSGSimtraceSampleTest.cc

    CSGCopyTest.cc

    intersect_leaf_phicut_test.cc 
    intersect_leaf_thetacut_test.cc
    intersect_leaf_box3_test.cc 
    intersect_leaf_cylinder_test.cc 
    intersect_leaf_cylinder_vs_oldcylinder_test.cc
    CSGIntersectComparisonTest.cc 

    distance_leaf_slab_test.cc

    CSGNodeScanTest.cc
    CSGSignedDistanceFieldTest.cc

    CSGGeometryTest.cc
    CSGGeometryFromGeocacheTest.cc
    CSGClassifyTest.cc
)

foreach(SRC ${TEST_SOURCES})
    get_filename_component(TGT ${SRC} NAME_WE)
    add_executable(${TGT} ${SRC})

    add_test(${name}.${TGT} ${TGT})

    target_link_libraries(${TGT} Opticks::CSG)
    install(TARGETS ${TGT} DESTINATION lib)
endforeach()


set(TGT CSGDemoTest)
add_executable(${TGT} CSGDemoTest.cc DemoGeo.cc DemoGrid.cc)
target_link_libraries(${TGT} Opticks::CSG)
install(TARGETS ${TGT} DESTINATION lib)


