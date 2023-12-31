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
#include "SYSRAP_API_EXPORT.hh"

#define SYSRAP_LOG__  {       SYSRAP_LOG::Initialize(SLOG::instance->prefixlevel_parse( info, "SYSRAP"), plog::get(), NULL );  } 
#define SYSRAP_LOG_ {         SYSRAP_LOG::Initialize(plog::get()->getMaxSeverity(), plog::get(), NULL ); } 
#define _SYSRAP_LOG( IDX ) {  SYSRAP_LOG::Init<IDX>( info, plog::get<IDX>(), nullptr ) ; }


struct SYSRAP_API SYSRAP_LOG 
{
    static void Initialize(int level, void* app1, void* app2 );
    static void Check(const char* msg);

    template<int instance>
    static void Init(int level, void* app1, void* app2 ); 
};

