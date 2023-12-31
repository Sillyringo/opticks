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


#include "NGLM.hpp"
#include "NGLMExt.hpp"

#include "GVector.hh"
#include "GMatrix.hh"

#include "OPTICKS_LOG.hh"
#include "GGEO_LOG.hh"


void test_matrix()
{
    GMatrixF a ;
    a.Summary("a");

    GMatrixF b(
        2.f,0.f,0.f,0.f, 
        0.f,2.f,0.f,0.f, 
        0.f,0.f,2.f,0.f, 
        0.f,0.f,0.f,2.f);

    b.Summary("b");
     
    GMatrixF c(
        0.f,0.f,0.f,1.f, 
        0.f,0.f,1.f,0.f, 
        0.f,1.f,0.f,0.f, 
        1.f,0.f,0.f,0.f);

    c.Summary("c");
     


    GMatrixF p ;
    p *= a ;
    p *= b ;
    p *= c ;

    p.Summary("p");

    GMatrixF t(
        0.f,0.f,0.f,10.f, 
        0.f,0.f,0.f,20.f, 
        0.f,0.f,0.f,30.f, 
        0.f,0.f,0.f,1.f);

    gfloat3 v(0.f,0.f,0.f);
    v *= t ; 
    v.Summary("v");

}

void test_cf_glm()
{
     glm::vec3 sc(10.) ; 
     glm::vec3 tr(1.,2.,3.) ; 
    

     GMatrix<float> s(sc.x);
     s.Summary("s");     

     GMatrix<float> t(tr.x,tr.y,tr.z,sc.x);
     t.Summary("t");     

     GMatrix<float> tt((float*)t.getPointer());
     tt.Summary("tt");


     glm::mat4 scale = glm::scale(glm::mat4(1.0f), sc);
     glm::mat4 translate = glm::translate(glm::mat4(1.0f), tr);
     glm::mat4 mat = translate * scale ; 

     GMatrix<float> gg(glm::value_ptr(mat));
     gg.Summary("gg");
}

void test_summary()
{
    GMatrix<float>* m = new GMatrix<float>(100.f, 200.f, 100.f,  10.f );
    m->Summary();

    std::cout << " sizeof(GMatrixF) " << sizeof(GMatrixF)
              << " sizeof(float)*4*4 " << sizeof(float)*4*4 
              << std::endl 
              ;
}


void test_cf_glm_2()
{
    glm::vec3 tlat(0,100,0); 
    glm::vec4 axis_angle(0,0,1,30.f); 
    glm::vec3 scal(1,1,1); 

    glm::mat4 trs = nglmext::make_transform("trs", tlat, axis_angle, scal );
    GMatrixF* gtrs = new GMatrix<float>(glm::value_ptr(trs));
    const float* data = static_cast<float*>(gtrs->getPointer());
    glm::mat4 trs2 = glm::make_mat4( data ) ;  

    std::cout << "trs " << trs << std::endl ; 
    std::cout << "trs2" << trs2 << std::endl ; 

    gtrs->Summary("gtrs");
}



int main(int argc, char** argv)
{
     OPTICKS_LOG(argc, argv);
     GGEO_LOG_ ;  

     test_matrix();
     test_cf_glm();
     test_summary();
     test_cf_glm_2();


     return 0 ; 
}
