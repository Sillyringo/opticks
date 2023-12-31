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

#include <iostream>
#include <iomanip>

#include "NBBox.hpp"
#include "NConvexPolyhedron.hpp"
#include "OPTICKS_LOG.hh"

nconvexpolyhedron* test_make_trapezoid()
{
    LOG(info) << "test_make_trapezoid" ; 

    float z  = 200 ;   // see NConvexPolyhedronTest.cc for ascii art explanation
    float x1 = 200 ; 
    float y1 = 200 ; 
    float x2 = 200 ; 
    float y2 = 200 ; 
  
    nconvexpolyhedron* cpol = nconvexpolyhedron::CreateTrapezoid( z,  x1,  y1,  x2,  y2 );
    return cpol ; 
}


void dump( nconvexpolyhedron* cpol)
{
    cpol->dump_planes();
    cpol->dump_uv_basis();
    cpol->check_planes();


    nbbox bb = cpol->bbox_model();
    std::cout << "bbox_model " << bb.desc() << std::endl ; 

    std::function<float(float,float,float)> _sdf = cpol->sdf() ;

    
    float x = 1.f ; 
    float y = 1.f ; 

    for( float z=-10. ; z < 10. ; z+=1 )
    {
         float sd = _sdf(x,y,z);
         std::cout 
              << "(" 
              << " " << std::setw(10) << x 
              << " " << std::setw(10) << y 
              << " " << std::setw(10) << z 
              << ")"
              << " -> " 
              << sd 
              << std::endl
              ; 
    }
}


nconvexpolyhedron*  test_make_segment()
{
    LOG(info) << "." ; 
    float startPhi = 0.f ; 
    //float deltaPhi = 180.f ;  // <-- degeneracies
    float deltaPhi = 90.f ; 

    float phi0 = startPhi ; 
    float phi1 = startPhi + deltaPhi ; 

    float segZ = 10.f ; 
    float segR = 10.f ; 

    nconvexpolyhedron* cpol = nconvexpolyhedron::CreateSegment(phi0, phi1, segZ, segR );

    // when the deltaphi is 180 : dont get a sensible segment 

    return cpol ; 
}



int main(int argc, char** argv)
{
    OPTICKS_LOG(argc, argv);

    //nconvexpolyhedron* cpol = test_make_trapezoid();
    nconvexpolyhedron* cpol = test_make_segment();
    dump(cpol); 


    return 0 ; 
}

