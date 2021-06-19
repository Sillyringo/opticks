#include <cstring>
#include "PLOG.hh"
#include "CRng.hh"
#include "CUt.hh"

#include "CUDA_CHECK.h"

const plog::Severity CRng::LEVEL = PLOG::EnvLevel("CRng", "DEBUG"); 

CRng::CRng(const char* path_)
    :
    path(strdup(path_)),
    num_items(0),
    d_rng_states(nullptr),    
    rng_states(nullptr),
    num_gen(100),
    d_gen(nullptr),
    gen(nullptr)
{
    init(); 
}

void CRng::init()
{
    load(); 
    upload(); 
    generate(); 
    dump(); 
}


/**
CRng::load
------------

Find that file_size is not a mutiple of item content. 
Presumably the 44 bytes of content get padded to 48 bytes
in the curandState which is typedef to curandStateXORWOW.

**/

void CRng::load()
{
    FILE *fp = fopen(path,"rb");
    if(fp == NULL) {
        LOG(fatal) << " error opening file " << path ; 
        return ; 
    }   

    fseek(fp, 0L, SEEK_END);
    long file_size = ftell(fp);
    rewind(fp);

    long type_size = sizeof(curandState) ;  
    long content_size = 44 ; 

    num_items = file_size/content_size ; 

    LOG(info) 
        << " path " << path 
        << " file_size " << file_size 
        << " type_size " << type_size 
        << " content_size " << content_size 
        << " num_items " << num_items
        ; 

    assert( file_size % content_size == 0 );  
    rng_states = (curandState*)malloc(sizeof(curandState)*num_items);

    for(long i = 0 ; i < num_items ; ++i )
    {   
        curandState& rng = rng_states[i] ;
        fread(&rng.d,                     sizeof(unsigned int),1,fp);   //  1
        fread(&rng.v,                     sizeof(unsigned int),5,fp);   //  5 
        fread(&rng.boxmuller_flag,        sizeof(int)         ,1,fp);   //  1 
        fread(&rng.boxmuller_flag_double, sizeof(int)         ,1,fp);   //  1
        fread(&rng.boxmuller_extra,       sizeof(float)       ,1,fp);   //  1
        fread(&rng.boxmuller_extra_double,sizeof(double)      ,1,fp);   //  2    11*4 = 44 
    }   
    fclose(fp);
}

void CRng::upload()
{
    LOG(LEVEL) << "[" ; 
    d_rng_states = CUt::UploadArray<curandState>(rng_states, num_items ) ;   
    LOG(LEVEL) << "]" ; 
}


extern "C" void CRng_generate(int threads_per_launch, curandState* d_rng_states, float* d_gen ) ; 

void CRng::generate()
{
    LOG(LEVEL) << "[" ; 

    CUDA_CHECK( cudaMalloc(reinterpret_cast<void**>( &d_gen ), num_gen*sizeof(float) )); 

    CRng_generate(num_gen, d_rng_states, d_gen );  

    gen = (float*)malloc(sizeof(float)*num_gen); 
    CUDA_CHECK( cudaMemcpy(reinterpret_cast<void*>( gen ), d_gen, sizeof(float)*num_gen, cudaMemcpyDeviceToHost )); 

    LOG(LEVEL) << "]" ; 
}

void CRng::dump()
{
    if( gen == nullptr ) return ; 
    for(int i=0 ; i < num_gen ; i++ ) 
    {
        std::cout 
            << std::setw(4) << i 
            << " : "
            << std::setw(10) << std::fixed << std::setprecision(7) << gen[i] 
            << std::endl
            ; 
    }
}


