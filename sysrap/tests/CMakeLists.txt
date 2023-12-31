cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
set(name SysRapTest)
project(${name} VERSION 0.1.0)
include(OpticksBuildOptions)   


set(TEST_SOURCES

   PythonImportTest.cc
   SOKConfTest.cc
   SArTest.cc
   SArrTest.cc
   SArgsTest.cc
   STimesTest.cc
   SEnvTest.cc
   SSysTest.cc
   SSys2Test.cc
   SSys3Test.cc
   SStrTest.cc
   SPathTest.cc
   STrancheTest.cc
   SVecTest.cc
   SNameVecTest.cc
   SMapTest.cc
   SCountTest.cc
   SSeqTest.cc
   SProcTest.cc
   SBase36Test.cc
   SSortKVTest.cc
   SPairVecTest.cc
   SDigestTest.cc
   SDigestNPTest.cc

   SCFTest.cc
   SGeoTest.cc
   SCurandStateTest.cc

   
   PLogTest.cc 
   SLOG_Test.cc 
   SLOGTest.cc 
   SYSRAP_LOG_Test.cc 
   SYSRAP_LOG_FileAppenderTest.cc

   SYSRAP_OPTICKS_LOG_Test.cc
   SYSRAP_OPTICKS_LOG_NULL_Test.cc 
   SOPTICKS_LOG_Test.cc 

   OPTICKS_LOG_Test.cc
   sLOG_MACRO_Test.cc
   SLOG_exename_Test.cc
   SLOG_Banner_Test.cc 
    
   reallocTest.cc
   OpticksCSGTest.cc
   hash_define_without_value.cc
   SDirectTest.cc 
   S_freopen_redirect_test.cc
   S_get_option_Test.cc

   SIdTest.cc

   ArrayTest.cc
   SBacktraceTest.cc
   SStackFrameTest.cc
   SGDMLTest.cc

   SSetTest.cc
   STimeTest.cc
   SASCIITest.cc
   SAbbrevTest.cc
   SPPMTest.cc
   SColorTest.cc
   SPackTest.cc 
   SBitTest.cc
   SBitSetTest.cc
   SEnabledTest.cc
   SBitFromStringTest.cc
   SRandTest.cc
   SOpticksTest.cc
   SOpticksKeyTest.cc
   SOpticksResourceTest.cc

   SRngSpecTest.cc 

   CheckGeoTest.cc

   SGLMTest.cc

   SConstantTest.cc
   SLabelCacheTest.cc

   CastTest.cc

   SOpticksVersionNumberTest.cc
   SRngTest.cc 
   SDirTest.cc
   SDiceTest.cc

   stranTest.cc
   stranRotateTest.cc

   SCenterExtentGenstepTest.cc
   SFrameGenstep_MakeCenterExtentGensteps_Test.cc

   SEventTest.cc
   SThetaCutTest.cc
   SPhiCutTest.cc
   scanvasTest.cc 

   OpticksPhotonTest.cc
   SUTest.cc
   SEventConfigTest.cc
   SFrameConfigTest.cc
   SGeoConfigTest.cc

   SCVDLabel.cc 

   SEvtTest.cc
   SEvtLoadTest.cc
   SEvt__UU_BURN_Test.cc

   SEvt_Lifecycle_Test.cc
   SEvt__HasInputPhoton_Test.cc 
   SEvt_AddEnvMeta_Test.cc

   SSimTest.cc
   SBndTest.cc
   SNameTest.cc
)


if(NLJSON_FOUND)
   list(APPEND TEST_SOURCES SMetaTest.cc)
endif()


if(WITH_SIMG)
   list(APPEND TEST_SOURCES SIMGTest.cc)
endif()

if(WITH_STTF)
   list(APPEND TEST_SOURCES STTFTest.cc)
endif()




set(_TEST_SOURCES
   UnwindTest.cc
)


set(OBJC_TEST_SOURCES
)

set(_OBJC_TEST_SOURCES

   NSLogTest_m.m
   NSLogTest_mm.mm
)




foreach(SRC ${TEST_SOURCES})
    get_filename_component(TGT ${SRC} NAME_WE)
    add_executable(${TGT} ${SRC})

    add_test(${name}.${TGT} ${TGT})
    #add_dependencies(check ${TGT})

    target_link_libraries(${TGT} SysRap)
    install(TARGETS ${TGT} DESTINATION lib)
endforeach()



set(TEST_COLORS 
    red
    green
    blue
)

foreach(COLOR ${TEST_COLORS})
    set(TGT SEnvTest)
    set(TEST ${name}.${TGT}.${COLOR})
    add_test(${TEST} ${TGT} SEnvTest_C --info )  # arguments for the executable, the envvar prefix and debug level
    set_tests_properties(${TEST} PROPERTIES ENVIRONMENT "SEnvTest_COLOR=${COLOR}")
endforeach()


#[=[
Debug these "fabricated" tests with::

   ctest -R SysRapTest.SEnvTest. -V


CTest fixtures

https://crascit.com/2016/10/18/test-fixtures-with-cmake-ctest/

#]=]


#foreach(SRC ${OBJC_TEST_SOURCES})
#    get_filename_component(TGT ${SRC} NAME_WE)
#    add_executable(${TGT} ${SRC})
#
#    add_test(${name}.${TGT} ${TGT})
#    #add_dependencies(check ${TGT})
#
#    target_link_libraries(${TGT} ${LIBRARIES} "-framework Foundation")
#    install(TARGETS ${TGT} DESTINATION lib)
#endforeach()



