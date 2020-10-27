#include "PLOG.hh"
#include "NPY.hpp"
#include "SensorLib.hh"
#include "OCtx.hh"  // the watertight wrapper, not the old OContext 
#include "OSensorLib.hh"

const plog::Severity OSensorLib::LEVEL = PLOG::EnvLevel("OSensorLib", "DEBUG"); 


OSensorLib::OSensorLib(optix::Context& ctx, SensorLib* sensorlib)
    :    
    m_octx(new OCtx((void*)ctx.get()->get())),
    m_sensorlib(sensorlib),
    m_angular_efficiency(m_sensorlib->getSensorAngularEfficiencyArray()),
    m_num_dim(  m_angular_efficiency->getNumDimensions()),
    m_num_cat(  m_angular_efficiency->getShape(0)),
    m_num_theta(m_angular_efficiency->getShape(1)),
    m_num_phi(  m_angular_efficiency->getShape(2)),
    m_num_elem( m_angular_efficiency->getShape(3)),
    m_tex_id( new unsigned[m_num_cat] )
{
    assert( m_num_dim == 4 ); 
    assert( m_num_cat < 10 ); 
    assert( m_num_elem == 1 ); 
}

void OSensorLib::convert()
{
    makeSensorAngularEfficiencyTexture() ; 
}

void OSensorLib::makeSensorAngularEfficiencyTexture()
{
    const char* config = "INDEX_NORMALIZED_COORDINATES" ; 
    for(unsigned i=0 ; i < m_num_cat ; i++)
    {
         unsigned item = i ; 
         void* buffer_ptr = m_octx->create_buffer(m_angular_efficiency, NULL, 'I', ' ', item ); 
         unsigned tex_id = m_octx->create_texture_sampler(buffer_ptr, config );
         LOG(info) << " item " << i << " tex_id " << tex_id ; 
         m_tex_id[item] = tex_id ; 
    }
}


 
