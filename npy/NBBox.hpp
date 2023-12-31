/*
 * Copyright (c) 2019 Opticks Team. All Rights Reserved.
 *
 * This file is part of Opticks
 * (see https://bitbucket.org/simoncblyth/opticks).
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); 
 * you may not use this file except in compliance with the License.  
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software 
 * distributed under the License is distributed on an "AS IS" BASIS, 
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  
 * See the License for the specific language governing permissions and 
 * limitations under the License.
 */

#pragma once

#include <functional>
#include <vector>
#include <glm/fwd.hpp>
#include "plog/Severity.h"

struct nmat4triple ; 

#include "OpticksCSG.h"
#include "NQuad.hpp"
#include "NBBoxEnum.hpp"

#include "NPY_API_EXPORT.hh"

class NPoint ; 


struct NPY_API nbbox 
{
    static const plog::Severity LEVEL ; 

    std::function<float(float,float,float)> sdf() const ;
    float operator()(const glm::vec3& q, const nmat4triple* t_=NULL ) const  ;
    float operator()(float x_, float y_, float z_, const nmat4triple* t_=NULL ) const  ;
    float sdf_(const glm::vec3& q, const nmat4triple* t_=NULL ) const  ;
    void scan_sdf( const glm::vec3& o, const glm::vec3& range, const nmat4triple* t=NULL ) const ;


    void dump(const char* msg="nbbox::dump");
    void include(const nbbox& other );
    void include(const glm::vec3& p);
    void include(const glm::vec4& p);

    static nbbox from_points(const std::vector<glm::vec3>& points, unsigned verbosity );
    static nbbox from_points(const NPoint* points );

    const char* desc() const;
    std::string description() const ; 


    static bool inside_range(const float v, const float vmin, const float vmax ) ;
  

    static float MaxDiff( const nbbox& a, const nbbox& b);  

    static NBBoxContainment_t classify_containment_1( float delta, float epsilon,  NBBoxContainment_t neg, NBBoxContainment_t eps, NBBoxContainment_t pos );
    static std::string containment_mask_string( unsigned mask );
    static const char* containment_name( NBBoxContainment_t cont );
    unsigned classify_containment( const nbbox& container, float epsilon ) const ; // of this bbox against purported container

    // transform returns a transformed copy of the bbox
    nbbox make_transformed( const glm::mat4& t ) const ;
    static void transform_brute(nbbox& tbb, const nbbox& bb, const glm::mat4& t );
    static void transform(nbbox& tbb, const nbbox& bb, const glm::mat4& t );




    static bool HasOverlap(const nbbox& a, const nbbox& b );
    static void SubtractOverlap(nbbox& result, const nbbox& a, const nbbox& a_overlap, int verbosity );
    static bool FindOverlap(nbbox& overlap, const nbbox& a, const nbbox& b );
    static void CombineCSG(nbbox& comb, const nbbox& a, const nbbox& b, int op, int verbosity );

    bool has_overlap(const nbbox& other);
    bool find_overlap(nbbox& overlap, const nbbox& other);


    void copy_from(const nbbox& src); 


    float diagonal() const ; 

    glm::vec4 ce() const ;  
    nvec4 center_extent() const ;
    nvec4 dimension_extent() const ;
    static float extent(const nvec4& dim);

    bool contains( const nvec3& p, float epsilon=1e-4) const ; 
    bool contains( const glm::vec3& p, float epsilon=1e-4) const ; 
    bool contains( const nbbox& other, float epsilon=1e-4) const ; 

    bool is_empty() const 
    {
        return min.x == 0. && min.y == 0. && min.z == 0. && max.x == 0. && max.y == 0. && max.z == 0.  ; 
    }

    bool is_equal(const nbbox& other) const 
    {
        return min.x == other.min.x && min.y == other.min.y && min.z == other.min.z && max.x == other.max.x && max.y == other.max.y && max.z == other.max.z && invert == other.invert   ; 
    }

    void set_empty()
    {
        min.x = 0. ; 
        min.y = 0. ; 
        min.z = 0. ; 

        max.x = 0. ; 
        max.y = 0. ; 
        max.z = 0. ; 
    } 

    glm::vec3 side() const 
    {
        return max - min ; 
    }

    void expand(float delta)
    {
        min -= delta ; 
        max += delta ; 
    } 

    void scale(float factor)
    {
        min *= factor ; 
        max *= factor ; 
    } 


    glm::vec3 min ; 
    glm::vec3 max ; 
    bool  invert ; 

};


inline NPY_API bool operator == (const nbbox& a , const nbbox& b )
{
   return a.min == b.min && a.max == b.max ; 
}

inline NPY_API void init_bbox(nbbox& bb)
{
    bb.set_empty();
    bb.invert = false ; 
}

inline NPY_API nbbox make_bbox()
{
    nbbox bb ; 
    init_bbox(bb);
    return bb ; 
}

// "ctor" assuming rotational symmetry around z axis




inline NPY_API nbbox make_bbox(float xmin, float ymin, float zmin, float xmax, float ymax, float zmax, bool invert=false )
{
    nbbox bb = make_bbox();

    assert( xmax >= xmin );
    assert( ymax >= ymin );
    assert( zmax >= zmin );

    bb.min = {xmin,ymin,zmin} ;
    bb.max = {xmax,ymax,zmax} ;
    bb.invert = invert ;

    return bb ; 
}

inline NPY_API nbbox make_bbox(const glm::vec3& min, const glm::vec3& max, bool invert=false)
{
    return make_bbox(min.x, min.y, min.z, max.x, max.y, max.z, invert );
}

inline NPY_API nbbox make_bbox_zsymmetric(float zmin, float zmax, float ymin, float ymax)
{
    return make_bbox( ymin,ymin, zmin, ymax, ymax, zmax) ;
}



