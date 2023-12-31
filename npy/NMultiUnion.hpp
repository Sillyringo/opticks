#pragma once

#include <vector>
#include "plog/Severity.h"

#include "NNode.hpp"
#include "NPY_API_EXPORT.hh"
struct nbbox ; 

/*

nmultiunion
=============

*/

struct nmat4triple ; 

struct NPY_API nmultiunion : nnode 
{
    static const plog::Severity LEVEL ; 

    static nmultiunion* Create(int type) ; 
    static nmultiunion* Create(int type, const nquad& param  ); 
    static nmultiunion* Create(int type, unsigned sub_num  ); 

    static nmultiunion* CreateFromTree( int type, const nnode* subtree ); 
    static nmultiunion* CreateFromList( int type, std::vector<nnode*>& prim  ); 

    nbbox bbox() const ; 
    float operator()(float x_, float y_, float z_) const ; 

    void pdump(const char* msg="nmultiunion::pdump") const ; 


    // placeholder zeros
    int par_euler() const ;
    unsigned par_nsurf() const ;
    unsigned par_nvertices(unsigned , unsigned ) const ; 

};


