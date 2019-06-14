#pragma once

/**
SStr
======

Static string utilities.



**/


#include <cstddef>

#include "SYSRAP_API_EXPORT.hh"

class SYSRAP_API SStr {

    typedef unsigned long long ULL ;
  public:
      static void FillFromULL( char* dest, unsigned long long value, char unprintable='.') ; 
      static const char* FromULL(unsigned long long value, char unprintable='.'); 
      static unsigned long long ToULL(const char* s8 ); 


      template <size_t SIZE>
      static const char* Format1( const char* fmt, const char* value );

      template <size_t SIZE>
      static const char* Format2( const char* fmt, const char* value1, const char* value2 );

      template <size_t SIZE>
      static const char* Format3( const char* fmt, const char* value1, const char* value2, const char* value3 );

      static bool Contains(const char* s, const char* q ); 
      static bool EndsWith( const char* s, const char* q);
      static bool StartsWith( const char* s, const char* q);

      static bool HasPointerSuffix( const char* name, unsigned hexdigits ) ;   // 12 typically, 9 with Geant4 ???
      static bool HasPointerSuffix( const char* name, unsigned min_hexdigits, unsigned max_hexdigits ) ;
      static int  GetPointerSuffixDigits( const char* name );

      const char* Concat( const char* a, const char* b, const char* c=NULL  );


};



