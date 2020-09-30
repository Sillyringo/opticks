#pragma once

/**
SPack
======

Static packing/unpacking utilities.


**/


#include <cstddef>

#include "SYSRAP_API_EXPORT.hh"

class SYSRAP_API SPack {
     public:
        union uif_t {
            unsigned u ; 
            int      i ;
            float    f ; 
        }; 

     public:
         static unsigned Encode(unsigned char x, unsigned char y, unsigned char z, unsigned char w); 
         static unsigned Encode(const unsigned char* ptr, const unsigned num); 

         static void Decode( const unsigned value,  unsigned char& x, unsigned char& y, unsigned char& z, unsigned char& w ); 
         static void Decode( const unsigned value,  unsigned char* ptr, const unsigned num); 

     public:
         static unsigned Encode13(unsigned char c, unsigned int ccc);
         static void Decode13( const unsigned int value, unsigned char& c, unsigned int& ccc );
     
     public:
         static float int_as_float( const int i ); 
         static int int_from_float( const float f ); 
         static float uint_as_float( const unsigned f ); 
         static unsigned uint_from_float( const float f ); 


};

