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
#include "plog/Severity.h"

#include "NPY_API_EXPORT.hh"
#include "NYJSON.hpp"
#include "NPY_HEAD.hh"

/**
NMeta (will remove, migrating to BMeta )
==========================================

Metadata persistency using nlohmann::json single header which 
comes is via yocto (gltf) external::

    /usr/local/opticks/externals/yoctogl/yocto-gl/yocto/ext/json.hpp

* https://github.com/nlohmann/json
* https://nlohmann.github.io/json/

NB 

* NMeta adds a limitation of only handling keyed structures, not lists

* are using a very old version of json.hpp version 2.0.7 from 2016
  just because that is what yocto uses 

* newer versions has a "contains(key)" method that would be handy 

* TODO: investigate updating yocto and this header : suspect 
  multiple versions of the json.hpp header used by NMeta and yocto 
  would not cause a problem : as the underlying type is well wrapped 
  and not making much use of yocto gltf anymore 


TODO: this depends on very little from NPY, but file handling from BoostRap
      so sink it down to BMeta and base directly off NLJSON rather than
      getting that via YoctoGL

**/

class NPY_API NMeta {
       static const plog::Severity LEVEL ; 
   public:
       static NMeta* Load(const char* path);
       static NMeta* Load(const char* dir, const char* name);
       static NMeta* FromTxt(const char* txt);
   public:
       NMeta();
       NMeta(const NMeta& other);

       void append(NMeta* other); // duplicate keys are overwritten

       unsigned size() const ; 

       nlohmann::json& js();
       const nlohmann::json& cjs() const ;
   public:
       const char* getKey(unsigned idx) const ;
       const char* getKey_old(unsigned idx) const ;
       unsigned    getNumKeys_old() ;             // non-const as may updateKeys
       unsigned    getNumKeys() const ;           // assumes obj 
       void        getKV(unsigned i, std::string& k, std::string& v ) const ; 

       std::vector<std::string>& getLines();  // non-const may prepLines
       std::string desc(unsigned wid=0);
       void fillMap(std::map<std::string, std::string>& mss, bool dump=false ); 
   private:
       void        updateKeys();
       void        prepLines();

   public:
       void   setObj(const char* name, NMeta* obj); 
       NMeta* getObj(const char* name) const ;
   public:
       template <typename T> void add(const char* name, T value);   // same as set, for easier migration for B_P_a_r_a_m_e_t_e_r_s
       template <typename T> void set(const char* name, T value);

       void appendString(const char* name, const std::string& value, const char* delim=" ");

       template <typename T> T get(const char* name) const ;
       template <typename T> T get(const char* name, const char* fallback) const ;
       int getIntFromString(const char* name, const char* fallback) const ;

   public:

       bool hasItem(const char* name) const ;
       bool hasKey(const char* key) const ; // same as hasItem

       void kvdump() const ;

   public:
       template <typename T> static T Get(const NMeta* meta, const char* name, const char* fallback)  ;
   public:
       void save(const char* path) const ;
       void save(const char* dir, const char* name) const ;
       void dump() const ; 
       void dump(const char* msg) const ; 
       void dumpLines(const char* msg="NMeta::dumpLines") ; 
   public:
       void addEnvvar( const char* key ) ;
       void addEnvvarsWithPrefix( const char* prefix="OPTICKS_", bool trim=true );  



   public:
       void load(const char* path);
       void load(const char* dir, const char* name);
       void loadTxt(const char* txt);

   private:
       // formerly used separate NJS, but that makes copy-ctor confusing 
       void read(const char* path0, const char* path1=NULL);
       void write(const char* path0, const char* path1=NULL) const ;
       void readTxt(const char* txt);
 
   private:
       nlohmann::json  m_js ;  
       std::vector<std::string> m_keys ; 
       std::vector<std::string> m_lines ; 

};

#include "NPY_TAIL.hh"


