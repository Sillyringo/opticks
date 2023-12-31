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

template <typename T> class NPY ; 
#include <glm/fwd.hpp>

#include "OKCORE_API_EXPORT.hh"
#include "OKCORE_HEAD.hh"

/**
OpticksDomain
===============

Canonical m_domain instance is a resident of OpticksEvent and
is instancianted by OpticksEvent::init. The domains are 
critically important for record domain compression.

* NB Opticks::setSpaceDomain only indirectly effects OpticksDomain on 
  creation of OpticksEvent instances

* OpticksEvent getters and setters defer to OpticksDomain.
* Note the vec and buffers duplication

  1. local glm::vec4/glm::ivec4 
  2. fdom/idom NPY buffers 

* copies both ways by updateBuffer() and importBuffer()

* domains are setup by Opticks::makeEvent on creating an OpticksEvent
  using results of Opticks getters such as Opticks::getSpaceDomain

* domain information comes from Opticks::setSpaceDomain which 
  triggers Opticks::postgeometry Opticks::configureDomains 

::

    [blyth@localhost opticks]$ opticks-f m_ok-\>setSpaceDomain
    ./cfg4/CGeometry.cc:    m_ok->setSpaceDomain(ce); // triggers Opticks::configureDomains
    ./opticksgeo/OpticksAim.cc:    m_ok->setSpaceDomain( ce0 );
    ./okop/OpIndexerApp.cc:    m_ok->setSpaceDomain(0.f,0.f,0.f,1000.f);  // this is required before can create an evt 


* OpticksAim::registerGeometry invokes Opticks::setSpaceDomain with 
  geometry information from mm0 the first GMergedMesh 

* OpticksHub::registerGeometry invokes OpticksAim::registerGeometry
  in the tail of OpticksHub::loadGeometry

* CGeometry::hookup also invokes Opticks::setSpaceDomain, which happens at CG4::CG4 

* Q: why twice ?  


**/

class OKCORE_API OpticksDomain {
    public:
       OpticksDomain();
       virtual ~OpticksDomain();
       void updateBuffer();  // copy from local vec into idom/fdom
       void importBuffer();  // copy from fdom/idom into local vec
       void dump(const char* msg="OpticksDomains::dump");
    public:
       // from m_settings
       unsigned getMaxRng() const ;
       unsigned getMaxRec() const ;
       unsigned getMaxBounce() const ;
       void setMaxRng(unsigned maxrng);
       void setMaxRec(unsigned maxrec);
       void setMaxBounce(unsigned maxbounce);

    public:
       NPY<float>* getFDomain() const ;
       NPY<int>*   getIDomain() const ;
       void setFDomain(NPY<float>* fdom);
       void setIDomain(NPY<int>* idom);

   public:
       // domains used for record compression
       void setSpaceDomain(const glm::vec4& space_domain);
       void setTimeDomain(const glm::vec4& time_domain);
       void setWavelengthDomain(const glm::vec4& wavelength_domain);

       const glm::vec4& getSpaceDomain() const ;
       const glm::vec4& getTimeDomain() const ;
       const glm::vec4& getWavelengthDomain() const ;

     private:
        void init();

     private:
        NPY<float>*     m_fdom ; 
        NPY<int>*       m_idom ; 

        glm::vec4       m_space_domain ; 
        glm::vec4       m_time_domain ; 
        glm::vec4       m_wavelength_domain ; 
        glm::ivec4      m_settings ; 

};

#include "OKCORE_TAIL.hh"


