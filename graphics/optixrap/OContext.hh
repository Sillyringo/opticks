#pragma once

class OConfig ; 
struct OTimes ; 
#include <optixu/optixpp_namespace.h>
#include "NPY.hpp"

class OContext {
    public:
        enum { 
               e_pinhole_camera_entry,
               e_generate_entry,
               e_entryPointCount 
            };

        enum {
                e_radiance_ray,
                e_touch_ray,
                e_propagate_ray,
                e_rayTypeCount 
             };

     public:
            OContext(optix::Context context);
            void cleanUp();
     public:
            void launch(unsigned int entry, unsigned int width, unsigned int height=1, OTimes* times=NULL);
     public:
            optix::Program createProgram(const char* filename, const char* progname );
            void setRayGenerationProgram( unsigned int index , const char* filename, const char* progname );
            void setExceptionProgram( unsigned int index , const char* filename, const char* progname );
            void setMissProgram( unsigned int index , const char* filename, const char* progname );
     public:
            unsigned int      getNumEntryPoint();
            unsigned int      getNumRayType();
            optix::Context    getContext();
            optix::Group      getTop();

     public:
            static RTformat       getFormat(NPYBase::Type_t type);

            template <typename T>
            static void           upload(optix::Buffer& buffer, NPY<T>* npy);

            template <typename T>
            static void           download(optix::Buffer& buffer, NPY<T>* npy);

            template<typename T>
            optix::Buffer  createIOBuffer(NPY<T>* npy, const char* name, bool interop=true);

     private:
            void init();
     private:
            optix::Context    m_context ; 
            optix::Group      m_top ; 
            OConfig*          m_cfg ; 

};


inline OContext::OContext(optix::Context context) : m_context(context)
{
    init();
}
inline optix::Context OContext::getContext()
{
     return m_context ; 
}
inline optix::Group OContext::getTop()
{
     return m_top ; 
}
inline unsigned int OContext::getNumRayType()
{
    return e_rayTypeCount ;
}
inline unsigned int OContext::getNumEntryPoint()
{
    //return m_evt ? e_entryPointCount : e_entryPointCount - 1 ; 
    return e_entryPointCount ; 
}






