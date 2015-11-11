#include "OPropagator.hh"

#include "Composition.hh"

#include "OContext.hh"
#include "OConfig.hh"
#include "OTimes.hh"
#include "OBuf.hh"
#include "OBufPair.hh"


#include <optixu/optixu.h>
#include <optixu/optixu_math_stream_namespace.h>

// npy-
#include "GLMPrint.hpp"
#include "timeutil.hpp"
#include "NPY.hpp"
#include "NumpyEvt.hpp"

#include <boost/log/trivial.hpp>
#define LOG BOOST_LOG_TRIVIAL
// trace/debug/info/warning/error/fatal


using namespace optix ; 


// cudawrap-
#include "cuRANDWrapper.hh"




void OPropagator::init()
{
    m_context = m_ocontext->getContext();

    float pe = 0.1f ; 
    m_context[ "propagate_epsilon"]->setFloat(pe);  // TODO: check impact of changing propagate_epsilon
    m_context[ "bounce_max" ]->setUint( m_bounce_max );
    m_context[ "record_max" ]->setUint( m_record_max );

    optix::uint4 debugControl = optix::make_uint4(m_ocontext->getDebugPhoton(),0,0,0);
    LOG(info) << "OPropagator::init debugControl " 
              << " x " << debugControl.x 
              << " y " << debugControl.y
              << " z " << debugControl.z 
              << " w " << debugControl.w 
              ;

    m_context["debug_control"]->setUint(debugControl); 
 
    const char* raygenprg = m_trivial ? "trivial" : "generate" ;  // last ditch debug technique
    LOG(info) << "OPropagtor::init " << raygenprg ; 

    m_ocontext->setRayGenerationProgram( OContext::e_generate_entry, "generate.cu.ptx", raygenprg );
    m_ocontext->setExceptionProgram(    OContext::e_generate_entry, "generate.cu.ptx", "exception");

    m_times = new OTimes ; 

    makeDomains();
    recordDomains();
}


void OPropagator::makeDomains()
{
    m_domain = NPY<float>::make(e_number_domain,1,4) ;
    m_domain->fill(0.f);
    m_idomain = NPY<int>::make(e_number_idomain,1,4) ;
    m_idomain->fill(0);
}

void OPropagator::recordDomains()
{
    glm::vec4 ce = m_composition->getDomainCenterExtent();
    glm::vec4 td = m_composition->getTimeDomain();

    m_context["center_extent"]->setFloat( make_float4( ce.x, ce.y, ce.z, ce.w ));
    m_context["time_domain"]->setFloat(   make_float4( td.x, td.y, td.z, td.w ));

    m_domain->setQuad(e_center_extent, 0, ce );
    m_domain->setQuad(e_time_domain  , 0, td );

    glm::vec4 wd ;
    m_context["boundary_domain"]->getFloat(wd.x,wd.y,wd.z,wd.w);
    m_domain->setQuad(e_boundary_domain  , 0, wd );
    print(wd, "OPropagator::initGeometry boundary_domain");

        // cf with MeshViewer::initGeometry
    LOG(info) << "OPropagator::initGeometry DONE "
              << " y: " << ce.y  
              << " z: " << ce.z  
              << " w: " << ce.w  ;


    glm::ivec4 ci ;
    ci.x = m_bounce_max ;  
    ci.y = m_rng_max ;  
    ci.z = 0 ;  
    ci.w = m_evt ? m_evt->getMaxRec() : 0 ;  
    m_idomain->setQuad(e_config_idomain, 0, ci );
}

void OPropagator::saveDomains(const char* dir)
{
    LOG(info) << "OPropagator::saveDomains" ; 
    m_domain->save(dir, "OPropagatorF.npy");
    m_idomain->save(dir, "OPropagatorI.npy");
}



void OPropagator::initRng()
{
    if(m_rng_max == 0 ) return ;

    const char* rngCacheDir = OConfig::RngDir() ;
    m_rng_wrapper = cuRANDWrapper::instanciate( m_rng_max, rngCacheDir );

    // OptiX owned RNG states buffer (not CUDA owned)
    m_rng_states = m_context->createBuffer( RT_BUFFER_INPUT_OUTPUT, RT_FORMAT_USER);
    m_rng_states->setElementSize(sizeof(curandState));
    m_rng_states->setSize(m_rng_max);
    m_context["rng_states"]->setBuffer(m_rng_states);

    curandState* host_rng_states = static_cast<curandState*>( m_rng_states->map() );
    m_rng_wrapper->setItems(m_rng_max);
    m_rng_wrapper->fillHostBuffer(host_rng_states, m_rng_max);
    m_rng_states->unmap();

    //
    // TODO: investigate Thrust based alternatives to curand initialization 
    //       potential for eliminating cudawrap- 
    //
}





void OPropagator::initEvent()
{
    if(!m_evt) return ;
    initEvent(m_evt);
}

void OPropagator::initEvent(NumpyEvt* evt)
{
    // when isInterop() == true 
    // the OptiX buffers for the evt data are actually references 
    // to the OpenGL buffers created with createBufferFromGLBO
    // by Scene::uploadEvt Scene::uploadSelection

    NPY<float>* gensteps =  evt->getGenstepData() ;

    m_genstep_buffer = m_ocontext->createIOBuffer<float>( gensteps, "gensteps");
    m_context["genstep_buffer"]->set( m_genstep_buffer );

    if(m_ocontext->isCompute()) 
    {
        LOG(info) << "OPropagator::initGenerate (COMPUTE)" 
                  << " uploading gensteps "
                  ;
        OContext::upload<float>(m_genstep_buffer, gensteps);
    }
    else if(m_ocontext->isInterop())
    {
        assert(gensteps->getBufferId() > 0); 
        LOG(info) << "OPropagator::initGenerate (INTEROP)" 
                  << " gensteps handed to OptiX by referencing OpenGL buffer id  "
                  ;
    }


    m_photon_buffer = m_ocontext->createIOBuffer<float>( evt->getPhotonData(), "photon" );
    m_context["photon_buffer"]->set( m_photon_buffer );

    m_record_buffer = m_ocontext->createIOBuffer<short>( evt->getRecordData(), "record");
    m_context["record_buffer"]->set( m_record_buffer );

    m_sequence_buffer = m_ocontext->createIOBuffer<unsigned long long>( evt->getSequenceData(), "sequence" );
    m_context["sequence_buffer"]->set( m_sequence_buffer );

/*
    m_aux_buffer = m_ocontext->createIOBuffer<short>( evt->getAuxData(), "aux" );
    if(m_aux_buffer.get())
       m_context["aux_buffer"]->set( m_aux_buffer );
*/


    m_sequence_buf = new OBuf("sequence", m_sequence_buffer );
    m_sequence_buf->setMultiplicity(1u);
    m_sequence_buf->setHexDump(true);

    m_photon_buf = new OBuf("photon", m_photon_buffer );

    // need to have done scene.uploadSelection for the recsel to have a buffer_id
}


void OPropagator::propagate()
{
    if(!m_evt) return ;

    unsigned int numPhotons = m_evt->getNumPhotons();
    assert( numPhotons <= getRngMax() && "Use ggeoview-rng-prep to prepare RNG states up to the maximal number of photons generated " );

    unsigned int width  = numPhotons ;
    unsigned int height = 1 ;

    LOG(info) << "OPropagator::propagate count " << m_count << " size(" <<  width << "," <<  height << ")";

    if(m_override > 0)
    {
        width = m_override ; 
        LOG(warning) << "OPropagator::generate OVERRIDE photon count for debugging to " << width ; 
    }

    m_ocontext->launch( OContext::e_generate_entry,  width, height );

    m_count += 1 ; 
}


void OPropagator::downloadEvent()
{
    if(!m_evt) return ;
    LOG(info)<<"OPropagator::downloadEvent" ;
 
    NPY<float>* dpho = m_evt->getPhotonData();
    OContext::download<float>( m_photon_buffer, dpho );

    NPY<short>* drec = m_evt->getRecordData();
    OContext::download<short>( m_record_buffer, drec );

    NPY<unsigned long long>* dhis = m_evt->getSequenceData();
    OContext::download<unsigned long long>( m_sequence_buffer, dhis );

    LOG(info)<<"OPropagator::downloadEvent DONE" ;
}



