cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
set(name ThrustRap)
project(${name} VERSION 0.1.0)
include(OpticksBuildOptions NO_POLICY_SCOPE)

#if(POLICY CMP0077)  # see note/issues/cmake-3.13.4-FindCUDA-warnings.rst
#    cmake_policy(SET CMP0077 OLD)
#endif()


set(ThrustRap_VERBOSE OFF)

find_package(CUDARap  REQUIRED CONFIG)

if(ThrustRap_VERBOSE)
  message(STATUS "ThrustRap.CUDA_NVCC_FLAGS : ${CUDA_NVCC_FLAGS} ")
endif()


set(SOURCES

    THRAP_LOG.cc

    TCURAND.cc
    TCURANDImp.cu
    TBuf_.cu
    TBufPair_.cu
    TRngBuf_.cu
    TSparse_.cu
    TUtil_.cu
)

SET(HEADERS

    THRAP_LOG.hh
    THRAP_API_EXPORT.hh
    THRAP_HEAD.hh
    THRAP_TAIL.hh

    THRAP_PUSH.hh
    THRAP_POP.hh

    TCURAND.hh
    TCURANDImp.hh
    TBuf.hh
    TRngBuf.hh
    TBufPair.hh
    TSparse.hh
    TUtil.hh

)

# strided_range.h and iexpand.h are moved to SysRap



CUDA_ADD_LIBRARY( ${name} ${SOURCES} OPTIONS )

get_target_property(_cal_ill ${name} INTERFACE_LINK_LIBRARIES)
#set_target_properties(${name}  PROPERTIES INTERFACE_LINK_LIBRARIES "" ) # kludge clear CAUSES GRIEF FOR INTEGRATED BUILD

#[=[
Scrubbing this causes all thrap tests to fail with the below exception in integrated build, but not for the 
proj by proj build, see thrap- for laborious debug that discovered this::

  libc++abi.dylib: terminating with uncaught exception of type thrust::system::system_error: device free failed: CUDA driver version is insufficient for CUDA runtime version

Somehow the integrated build uses::

   /Developer/NVIDIA/CUDA-9.1/lib/libcudart_static.a;-Wl,-rpath,/Developer/NVIDIA/CUDA-9.1/lib

When it should be using::

   /Developer/NVIDIA/CUDA-9.1/lib/libcudart_static.a;-Wl,-rpath,/usr/local/cuda/lib 

-- ThrustRap CUDA_ADD_LIBRARY.INTERFACE_LINK_LIBRARIES : /Developer/NVIDIA/CUDA-9.1/lib/libcudart_static.a;-Wl,-rpath,/usr/local/cuda/lib 

#]=]

target_link_libraries( ${name} Opticks::CUDARap)
target_compile_definitions( ${name} PUBLIC OPTICKS_THRAP )
#target_compile_definitions( ${name} PRIVATE WITH_NVTOOLS )
target_include_directories( ${name} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}> )

if(ThrustRap_VERBOSE)


   message(STATUS "${name} CUDA_ADD_LIBRARY.INTERFACE_LINK_LIBRARIES : ${_cal_ill} ") 
   echo_target_std(${name})
endif()


#install(TARGETS ${name} LIBRARY DESTINATION lib)    see notes/issues/lib64-lib-install.rst
install(FILES ${HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

bcm_deploy(TARGETS ${name} NAMESPACE Opticks:: SKIP_HEADER_INSTALL)

add_subdirectory(tests)



