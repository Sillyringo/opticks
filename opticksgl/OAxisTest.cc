// npy-
#include "NPY.hpp"
#include "ViewNPY.hpp"
#include "MultiViewNPY.hpp"

// optixrap-
#include "OContext.hh"
#include <optixu/optixu.h>

// optix-
#include <optixu/optixu.h>
#include <optixu/optixu_math_stream_namespace.h>
using namespace optix ; 


// opticksgl-
#include "OAxisTest.hh"

#include "PLOG.hh"

OAxisTest::OAxisTest(OContext* ocontext, NPY<float>* axis_data)
    :
     m_ocontext(ocontext),
     m_axis_data(axis_data)
{
    init();
}

void OAxisTest::init()
{
    m_ocontext->addRayGenerationProgram(   "axisTest.cu.ptx", "axisModify" ); // from optixrap
    m_ocontext->addExceptionProgram(       "axisTest.cu.ptx", "exception");
    m_entry = 0 ; 

    m_buffer = m_ocontext->createIOBuffer<float>( m_axis_data, "axis");
    m_ni = m_axis_data->getShape(0);

    optix::Context& context = m_ocontext->getContextRef() ;
    context["axis_buffer"]->set(m_buffer);
}

void OAxisTest::prelaunch()
{
    m_ocontext->launch( OContext::VALIDATE,  m_entry, m_ni, 1);
    m_ocontext->launch( OContext::COMPILE,   m_entry, m_ni, 1);
    m_ocontext->launch( OContext::PRELAUNCH, m_entry, m_ni, 1);
}

void OAxisTest::launch(unsigned count)
{
  //  if(count < 1000) return ; 
    LOG(info) << "OAxisTest::launch " << count ; 


    float x=(count % 1000)*1.f ;
    float y=0.f ;
    float z=0.f ;
    float w=0.f ;

    optix::Context& context = m_ocontext->getContextRef() ;
    context["center_extent"]->setFloat( make_float4( x, y, z, w ));

    m_ocontext->launch( OContext::LAUNCH, m_entry, m_ni, 1);
}

void OAxisTest::download()
{
    OContext::download<float>( m_buffer, m_axis_data );

    m_axis_data->dump();
    m_axis_data->save("$TMP/OOAxisAppTest.npy");
}




