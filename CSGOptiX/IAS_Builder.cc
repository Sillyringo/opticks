#include <cassert>
#include <iostream>
#include <iomanip>
#include <map>
#include <optix.h>
#include <optix_stubs.h>

#include <cuda_runtime.h>
#include "scuda.h"    // roundUp
#include "NPX.h"

#include "CUDA_CHECK.h"
#include "OPTIX_CHECK.h"

#include "Ctx.h"
#include "Properties.h"

//#include "InstanceId.h"  not in use due to bit limits 

#include "GAS.h"
#include "IAS.h"
#include "IAS_Builder.h"
#include "SBT.h"

#include "SLOG.hh"


const plog::Severity IAS_Builder::LEVEL = SLOG::EnvLevel("IAS_Builder", "DEBUG"); 


/**
IAS_Builder::CollectInstances
-------------------------------

Collecting OptixInstance was taking 0.42s for 48477 inst, 
as SBT::getOffset was being called for every instance. Instead 
of doing this caching the result in the gasIdx_sbtOffset brings
the time down to zero. 

HMM: Could make better use of instanceId, eg with bitpack gas_idx, ias_idx ?
See note in InstanceId.h its not so easy due to bit limits.  

But it doesnt matter much as can just do lookups CPU side based 
on simple indices from GPU side. 

**/


void IAS_Builder::CollectInstances(std::vector<OptixInstance>& instances, const std::vector<qat4>& ias_inst, const SBT* sbt ) // static 
{
    unsigned num_ias_inst = ias_inst.size() ; 
    unsigned flags = OPTIX_INSTANCE_FLAG_DISABLE_ANYHIT ;  
    unsigned prim_idx = 0u ;  // need sbt offset for the outer prim(aka layer) of the GAS 

    std::map<unsigned, unsigned> gasIdx_sbtOffset ;  

    for(unsigned i=0 ; i < num_ias_inst ; i++)
    {
        const qat4& q = ias_inst[i] ;   
        int ins_idx,  gasIdx, sensor_identifier, sensor_index ; 
        q.getIdentity(ins_idx, gasIdx, sensor_identifier, sensor_index );
        unsigned instanceId = q.get_IAS_OptixInstance_instanceId() ; 

        bool instanceId_is_allowed = instanceId < sbt->properties->limitMaxInstanceId ; 
        LOG_IF(fatal, !instanceId_is_allowed)
            << " instanceId " << instanceId 
            << " sbt->properties->limitMaxInstanceId " << sbt->properties->limitMaxInstanceId
            << " instanceId_is_allowed " << ( instanceId_is_allowed ? "YES" : "NO " )
            ; 
        assert( instanceId_is_allowed  ) ; 

        const GAS& gas = sbt->getGAS(gasIdx);  // susceptible to out-of-range errors for stale gas_idx 
        
        bool found = gasIdx_sbtOffset.count(gasIdx) == 1 ; 
        unsigned sbtOffset = found ? gasIdx_sbtOffset.at(gasIdx) : sbt->getOffset(gasIdx, prim_idx ) ;
        if(!found) 
        {
            gasIdx_sbtOffset[gasIdx] = sbtOffset ; 
            LOG(LEVEL)
                << " i " << std::setw(7) << i 
                << " gasIdx " << std::setw(3) << gasIdx 
                << " sbtOffset " << std::setw(6) << sbtOffset 
                << " gasIdx_sbtOffset.size " << std::setw(3) << gasIdx_sbtOffset.size()
                << " instanceId " << instanceId
                ;
        }
        OptixInstance instance = {} ; 
        q.copy_columns_3x4( instance.transform ); 
        instance.instanceId = instanceId ;  
        instance.sbtOffset = sbtOffset ;            
        instance.visibilityMask = 255;
        instance.flags = flags ;
        instance.traversableHandle = gas.handle ; 
    
        instances.push_back(instance); 
    }
}

/**
IAS_Builder::Build
----------------------

Converts *ias_inst* a vector of geometry identity instrumented transforms into
a vector of OptixInstance. The instance.sbtOffset are set using SBT::getOffset
for the gas_idx and with prim_idx:0 indicating the outer prim(aka layer) 
of the GAS.

Canonically invoked during CSGOptiX instanciation, from stack::

    CSGOptiX::CSGOptiX/CSGOptiX::init/CSGOptiX::initGeometry/SBT::setFoundry/SBT::createGeom/SBT::createIAS


**/

void IAS_Builder::Build(IAS& ias, const std::vector<qat4>& ias_inst, const SBT* sbt) // static 
{
    unsigned num_ias_inst = ias_inst.size() ; 
    LOG(LEVEL) << "num_ias_inst " << num_ias_inst ; 
    assert( num_ias_inst > 0); 
 
    std::vector<OptixInstance> instances ;  

    LOG(LEVEL) << "[ collect OptixInstance " ;  
    CollectInstances( instances, ias_inst, sbt ); 
    LOG(LEVEL) << "] collect OptixInstance " ;  

    LOG(LEVEL) << "[ build ias " ;  
    Build(ias, instances); 
    LOG(LEVEL) << "] build ias " ;  
}



NP* IAS_Builder::Serialize( const std::vector<OptixInstance>& instances ) // static 
{
    return NPX::ArrayFromVec<unsigned, OptixInstance>(instances) ; 
}

/**
IAS_Builder::Build
-------------------

Boilerplate turning the vector of OptixInstance into an IAS.

**/

void IAS_Builder::Build(IAS& ias, const std::vector<OptixInstance>& instances)
{
    unsigned numInstances = instances.size() ; 
    LOG(LEVEL) << "numInstances " << numInstances ; 

    unsigned numBytes = sizeof( OptixInstance )*numInstances ; 
    ias.instances = Serialize(instances) ;  // optional for debug 


    CUDA_CHECK( cudaMalloc( reinterpret_cast<void**>( &ias.d_instances ), numBytes ) );
    CUDA_CHECK( cudaMemcpy(
                reinterpret_cast<void*>( ias.d_instances ),
                instances.data(),
                numBytes,
                cudaMemcpyHostToDevice
                ) );

 
    OptixBuildInput buildInput = {};

    buildInput.type = OPTIX_BUILD_INPUT_TYPE_INSTANCES;
    buildInput.instanceArray.instances = ias.d_instances ; 
    buildInput.instanceArray.numInstances = numInstances ; 


    OptixAccelBuildOptions accel_options = {};
    accel_options.buildFlags = 
        OPTIX_BUILD_FLAG_PREFER_FAST_TRACE |
        OPTIX_BUILD_FLAG_ALLOW_COMPACTION ;
    accel_options.operation  = OPTIX_BUILD_OPERATION_BUILD;

    OptixAccelBufferSizes as_buffer_sizes;

    OPTIX_CHECK( optixAccelComputeMemoryUsage( Ctx::context, &accel_options, &buildInput, 1, &as_buffer_sizes ) );
    CUdeviceptr d_temp_buffer_as;
    CUDA_CHECK( cudaMalloc( reinterpret_cast<void**>( &d_temp_buffer_as ), as_buffer_sizes.tempSizeInBytes ) );

    // non-compacted output
    CUdeviceptr d_buffer_temp_output_as_and_compacted_size;
    size_t      compactedSizeOffset = roundUp<size_t>( as_buffer_sizes.outputSizeInBytes, 8ull );
    CUDA_CHECK( cudaMalloc(
                reinterpret_cast<void**>( &d_buffer_temp_output_as_and_compacted_size ),
                compactedSizeOffset + 8
                ) );

    OptixAccelEmitDesc emitProperty = {};
    emitProperty.type               = OPTIX_PROPERTY_TYPE_COMPACTED_SIZE;
    emitProperty.result             = ( CUdeviceptr )( (char*)d_buffer_temp_output_as_and_compacted_size + compactedSizeOffset );


    OPTIX_CHECK( optixAccelBuild( Ctx::context,
                                  0,                  // CUDA stream
                                  &accel_options,
                                  &buildInput,
                                  1,                  // num build inputs
                                  d_temp_buffer_as,
                                  as_buffer_sizes.tempSizeInBytes,
                                  d_buffer_temp_output_as_and_compacted_size,
                                  as_buffer_sizes.outputSizeInBytes,
                                  &ias.handle,
                                  &emitProperty,      // emitted property list
                                  1                   // num emitted properties
                                  ) );

    CUDA_CHECK( cudaFree( (void*)d_temp_buffer_as ) );

    size_t compacted_as_size;
    CUDA_CHECK( cudaMemcpy( &compacted_as_size, (void*)emitProperty.result, sizeof(size_t), cudaMemcpyDeviceToHost ) );


    if( compacted_as_size < as_buffer_sizes.outputSizeInBytes )
    {
        CUDA_CHECK( cudaMalloc( reinterpret_cast<void**>( &ias.d_buffer ), compacted_as_size ) );

        // use ias.handle as input and output
        OPTIX_CHECK( optixAccelCompact( Ctx::context, 0, ias.handle, ias.d_buffer, compacted_as_size, &ias.handle ) );

        CUDA_CHECK( cudaFree( (void*)d_buffer_temp_output_as_and_compacted_size ) );

        LOG(LEVEL)
            << "(compacted is smaller) "
            << " compacted_as_size : " << compacted_as_size
            << " as_buffer_sizes.outputSizeInBytes : " << as_buffer_sizes.outputSizeInBytes
            ; 

    }
    else
    {
        ias.d_buffer = d_buffer_temp_output_as_and_compacted_size;

        LOG(LEVEL) 
            << "(compacted not smaller) "
            << " compacted_as_size : " << compacted_as_size
            << " as_buffer_sizes.outputSizeInBytes : " << as_buffer_sizes.outputSizeInBytes
            ; 
    }
}


