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

#include <vector>
#include <string>

#include "BRAP_API_EXPORT.hh"
#include "BRAP_HEAD.hh"

class BRAP_API BDir {
   public:
      static void dirlist(std::vector<std::string>& names,  const char* path);
      static void dirlist(std::vector<std::string>& basenames,  const char* path, const char* ext);

     // basenames of directories within the path directory
      static void dirdirlist(std::vector<std::string>& names,  const char* path, const char* name_suffix, bool endswith );
      static void dirdirlist(std::vector<std::string>& names,  const char* path );

};

#include "BRAP_TAIL.hh"

