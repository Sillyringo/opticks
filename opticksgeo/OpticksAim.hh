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

#include <string>
#include <map>
#include <glm/fwd.hpp>
#include "plog/Severity.h"

class GGeo ; 

class Opticks ; 
class OpticksHub ; 
class Composition ; 

#include "OKGEO_API_EXPORT.hh"

/**
OpticksAim
===========

Canonical m_aim is resident of OpticksHub and is instanciated by OpticksHub::init
The crucial for domain setup OpticksAim::registerGeometry is 

**/

class OKGEO_API OpticksAim {
    public:
       static const plog::Severity LEVEL ; 
    public:
       OpticksAim(OpticksHub* hub);
       void registerGeometry(GGeo* ggeo);
    public:
       void            target();   // point composition at geocenter or the m_evt (last created)
       void            setupCompositionTargetting() ;
       unsigned        getTarget() const ;
       void            setTarget(unsigned target=0, bool aim=true);
    private:
       static int      Preinit(); 
       void            init(); 
       void            dumpTarget(const char* msg="OpticksAim::dumpTarget") const ;  
    private:
       int             m_preinit ; 
       Opticks*        m_ok ; 
       bool            m_dbgaim ;  // --dbgaim
       OpticksHub*     m_hub ; 
       Composition*    m_composition ; 

       GGeo*           m_ggeo ; 
       int             m_target ;
       int             m_gdmlaux_target ; 
       int             m_cmdline_targetpvn ; 
       bool            m_autocam ; 

       std::map<std::string, int> m_targets ; 


};


