#include <iostream>
#include <glm/glm.hpp>
#include "glm/gtc/matrix_transform.hpp"

#include "Geo.h"

Geo* Geo::fGeo = NULL ; 

Geo::Geo()
{
    fGeo = this ; 

    init_sphere_containing_grid_of_two_radii_spheres();
}

void Geo::init_sphere_containing_grid_of_two_radii_spheres()
{
    float ias_extent = 10.f ; 
    float ias_step = 2.f ; 

    makeGAS(0.7f); 
    makeGAS(1.0f); 
    std::vector<unsigned> gas_modulo = {0, 1} ;

    makeGAS(ias_extent*2.0f); 
    std::vector<unsigned> gas_single = {2} ;

    makeIAS(ias_extent, ias_step, gas_modulo, gas_single ); 
}



Geo* Geo::Get()
{
    return fGeo ; 
}

/**
Geo::makeGAS
---------------

GAS can hold multiple bbox, but for now use just use one each 
with symmetric extent about the origin.

**/
void Geo::makeGAS(float extent)
{
    std::cout << "Geo::makeGAS extent " << extent << std::endl ; 
    std::vector<float> bb = { -extent, -extent, -extent, +extent, +extent, +extent } ;  
    GAS gas = GAS::Build(bb); 
    vgas.push_back(gas); 
    vgas_extent.push_back(extent); 
}

unsigned Geo::getNumGAS() const 
{
    assert( vgas_extent.size() == vgas.size() ); 
    return vgas.size() ; 
}
unsigned Geo::getNumIAS() const 
{
    assert( vias_extent.size() == vias.size() ); 
    return vias.size() ; 
}
const GAS& Geo::getGAS(unsigned gas_idx) const
{
    assert( gas_idx < vgas.size() ); 
    return vgas[gas_idx] ; 
}
const IAS& Geo::getIAS(unsigned ias_idx) const
{
    assert( ias_idx < vias.size() ); 
    return vias[ias_idx] ; 
}
float Geo::getGAS_Extent(unsigned gas_idx) const
{
    assert( gas_idx < vgas_extent.size() ); 
    return vgas_extent[gas_idx] ; 
}
float Geo::getIAS_Extent(unsigned ias_idx) const
{
    assert( ias_idx < vias_extent.size() ); 
    return vias_extent[ias_idx] ; 
}



float unsigned_as_float( unsigned u ) 
{
    union { unsigned u; int i; float f; } uif ;   
    uif.u = u  ;   
    return uif.f ; 
}

OptixTraversableHandle Geo::getTop() const
{ 
    const IAS& ias = getIAS(0); 
    return ias.handle ;
}

/**
Geo::makeIAS
-------------

Create vector of transfoms and creat IAS from that.
Currently a 3D grid of translate transforms with all available GAS repeated modulo

**/

void Geo::makeIAS(float extent, float step, const std::vector<unsigned>& gas_modulo, const std::vector<unsigned>& gas_single )
{
    int n=int(extent) ;   // 
    int s=int(step) ; 

    unsigned num_gas = getNumGAS(); 
    unsigned num_gas_modulo = gas_modulo.size() ; 
    unsigned num_gas_single = gas_single.size() ; 


    std::cout << "Geo::makeIAS"
              << " num_gas " << num_gas
              << " num_gas_modulo " << num_gas_modulo
              << " num_gas_single " << num_gas_single
              << std::endl
              ;

    for(unsigned i=0 ; i < num_gas_modulo ; i++ ) assert(gas_modulo[i] < num_gas) ; 
    for(unsigned i=0 ; i < num_gas_single ; i++ ) assert(gas_single[i] < num_gas) ; 


    std::vector<glm::mat4> trs ; 

    for(int i=0 ; i < int(num_gas_single) ; i++)
    {
        unsigned idx = trs.size(); 
        unsigned instance_id = idx ; 
        unsigned gas_idx = gas_single[i] ; 

        glm::mat4 tr(1.f) ;
        tr[0][3] = unsigned_as_float(instance_id); 
        tr[1][3] = unsigned_as_float(gas_idx) ;
        tr[2][3] = unsigned_as_float(0) ;   
        tr[3][3] = unsigned_as_float(0) ;   

        trs.push_back(tr); 
    }


    for(int i=-n ; i <= n ; i+=s ){
    for(int j=-n ; j <= n ; j+=s ){
    for(int k=-n ; k <= n ; k+=s ){

        glm::vec3 tlat(i*1.f,j*1.f,k*1.f) ; 
        glm::mat4 tr(1.f) ;
        tr = glm::translate(tr, tlat );

        unsigned idx = trs.size(); 
        unsigned instance_id = idx ; 
        unsigned gas_modulo_idx = idx % num_gas_modulo ; 
        unsigned gas_idx = gas_modulo[gas_modulo_idx] ; 

        tr[0][3] = unsigned_as_float(instance_id); 
        tr[1][3] = unsigned_as_float(gas_idx) ;
        tr[2][3] = unsigned_as_float(0) ;   
        tr[3][3] = unsigned_as_float(0) ;   

        trs.push_back(tr); 
    }
    }
    }

    IAS ias = IAS::Build(trs); 
    vias.push_back(ias); 
    vias_extent.push_back(extent);
}

