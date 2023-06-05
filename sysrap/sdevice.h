#pragma once

/**
sdevice.h 
============

Simplfied version of the former cudarap/CDevice.cu 

**/

#include <cstddef>
#include <string>
#include <vector>
#include <iostream>

struct sdevice 
{

    static int VISIBLE_COUNT ; 

    int ordinal ; 
    int index ; 

    char name[256] ; 
    char uuid[16] ; 
    int major  ; 
    int minor  ; 
    int compute_capability ; 
    int multiProcessorCount ; 
    size_t totalGlobalMem ; 

    float totalGlobalMem_GB() const ;
    void read( std::istream& in ); 
    void write( std::ostream& out ) const ; 
    bool matches(const sdevice& other) const ; 

    const char* brief() const ;
    const char* desc() const ; 

    static const char* CVD ; 
    static const char* FILENAME ; 
    static int Size(); 

    static void Visible(std::vector<sdevice>& visible, const char* dirpath, bool nosave=false ); 
    static void Collect(std::vector<sdevice>& devices, bool ordinal_from_index=false ); 
    static int FindIndexOfMatchingDevice( const sdevice& d, const std::vector<sdevice>& all );

    static std::string Path(const char* dirpath); 
    static void Dump(const std::vector<sdevice>& devices, const char* msg ); 
    static void Save(const std::vector<sdevice>& devices, const char* dirpath); 
    static void Load(      std::vector<sdevice>& devices, const char* dirpath); 

    static void PrepDir(const char* dirpath); 
    static std::string Brief( const std::vector<sdevice>& devices ); 


};




#include <cassert>
#include <sstream>
#include <iomanip>
#include <string>
#include <cstring>
#include <cstdlib>
#include <fstream>

#include <cuda_runtime_api.h>



int sdevice::VISIBLE_COUNT = 0 ; 


const char* sdevice::CVD = "CUDA_VISIBLE_DEVICES" ; 

const char* sdevice::brief() const 
{
    std::stringstream ss ; 
    ss << "idx/ord/mpc/cc:" 
       << index 
       << "/" 
       << ordinal 
       << "/" 
       << multiProcessorCount 
       << "/" 
       << compute_capability 
       << std::setw(8) << std::fixed << std::setprecision(3) <<  totalGlobalMem_GB() << " GB "
       ;  
    std::string s = ss.str(); 
    return strdup(s.c_str());  
}

const char* sdevice::desc() const 
{
    std::stringstream ss ; 
    ss 
        << std::setw(30) << brief() 
        << " "
        << name 
        ; 
    std::string s = ss.str(); 
    return strdup(s.c_str());  
} 

bool sdevice::matches(const sdevice& other) const 
{
   return strncmp(other.uuid, uuid, sizeof(uuid)) == 0 && strncmp(other.name, name, sizeof(name)) == 0;   
}

float sdevice::totalGlobalMem_GB() const 
{
    return float(totalGlobalMem)/float(1024*1024*1024)  ;  
}



/**
sdevice::Collect
--------------------

Use CUDA API to collect a summary of the cudaDeviceProp properties 
regarding all attached devices into the vector of sdevice argument.

When ordinal_from_index=true the sdevice.ordinal value is taken 
from the index in the order returned by cudaGetDeviceProperties(&p, i)

**/

void sdevice::Collect(std::vector<sdevice>& devices, bool ordinal_from_index)
{
    int devCount;
    cudaGetDeviceCount(&devCount);
    std::cout << "cudaGetDeviceCount : " << devCount << std::endl ; 

    for (int i = 0; i < devCount; ++i)
    {   
        cudaDeviceProp p;
        cudaGetDeviceProperties(&p, i); 

        sdevice d ;   

        assert( sizeof(p.name) == sizeof(char)*256 ) ;  
        assert( sizeof(d.name) == sizeof(char)*256 ) ;  
        strncpy( d.name, p.name, sizeof(d.name) ); 

#ifndef CUDART_VERSION
#error CUDART_VERSION Undefined!
#elif (CUDART_VERSION >= 10000)
        assert( sizeof(p.uuid) == sizeof(uuid) ); 
        strncpy( d.uuid, p.uuid.bytes, sizeof(p.uuid) ); 
#elif (CUDART_VERSION >= 9000)
#endif

        d.index = i ; 
        d.ordinal = ordinal_from_index ? i : -1 ;    
        d.major = p.major ; 
        d.minor = p.minor ; 
        d.compute_capability = p.major*10 + p.minor ; 

        d.multiProcessorCount = p.multiProcessorCount ;  
        d.totalGlobalMem = p.totalGlobalMem ; 

        devices.push_back(d); 
    }   
}

int sdevice::Size() 
{
    return 
        sizeof(int) +   // ordinal
        sizeof(int) +   // index
        sizeof(char)*256 +  // name 
        sizeof(char)*16 +     // uuid
        sizeof(int) +     // major 
        sizeof(int) +     // minor 
        sizeof(int) +   // compute_capability
        sizeof(int) +   // multiProcessorCount 
        sizeof(size_t) ;   // totalGlobalMem
}
void sdevice::write( std::ostream& out ) const
{
    int size = Size(); 
    char* buffer = new char[size];
    char* p = buffer ; 

    memcpy( p, &ordinal,             sizeof(ordinal) )             ; p += sizeof(ordinal) ; 
    memcpy( p, &index,               sizeof(index) )               ; p += sizeof(index) ; 
    memcpy( p, name,                 sizeof(name) )                ; p += sizeof(name) ; 
    memcpy( p, uuid,                 sizeof(uuid) )                ; p += sizeof(uuid) ; 
    memcpy( p, &major,               sizeof(major) )               ; p += sizeof(major) ; 
    memcpy( p, &minor,               sizeof(minor) )               ; p += sizeof(minor) ; 
    memcpy( p, &compute_capability,  sizeof(compute_capability) )  ; p += sizeof(compute_capability) ; 
    memcpy( p, &multiProcessorCount, sizeof(multiProcessorCount) ) ; p += sizeof(multiProcessorCount) ; 
    memcpy( p, &totalGlobalMem,      sizeof(totalGlobalMem) )      ; p += sizeof(totalGlobalMem) ; 

    out.write(buffer, size);   
    assert( p - buffer == size ); 
    delete [] buffer ; 

}

void sdevice::read( std::istream& in )
{
    int size = Size(); 
    char* buffer = new char[size];
    in.read(buffer, size);   
    char* p = buffer ; 

    memcpy( &ordinal,  p,           sizeof(ordinal) )             ; p += sizeof(ordinal) ; 
    memcpy( &index,    p,           sizeof(index) )               ; p += sizeof(index) ; 
    memcpy( name,      p,           sizeof(name) )                ; p += sizeof(name) ; 
    memcpy( uuid,      p,           sizeof(uuid) )                ; p += sizeof(uuid) ; 
    memcpy( &major,    p,           sizeof(major) )               ; p += sizeof(major) ; 
    memcpy( &minor,    p,           sizeof(minor) )               ; p += sizeof(minor) ; 
    memcpy( &compute_capability, p, sizeof(compute_capability) )  ; p += sizeof(compute_capability) ; 
    memcpy( &multiProcessorCount,p, sizeof(multiProcessorCount) ) ; p += sizeof(multiProcessorCount) ; 
    memcpy( &totalGlobalMem,     p, sizeof(totalGlobalMem) )      ; p += sizeof(totalGlobalMem) ; 

    delete [] buffer ; 
}  



/**
sdevice::Visible
------------------

This assumes that the ordinal is the index when all GPUs are visible 
and it finds this by arranging to persist the query when 
CUDA_VISIBLE_DEVICES is not defined and use that to provide something 
to match against when the envvar is defined.

Initially tried to do this in one go by changing envvar 
and repeating the query. But that doesnt work, 
presumably as the CUDA_VISIBLE_DEVICES value only has 
any effect when cuda is initialized.

Of course the disadvantage of this approach 
is that need to arrange to do the persisting of all devices 
at some initialization time and need to find an 
appropriate place for the file.

The purpose is for reference running, especially performance
scanning : so its acceptable to require running a metadata
capturing executable prior to scanning.

Possibly NVML can provide a better solution, see nvml-
Actually maybe not : the NVML enumeration order follows nvidia-smi 
not CUDA. 

**/

void sdevice::Visible(std::vector<sdevice>& visible, const char* dirpath, bool nosave)
{
    char* cvd = getenv(CVD); 
    bool no_cvd = cvd == NULL ;  
    std::vector<sdevice> all ; 

    bool ordinal_from_index = no_cvd  ; 
    Collect(visible, ordinal_from_index); 

    VISIBLE_COUNT = visible.size() ; 

    if( no_cvd )
    {
        std::cerr << "sdevice::Visible no_cvd " << std::endl ; 
        if(!nosave)
        Save( visible, dirpath );      
    }
    else
    {
        std::cerr << "sdevice::Visible with cvd " << cvd << std::endl ; 
        Load(all,  dirpath); 

        for(unsigned i=0 ; i < visible.size() ; i++)
        {
            sdevice& v = visible[i] ; 
            v.ordinal = FindIndexOfMatchingDevice( v, all );   
        }
    }
}

/**
sdevice::FindIndexOfMatchingDevice
------------------------------------

**/

int sdevice::FindIndexOfMatchingDevice( const sdevice& d, const std::vector<sdevice>& all )
{
    int index = -1 ; 
    std::cout 
         << "sdevice::FindIndexOfMatchingDevice"
         << " d " << d.desc() 
         << " all.size " << all.size()
         << std::endl 
         ;  

    for(unsigned i=0 ; i < all.size() ; i++)
    {
        const sdevice& a = all[i] ; 
        bool m = a.matches(d) ; 
        std::cout
            << " a " << a.desc()
            << " m " << m 
            << std::endl 
            ;  

        if(m)
        {
           index = a.index ; 
           break ; 
        } 
    }
    std::cout << " index : " << index << std::endl ;  
    return index ; 
}


void sdevice::Dump( const std::vector<sdevice>& devices, const char* msg )
{
    std::cout << msg << "[" << Brief(devices) << "]" << std::endl  ; 
    for(unsigned i=0 ; i < devices.size() ; i++)
    {
        const sdevice& d = devices[i] ; 
        std::cout << d.desc() << std::endl ;    
    }  
}


const char* sdevice::FILENAME = "sdevice.bin" ; 

std::string sdevice::Path(const char* dirpath)
{
    std::stringstream ss ; 
    if( dirpath ) ss << dirpath << "/" ; 
    ss << FILENAME ;  
    return ss.str(); 
}


void sdevice::PrepDir(const char* dirpath)
{
    //mkdirp(dirpath, 0777);
}

void sdevice::Save( const std::vector<sdevice>& devices, const char* dirpath)
{
    std::string path = Path(dirpath); 
    //PrepDir(dirpath); 
    std::cout << "path " << path << std::endl ; 

    std::ofstream out(path.c_str(), std::ofstream::binary);
    if(out.fail())
    {
        std::cerr << " failed open for " << path << std::endl ; 
        return ; 
    }

    for(unsigned i = 0 ; i < devices.size() ; ++i )
    {
        const sdevice& d = devices[i] ; 
        d.write(out);   
    }
}

void sdevice::Load( std::vector<sdevice>& devices, const char* dirpath)
{
    std::string path = Path(dirpath); 
    std::cout
        << "dirpath " << dirpath 
        << "path " << path 
        << std::endl 
        ; 
    std::ifstream in(path.c_str(), std::ofstream::binary);

    sdevice d ; 
    while(true)
    {
        d.read(in);   
        if(in.eof()) return ;   
        if(in.fail())
        {
            std::cerr << " failed read from " << path << std::endl ; 
            return ; 
        } 
        devices.push_back(d); 
    }
}

std::string sdevice::Brief( const std::vector<sdevice>& devices )
{ 
    std::stringstream ss ; 
    for(unsigned i=0 ; i < devices.size() ; i++)
    {
        const sdevice& d = devices[i] ; 
        ss << d.ordinal << ':' ; 
        for(unsigned j=0 ; j < strlen(d.name) ; j++)
        {   
            char c = *(d.name+j) ;   
            ss << ( c == ' ' ? '_' : c ) ;   
        }
        if( i < devices.size() - 1 ) ss << ' ' ;
    }   
    return ss.str(); 
}


