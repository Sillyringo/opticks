cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
set(name OptiXRapTest)
project(${name} VERSION 0.1.0)
include(OpticksBuildOptions)

set(TEST_SOURCES

    OContextCreateTest.cc
    OScintillatorLibTest.cc

    LTOOContextUploadDownloadTest.cc
    Roots3And4Test.cc

    UseOptiXTest.cc 
    OFormatTest.cc 
    OCtxTest.cc 

    UseOptiX.cc
)


set(PAIRED_TEST_SOURCES  # tests with paired cc and cu

    bufferTest.cc
    textureTest.cc
    boundaryTest.cc
    reemissionTest.cc
    rngTest.cc
    boundaryLookupTest.cc
    texTest.cc
    tex0Test.cc
    minimalTest.cc
    rayleighTest.cc
    writeBufferTest.cc
    writeBufferLowLevelTest.cc
    redirectLogTest.cc
    downloadTest.cc
    eventTest.cc
    interpolationTest.cc
    ORngTest.cc

    OSensorLibTest.cc 

    OCtx2dTest.cc 
    OCtx3dTest.cc 
)


set(MULTI_CU_TEST_SOURCES   # .cc tests with more than one associated .cu 

    OSensorLibGeoTest.cc@cu/OSensorLibGeoTest.cu__cu/sphere.cu 

)

foreach(MSRC ${MULTI_CU_TEST_SOURCES})
    message(STATUS "MSRC:${MSRC}" )
    string(REPLACE "@" ";" MSRC ${MSRC})
    message(STATUS "MSRC:${MSRC}" )

    list(LENGTH MSRC MSRC_LEN)
    message(STATUS "MSRC_LEN:${MSRC_LEN}" )

    list(GET MSRC 0 MSRC_CC)
    list(GET MSRC 1 MSRC_CU)
 
    message(STATUS "MSRC_CC:${MSRC_CC}" )
    message(STATUS "MSRC_CU:${MSRC_CU}" )

    string(REPLACE "__" ";" MSRC_CU ${MSRC_CU})

    foreach(MSRC_CU_ELEM ${MSRC_CU})
       message(STATUS "MSRC_CU_ELEM:${MSRC_CU_ELEM}")
    endforeach()

    get_filename_component(TGT ${MSRC_CC} NAME_WE)

    CUDA_WRAP_SRCS( ${TGT} PTX _generated_PTX_files ${MSRC_CU} )
    CUDA_WRAP_SRCS( ${TGT} OBJ _generated_OBJ_files ${MSRC_CC} )  

    add_executable( ${TGT} ${_generated_OBJ_files} ${_generated_PTX_files} ${MSRC_CU} ${MSRC_CC} )
    add_test(${name}.${TGT} ${TGT})

    message(STATUS "MSRC:${MSRC} TGT:${TGT}  _generated_PTX_files:${_generated_PTX_files} _generated_OBJ_files:${_generated_OBJ_files} ")

    install(FILES ${_generated_PTX_files} DESTINATION installcache/PTX/tests)
    target_link_libraries(${TGT} OptiXRap  )
    install(TARGETS ${TGT} DESTINATION lib)
endforeach()






foreach(SRC ${TEST_SOURCES})
    get_filename_component(TGT ${SRC} NAME_WE)
    add_executable(${TGT} ${SRC})

    add_test(${name}.${TGT} ${TGT})

    target_link_libraries(${TGT} OptiXRap  )
    install(TARGETS ${TGT} DESTINATION lib)
endforeach()


foreach(SRC ${PAIRED_TEST_SOURCES})
    get_filename_component(TGT ${SRC} NAME_WE)

    CUDA_WRAP_SRCS( ${TGT} PTX _generated_PTX_files cu/${TGT}.cu )
    CUDA_WRAP_SRCS( ${TGT} OBJ _generated_OBJ_files ${TGT}.cc )  

    add_executable( ${TGT} ${_generated_OBJ_files} ${_generated_PTX_files} ${SRC} )
    add_test(${name}.${TGT} ${TGT})

    message(STATUS "SRC:${SRC} TGT:${TGT}  _generated_PTX_files:${_generated_PTX_files} _generated_OBJ_files:${_generated_OBJ_files} ")

    install(FILES ${_generated_PTX_files} DESTINATION installcache/PTX/tests)
    target_link_libraries(${TGT} OptiXRap  )
    install(TARGETS ${TGT} DESTINATION lib)
endforeach()




#[=[
intersectAnalyticyTest is special as a single cc needs multiple cu for each analytic shape
#]=]

set(SRC intersectAnalyticTest.cc)
get_filename_component(TGT ${SRC} NAME_WE)

set(INTERSECT_TEST_SOURCES

    cu/iaDummyTest.cu
    cu/iaTorusTest.cu
    cu/iaSphereTest.cu
    cu/iaConeTest.cu
    cu/iaConvexpolyhedronTest.cu
)


CUDA_WRAP_SRCS( ${TGT} PTX _generated_PTX_files ${INTERSECT_TEST_SOURCES} )
CUDA_WRAP_SRCS( ${TGT} OBJ _generated_OBJ_files ${TGT}.cc )  

add_executable( ${TGT} ${_generated_OBJ_files} ${_generated_PTX_files} ${SRC} )
add_test(${name}.${TGT} ${TGT})

message(STATUS "SRC:${SRC} TGT:${TGT}  _generated_PTX_files:${_generated_PTX_files} _generated_OBJ_files:${_generated_OBJ_files} ")

install(FILES ${_generated_PTX_files} DESTINATION installcache/PTX/tests)
target_link_libraries(${TGT} OptiXRap  )
install(TARGETS ${TGT} DESTINATION lib)


foreach(INTERSECT_TEST ${INTERSECT_TEST_SOURCES})

    get_filename_component(NM ${INTERSECT_TEST} NAME_WE)
    get_filename_component(CU ${INTERSECT_TEST} NAME)

    set(TEST ${name}.${TGT}.${NM})
    message( STATUS "NM:${NM} CU:${CU} TEST:${TEST}") 
    add_test(${TEST} ${TGT} --cu ${CU} )  # arguments for the executable
    #set_tests_properties(${TEST} PROPERTIES ENVIRONMENT "SEnvTest_COLOR=${COLOR}")
endforeach()


#[=[
Debug fabricated tests with::

  ctest -R OptiXRapTest.intersectAnalyticTest. --output-on-failure 

Note this approach to fabricated tests is simpler than my 
previous try commented in ~/opticks/ok/tests/CMakeLists.txt 

Note that are applying the same building pattern used for OptiXRap lib 
to the building all the test executables.

#]=]

