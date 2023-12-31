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
/**
SASCII
========

**/


#include <string>
#include <cstddef>
#include "SYSRAP_API_EXPORT.hh"

template <unsigned> struct SDice ; 


class SYSRAP_API SASCII 
{
  public:
      static const char UPPER[] ;  
      static const char LOWER[] ;  
      static const char NUMBER[] ;  
      static const char OTHER[] ;  
      static const char EXTRA[] ;  
      static const char ALLOWED[] ;  

  public:
      static int Index(const char* arr, int num,  char s ); 
      static int LowerIndex(char s); 
      static int UpperIndex(char s); 

      static char ToUpper(char s ); 
      static char ToLower(char s ); 
      static const char* ToUpper( const char* str ); 
      static const char* ToLower( const char* str ); 
  public:
      static unsigned Count( char c, const char* list );  
      static bool IsUpper( char c );  
      static bool IsLower( char c );  
      static bool IsNumber( char c );  
      static bool IsOther( char c );  
      static bool IsExtra( char c );  
      static bool IsAllowed( char c );
      static char Classify( char c); 

      static void DumpAllowed();
      static void Dump(const char* s);

  public:
      SASCII( const char* s_); 
      std::string getFirst(unsigned n) const ; 
      std::string getFirstUpper(unsigned n) const ; 
      std::string getFirstLast() const ; 
      std::string getTwoChar(unsigned first, unsigned second) const ; 
      std::string getTwoRandom(SDice<26>& rng ) const ; 

      void dump() const ; 

  private:
      void init();   
  public:
      const char* s ; 
      unsigned len ; 
      unsigned upper; 
      unsigned lower; 
      unsigned number; 
      unsigned other ; 
      unsigned extra ; 
      unsigned allowed ; 

      int first_upper_index ; 
      int first_other_index ; 
      int first_extra_index ; 
      int first_number_index ; 
};




