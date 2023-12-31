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
#include "plog/Severity.h"

struct BConfig ; 

#include "NPY_API_EXPORT.hh"

/**
NFlightConfig
==================

Principal consumer is OpTracer::flightpath

An Opticks resident instance is lazily constructed by Opticks::getFlightConfig 
which gets called from OpTracer::OpTracer

**/

struct NPY_API NFlightConfig 
{
    static const plog::Severity LEVEL ; 
    static const char* DEFAULT ; 

    NFlightConfig(const char* cfg);
    BConfig* bconfig ;  
    void dump(const char* msg="NFlightConfig::dump") const ; 
    std::string desc() const ; 

    int         width ; 
    float       scale0 ;   
    float       scale1 ; 
    std::string flight ; // eg RoundaboutXY
    std::string ext ;    // typically .jpg due to its compression
    int         period ;  
    int         framelimit ;  
    int         framelimit_override ;  // from envvar 

    const char* getCfg() const ; 
    unsigned getFrameLimit() const ;
    std::string getFrameName(const char* prefix, int index) const ;
};

