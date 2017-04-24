#pragma once

#include "NGLM.hpp"
#include "NNode.hpp"
#include "NPart.hpp"

#include "NPY_API_EXPORT.hh"

struct NPY_API nbox : nnode 
{
    float operator()(float x, float y, float z) const ;

    float sdf1(float x, float y, float z) ;
    float sdf2(float x, float y, float z) ;

    void adjustToFit(const nbbox& container_bb, float scale);

    nbbox bbox() const ;

    glm::vec3 gseedcenter();

    void pdump(const char* msg="nbox::pdump", int verbosity=1);

    glm::vec3 center ; 

};

// only methods that are specific to boxes 
// and need to override the nnode need to be here 




inline NPY_API void init_box(nbox& b, const nquad& p )
{
    b.param = p ; 
    b.center.x = p.f.x ; 
    b.center.y = p.f.y ; 
    b.center.z = p.f.z ; 
}




inline NPY_API nbox make_box(const nquad& p)
{
    nbox n ; 
    nnode::Init(n,CSG_BOX) ; 
    init_box(n, p );
    return n ;
}

inline NPY_API nbox make_box(float x, float y, float z, float w)
{
    nquad param ;
    param.f =  {x,y,z,w} ;
    return make_box( param ); 
}


