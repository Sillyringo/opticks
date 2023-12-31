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

#include <cstddef>
#include <string>

template <class T> class GAry ; 
template <class T> class GDomain ; 

#include "GGEO_API_EXPORT.hh"

template <class T>
class GGEO_API GAry {

public: 
   static GAry<T>* urandom(unsigned int n=100); 
   static GAry<T>* create_from_floats(unsigned int length, float* values);
   static GAry<T>* product(GAry<T>* a, GAry<T>* b);
   static GAry<T>* subtract(GAry<T>* a, GAry<T>* b);
   static GAry<T>* add(GAry<T>* a, GAry<T>* b);
   static GAry<T>* reciprocal(GAry<T>* a, T scale=1);
   static T        maxdiff(GAry<T>* a, GAry<T>* b, bool dump=false);
   static GAry<T>* from_constant(unsigned int length, T value );
   static GAry<T>* zeros(unsigned int length);
   static GAry<T>* ones(unsigned int length);
   static GAry<T>* ramp(unsigned int length, T low, T step );
   static GAry<T>* from_domain(GDomain<T>* domain);
   static GAry<T>* linspace(T num, T start=0, T stop=1);
   static T            step(T num, T start=0, T stop=1);
   static GAry<T>* np_interp(GAry<T>* xi, GAry<T>* xp, GAry<T>* fp );
   static T np_interp(const T z, GAry<T>* xp, GAry<T>* fp );
public: 
   static GAry<T>* planck_spectral_radiance(GAry<T>* nm, T blackbody_temp_kelvin=6500.);
   static GAry<T>* cie_weight(GAry<T>* nm, unsigned int component);
   static GAry<T>* cie_X(GAry<T>* nm);
   static GAry<T>* cie_Y(GAry<T>* nm);
   static GAry<T>* cie_Z(GAry<T>* nm);
public: 
   GAry<T>* copy();
   GAry(GAry<T>* other);
   GAry(unsigned int length, T* values=0);
   virtual ~GAry();

public: 
   GAry<T>* cumsum(unsigned int offzero=0);
   GAry<T>* diff(); // domain bin widths
   GAry<T>* mid();  // average of values at bin edges, ie linear approximation of mid bin value 
   GAry<T>* reversed(bool reciprocal=false, T scale=1);
   GAry<T>* sliced(int ifr, int ito);
   GAry<T>* g4_groupvel_bintrick();
   GAry<T>* gradient();
   GAry<T>* reciprocal(T scale=1);
   GAry<T>* clip(GAry<T>* low, GAry<T>* high, GAry<T>* low_fallback=NULL, GAry<T>* high_fallback=NULL );
   void save(const char* path);

public: 
   T getLeft() const {                       return m_values[0] ; }
   T getRight() const {                      return m_values[m_length-1] ; }
   T getValue(unsigned int index) const {    return m_values[index] ;}
   T operator[](int index) const {           return index < 0 ? m_values[m_length + index] : m_values[index] ;}
   T* getValues(){                           return m_values ; }
   unsigned int getLength() const {          return m_length ; }
   unsigned int getNbytes() const {          return m_length*sizeof(T) ; }

public: 
   T min(unsigned& idx) const ; 
   T max(unsigned& idx) const ; 
   T getValueFractional(T findex); // fractional bin
   T getValueLookup(T u);          // from u(0:1) to fractional bin to values
   unsigned int getLeftZero();
   unsigned int getRightZero();

public: 
   void setValue(unsigned int index, T val){ m_values[index] = val ;}
   void setValues(T val);
public: 
   void Summary(const char* msg="GAry::Summary", unsigned int imod=1, T presentation_scale=1.0);
   void scale(T sc);
   void add(GAry<T>* other);
   void subtract(GAry<T>* other);
   void reciprocate();
   std::string digest() const ; 

   // find the index of the value closest to the random draw u on the low side
   int binary_search(T u);
   int linear_search(T u);
   T fractional_binary_search(T u);  // like binary search but provides the fractional bin too

   unsigned int sample_cdf(T u);

private:
    T* m_values ; 
    unsigned int m_length ; 
};




typedef GAry<float> GAryF ;
typedef GAry<double> GAryD ;


