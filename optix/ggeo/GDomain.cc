#include "GDomain.hh"

#include "assert.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>


template <typename T>
bool GDomain<T>::isEqual(GDomain<T>* other)
{
    return 
       getLow() == other->getLow()   &&
       getHigh() == other->getHigh() &&
       getStep() == other->getStep() ;
}


template <typename T>
unsigned int GDomain<T>::getLength()
{
   T x = m_low ; 

   unsigned int n = 0 ;
   while( x <= m_high )
   {
      x += m_step ;
      n++ ; 
   }
   assert(n < 500); // sanity check 

   return n+1 ;
} 


template <typename T>
T* GDomain<T>::getValues()
{
   unsigned int length = getLength(); 
   T* domain = new T[length];
   for(unsigned int i=0 ; i < length ; i++)
   {
      domain[i] = m_low + i*m_step ; 
   }
   return domain ;
}

/*
* :google:`move templated class implementation out of header`
* http://www.drdobbs.com/moving-templates-out-of-header-files/184403420

A compiler warning "declaration does not declare anything" was avoided
by putting the explicit template instantiation at the tail rather than the 
head of the implementation.
*/

template class GDomain<float>;
template class GDomain<double>;


