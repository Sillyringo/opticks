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
#include "NPY_API_EXPORT.hh"

#define NPY_LOG__  {     NPY_LOG::Initialize(SLOG::instance->prefixlevel_parse( info, "NPY"), plog::get(), NULL );  } 

#define NPY_LOG_ {     NPY_LOG::Initialize(plog::get()->getMaxSeverity(), plog::get(), NULL ); } 

struct NPY_API NPY_LOG 
{
    static void Initialize(int level, void* app1, void* app2 );
    static void Check(const char* msg);
};

