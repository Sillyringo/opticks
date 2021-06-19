#pragma once

/**
CRng
====

Aiming for a pure CUDA struct providing the essential features 
of optixrap/ORng but without any OptiX

**/

#include "CUDARAP_API_EXPORT.hh"
#include "plog/Severity.h"
#include "curand_kernel.h"

struct CUDARAP_API CRng 
{
    static const plog::Severity LEVEL ; 

    const char*   path ; 
    long          num_items ; 
    curandState*  d_rng_states ;    
    curandState*  rng_states ;    

    int           num_gen ;   
    float*        d_gen ; 
    float*        gen ; 


    CRng(const char* path); 

    void init(); 
    void load(); 
    void upload(); 
    void generate(); 
    void dump(); 
};


