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

#include <algorithm>
#include <boost/algorithm/string/replace.hpp>

#include "NGLM.hpp"
#include "NPY.hpp"
#include "NPYBase.hpp"
#include "NP.hh"

#include <cstring>
#include <sstream>
#include <iostream>
#include <algorithm>

// sysrap-
#include "SDigest.hh"

//brap- 
#include "BFile.hh"
#include "BStr.hh"
#include "BMeta.hh"

#include "NPYSpec.hpp"
#include "SLOG.hh"


const unsigned NPYBase::SIGNBIT    = 0x80000000 ; 
const unsigned NPYBase::NOTSIGNBIT = 0x7fffffff ; 


bool NPYBase::GLOBAL_VERBOSE = false ; 

bool  NPYBase::NPDump = false ; 
void NPYBase::SetNPDump(bool npdump) { NPDump = npdump ; }
bool NPYBase::IsNPDump(){ return NPDump ; }

const plog::Severity NPYBase::LEVEL = SLOG::EnvLevel("NPYBase", "DEBUG"); 


const char* NPYBase::FLOAT_ = "FLOAT" ; 
const char* NPYBase::SHORT_ = "SHORT" ; 
const char* NPYBase::DOUBLE_ = "DOUBLE" ; 
const char* NPYBase::INT_ = "INT" ; 
const char* NPYBase::UINT_ = "UINT" ; 
const char* NPYBase::CHAR_ = "CHAR" ; 
const char* NPYBase::UCHAR_ = "UCHAR" ; 
const char* NPYBase::ULONGLONG_ = "ULONGLONG" ; 

const char* NPYBase::TypeName(Type_t type)
{
    const char* name = NULL ; 
    switch(type)
    { 
        case FLOAT:name=FLOAT_;break;
        case SHORT:name=SHORT_;break;
        case DOUBLE:name=DOUBLE_;break;
        case INT:name=INT_;break;
        case UINT:name=UINT_;break;
        case CHAR:name=CHAR_;break;
        case UCHAR:name=UCHAR_;break;
        case ULONGLONG:name=ULONGLONG_;break;
    } 
    return name ; 
}


bool NPYBase::IsRealType(Type_t type) // static 
{
    return type == FLOAT || type == DOUBLE ; 
}

bool NPYBase::IsIntegerType(Type_t type) // static 
{
    return type == INT || type == SHORT || type == CHAR || type == UCHAR || type == UINT || type == ULONGLONG ; 
}

bool NPYBase::IsUnsignedType(Type_t type) // static 
{
    return type == UCHAR || type == UINT || type == ULONGLONG ; 
}

bool NPYBase::IsCharType(Type_t type) // static 
{
    return type == CHAR || type == UCHAR ;  
}






NPYBase* NPYBase::Load( const char* path, NPYBase::Type_t type )
{
    NPYBase* buffer = NULL ; 
    switch(type)
    {
        case FLOAT:     buffer = NPY<float>::load(path)               ; break ; 
        case SHORT:     buffer = NPY<short>::load(path)               ; break ; 
        case DOUBLE:    buffer = NPY<double>::load(path)              ; break ; 
        case INT:       buffer = NPY<int>::load(path)                 ; break ; 
        case UINT:      buffer = NPY<unsigned>::load(path)            ; break ; 
        case CHAR:      buffer = NPY<char>::load(path)                ; break ; 
        case UCHAR:     buffer = NPY<unsigned char>::load(path)       ; break ; 
        case ULONGLONG: buffer = NPY<unsigned long long>::load(path)  ; break ; 
    } 
    return buffer ; 
}

NPYBase* NPYBase::Make( unsigned ni, const NPYSpec* itemspec, bool zero )
{
    NPYBase* buffer = NULL ; 
    NPYBase::Type_t type = itemspec->getType();  
    switch(type)
    {
        case FLOAT:     buffer = NPY<float>::make(ni, itemspec)               ; break ; 
        case SHORT:     buffer = NPY<short>::make(ni, itemspec)               ; break ; 
        case DOUBLE:    buffer = NPY<double>::make(ni, itemspec)              ; break ; 
        case INT:       buffer = NPY<int>::make(ni, itemspec)                 ; break ; 
        case UINT:      buffer = NPY<unsigned>::make(ni, itemspec)            ; break ; 
        case CHAR:      buffer = NPY<char>::make(ni, itemspec)                ; break ; 
        case UCHAR:     buffer = NPY<unsigned char>::make(ni, itemspec)       ; break ; 
        case ULONGLONG: buffer = NPY<unsigned long long>::make(ni, itemspec)  ; break ; 
    } 
    assert(buffer);
    if(zero)
    {
        buffer->zero();  
    }
    return buffer ; 
}



/*

std::string NPYBase::path(const char* dir, const char* reldir, const char* name)
{
    std::string path = BOpticksEvent::path(dir, reldir, name);
    return path ; 
}

std::string NPYBase::path(const char* dir, const char* name)
{
    std::string path = BOpticksEvent::path(dir, name);
    return path ; 
}

std::string NPYBase::path(const char* det, const char* source, const char* tag, const char* tfmt)
{
    std::string path = BOpticksEvent::path(det, source, tag, tfmt );
    return path ; 
}

*/



int NPYBase::checkNumItems(NPYBase* data) 
{
    return data && data->hasData() ? data->getNumItems() : -1 ;
}


void NPYBase::setGlobalVerbose(bool verbose)
{
    GLOBAL_VERBOSE = verbose ;
}

void NPYBase::setLookup(NLookup* lookup)
{
    m_lookup = lookup ; 
}
NLookup* NPYBase::getLookup() const 
{
    return m_lookup ; 
}




BMeta* NPYBase::getParameters() const 
{
    return m_parameters ;
}

template <typename T>
void NPYBase::setParameter(const char* key, T value)
{
    m_parameters->set<T>(key, value);
}
template <typename T>
T NPYBase::getParameter(const char* key, const char* fallback) const 
{
    m_parameters->get<T>(key,fallback);
}






/**
NPY::setReservation getReservation hasReservation
---------------------------------------------------

The reservation is the number of items (not values) 
that the capacity of the vector will be reserved to hold
when first adding values into a zero item array.

This is used for example from cfg4/CGenstepCollector 
to reduce resource usage from the multiple reallocs necessary from 
adding thousands of gensteps into an array without any reservation.

**/


void NPYBase::setReservation(int items)
{
    LOG(LEVEL) << "items " << items ; 
    m_reservation = items ; 
}
int NPYBase::getReservation() const
{
    return m_reservation ; 
}
bool NPYBase::hasReservation() const
{
    return m_reservation > 0 ; 
}
















//// TODO : get rid of the specifics 
////        they look out of place in this generic code

bool NPYBase::isGenstepTranslated() const 
{
    return m_parameters->get<bool>("GenstepTranslated","0");  // fallback to false if not set 
}
void NPYBase::setGenstepTranslated(bool flag)
{
    m_parameters->add<bool>("GenstepTranslated", flag); 
}
void NPYBase::setNumHit(unsigned num_hit)
{
    m_parameters->set<unsigned>("NumHit", num_hit); 
}
unsigned NPYBase::getNumHit() const 
{
    return m_parameters->get<unsigned>("NumHit","0"); 
}





void NPYBase::setMeta(BMeta* meta)
{
    m_meta = meta ;
}
BMeta* NPYBase::getMeta() const 
{
    return m_meta ; 
}


template <typename T>
void NPYBase::setMeta(const char* key, T value)
{
    if(!m_meta) m_meta = new BMeta ; 
    m_meta->set<T>(key, value);
}
template <typename T>
T NPYBase::getMeta(const char* key, const char* fallback) const 
{
    assert(m_meta); 
    return m_meta->get<T>(key,fallback);
}

void NPYBase::getMetaKeys( std::vector<std::string>& keys ) const
{
    unsigned num_keys = m_meta->getNumKeys(); 
    for(unsigned i=0 ; i < num_keys ; i++)
    {
        const char* key = m_meta->getKey(i); 
        keys.push_back(key); 
    } 
}

void NPYBase::CopyMeta( NP* dst, const NPYBase* src ) // static 
{
    LOG(LEVEL) << "[" ; 

    BMeta* meta = src->getMeta(); 
    unsigned num_keys = meta->getNumKeys();

    LOG(LEVEL) << meta->desc(); 

    for(unsigned i=0 ; i < num_keys ; i++)
    {
        const char* k = meta->getKey(i) ; 
        const char* v = meta->getValue(i) ; 
        LOG(LEVEL) << " k " << k << " v " << v ; 
        dst->set_meta<std::string>(k, v ); 
    } 

    LOG(LEVEL) << "]" ; 
}





const char* NPYBase::ArrayContentVersion = "ArrayContentVersion" ; 

int NPYBase::getArrayContentVersion() const 
{
    return getMeta<int>(ArrayContentVersion, "0");
}
void NPYBase::setArrayContentVersion(int acv)
{
    setMeta<int>(ArrayContentVersion, acv) ;
}


const char* NPYBase::ArrayContentIndex = "ArrayContentIndex" ; 

int NPYBase::getArrayContentIndex() const 
{
    return getMeta<int>(ArrayContentIndex, "0");
}
void NPYBase::setArrayContentIndex(int aci)
{
    setMeta<int>(ArrayContentIndex, aci) ;
}


/**
NPYBase::setBasePtr
---------------------

**/

void NPYBase::setBasePtr(void* base_ptr)
{
    m_base_ptr = base_ptr ; 
}
void* NPYBase::getBasePtr() const 
{
    return m_base_ptr ; 
}

/**
NPYBase::write_
------------------

CAUTION: this is a bit dodgy as a growing vector can be relocated, so 
in that case it is necessary for the base ptr to be updated otherwise 
it will become stale.

The updating via setBasePtr is done in NPY::allocate.

**/

void NPYBase::write_(void* dst ) const 
{
    LOG(LEVEL) << "[" ; 
    unsigned num_bytes = getNumBytes(0) ;
    if( num_bytes == 0 )
    {
         LOG(warning) << " warning writing empty " << getName() ; 
         assert(0); 
    }
    else
    {
        assert( m_base_ptr ); 
        memcpy( dst, m_base_ptr, num_bytes ); 
    }
    LOG(LEVEL) << "]" ; 
}
void NPYBase::write_item_(void* dst, unsigned item) const 
{
    LOG(LEVEL) << "[" ; 
    unsigned num_bytes = getNumBytes(1) ; // from_dim
    if( num_bytes == 0 )
    {
         LOG(warning) << " warning writing empty item " << getName() ; 
    } 
    else
    {
        assert( m_base_ptr ); 
        unsigned item_size = num_bytes ; // from_dim 
        const void* src = (char*)m_base_ptr + item*item_size ; 
        LOG(LEVEL) 
            << " base_ptr " << m_base_ptr 
            << " item_size " << item_size
            << " item " << item   
            << " dst " << dst            
            << " src " << src
            ;            

        memcpy( dst, src, item_size ); 
    }
    LOG(LEVEL) << "]" ; 
}

void NPYBase::read_(const void* src) 
{
    LOG(LEVEL) << "[" ; 
    unsigned num_bytes = getNumBytes(0) ; // from_dim
    if( num_bytes == 0)
    {
         LOG(warning) << " warning reading empty " ; 
    }  
    else
    {
        assert( m_base_ptr ); 
        memcpy( m_base_ptr, src, num_bytes ); 
    }
    LOG(LEVEL) << "]" ; 
}
void NPYBase::read_item_(const void* src, unsigned item)
{
    LOG(LEVEL) << "[" ; 
    unsigned num_bytes = getNumBytes(1) ; // from_dim 
    if(num_bytes == 0)
    {
         LOG(warning) << " warning reading empty item " ; 
    }
    else
    {
        assert( m_base_ptr ); 
        unsigned item_size = num_bytes ;
        memcpy( (char*)m_base_ptr + item*item_size, src, item_size );
    }
    LOG(LEVEL) << "]" ; 
}




NPYBase::NPYBase(const std::vector<int>& shape, unsigned long long sizeoftype, Type_t type, std::string& metadata, bool has_data) 
    :
    m_shape(shape),
    m_sizeoftype(sizeoftype),
    m_type(type),
    m_metadata(metadata),
    m_has_data(has_data),
    m_base_ptr(NULL),
    m_ni(getShape(0)),
    m_nj(getShape(1)),
    m_nk(getShape(2)),
    m_nl(getShape(3)),
    m_nm(getShape(4)),
    m_dim(m_shape.size()),
    m_shape_spec(new NPYSpec(NULL, m_ni, m_nj, m_nk, m_nl, m_nm, m_type, "" )),
    m_item_spec(new NPYSpec(NULL,    0, m_nj, m_nk, m_nl, m_nm, m_type, "" )),
    m_buffer_spec(NULL),
    m_buffer_id(-1),
    m_buffer_target(-1),
    m_buffer_control(0),
    m_buffer_name(NULL),
    m_buffer_index(-1),
    m_action_control(0),
    m_aux(NULL),
    m_verbose(false),
    m_allow_prealloc(false),
    m_dynamic(false),
    m_lookup(NULL),
    m_parameters(new BMeta),
    m_meta(new BMeta),
    m_name(NULL),
    m_reservation(0)
{
} 


/**
NPYBase::updateDimensions
---------------------------

Called internally when resizing. 

Note that the updates to m_shape_spec and m_item_spec were
omitted prior to Aug 31 2020. Fixing a very old bug.
**/

void NPYBase::updateDimensions()
{
    m_ni = getShape(0); 
    m_nj = getShape(1);
    m_nk = getShape(2);
    m_nl = getShape(3);  // gives 0 when beyond dimensions
    m_nm = getShape(4);
    m_dim = m_shape.size();

    delete m_shape_spec ; 
    delete m_item_spec ; 
    m_shape_spec = new NPYSpec(NULL, m_ni, m_nj, m_nk, m_nl, m_nm, m_type, "" ) ;
    m_item_spec  = new NPYSpec(NULL,    0, m_nj, m_nk, m_nl, m_nm, m_type, "" ) ; 
}


NPYBase::~NPYBase()
{
}

const char* NPYBase::getName() const 
{
    return m_name ;  
}
void NPYBase::setName(const char* name) 
{
    m_name = name ? strdup(name) : NULL ; 
}



 void NPYBase::setHasData(bool has_data)
{
    m_has_data = has_data ; 
}

 bool NPYBase::hasData() const 
{
    return m_has_data ; 
}

const NPYSpec* NPYBase::getShapeSpec() const 
{
    return m_shape_spec ; 
}
const NPYSpec* NPYBase::getItemSpec() const 
{
    return m_item_spec ; 
}



// shape related

const std::vector<int>& NPYBase::getShapeVector() const 
{
    return m_shape ; 
}




 unsigned int NPYBase::getNumItems(int ifr, int ito) const 
{
    //  A) default ifr/ito  0/1 correponds to shape of 1st dimension
    //
    //  B) example ifr/ito  0/-1  gives number of items excluding last dimension 
    //               -->    0/2   --> shape(0)*shape(1)    for ndim 3 
    //
    //  C)       ifr/ito  0/3 for ndim 3   shape(0)*shape(1)*shape(2)
    //
    //  D)  ifr/ito 0/0     for any dimension
    //           -> 0/ndim     -> shape of all dimensions  
    //
    //
    int ndim = m_shape.size();
    if(ifr <  0) ifr += ndim ; 
    if(ito <= 0) ito += ndim ; 

    assert(ifr >= 0 && ifr < ndim);
    assert(ito >= 0 && ito <= ndim);

    unsigned int nit(1) ; 
    for(int i=ifr ; i < ito ; i++) nit *= getShape(i);
    return nit ;
}
 unsigned int NPYBase::getNumElements() const 
{
    return getShape(m_shape.size()-1);
}

unsigned int NPYBase::getNumDimensions() const 
{
    return m_shape.size(); 
}
 unsigned int NPYBase::getDimensions() const 
{
    return m_shape.size();
}
 unsigned int NPYBase::getShape(int n) const 
{
    if( n < 0 ) n += m_shape.size() ; 
    return n < int(m_shape.size()) ? m_shape[n] : 0 ;
}




// OpenGL related

 void NPYBase::setBufferId(int buffer_id)
{
    m_buffer_id = buffer_id  ;
}
 int NPYBase::getBufferId() const 
{
    return m_buffer_id ;
}
bool NPYBase::isComputeBuffer() const 
{
    return m_buffer_id == -1 ; 
}
bool NPYBase::isInteropBuffer() const 
{
    return m_buffer_id > -1 ; 
}


 void NPYBase::setBufferTarget(int buffer_target)
{
    m_buffer_target = buffer_target  ;
}
 int NPYBase::getBufferTarget() const 
{
    return m_buffer_target ;
}


void NPYBase::setBufferControl(unsigned long long control)
{
    m_buffer_control = control ;  
}
unsigned long long NPYBase::getBufferControl() const 
{
    return m_buffer_control ;  
}
unsigned long long* NPYBase::getBufferControlPtr() 
{
    return &m_buffer_control ;  
}



void NPYBase::addActionControl(unsigned long long control)
{
    m_action_control |= control ;  
}
void NPYBase::setActionControl(unsigned long long control)
{
    m_action_control = control ;  
}

unsigned long long NPYBase::getActionControl() const 
{
    return m_action_control ;  
}
unsigned long long* NPYBase::getActionControlPtr() 
{
    return &m_action_control ;  
}

/**
NPYBase::transfer
------------------

Used via NPY::copy by NPY::clone 

**/

void NPYBase::transfer(NPYBase* dst, const NPYBase* src)
{
    const NPYSpec* spec = src->getBufferSpec();

    dst->setBufferSpec(spec ? spec->clone() : NULL);
    dst->setBufferControl(src->getBufferControl());
    dst->setActionControl(src->getActionControl());
    dst->setLookup(src->getLookup());   // NB not cloning, as lookup is kinda global
}


void NPYBase::setBufferName(const char* name )
{
    m_buffer_name = name ? strdup(name) : NULL  ;  
}
void NPYBase::setBufferIndex(unsigned index )
{
    m_buffer_index = index  ;  
}
unsigned NPYBase::getBufferIndex() const 
{
    return m_buffer_index ;  
}




void NPYBase::setBufferSpec(const NPYSpec* spec)
{
    // set when OpticksEvent uses the ctor  NPY<T>::make(NPYSpec* )
    m_buffer_spec = spec ; 
    setBufferName(spec ? spec->getName() : NULL);
}

const char* NPYBase::getBufferName() const 
{
    return m_buffer_name ;  
}
const NPYSpec* NPYBase::getBufferSpec() const 
{
    return m_buffer_spec ; 
}









// used for CUDA OpenGL interop
 void NPYBase::setAux(void* aux)
{
    m_aux = aux ; 
}
 void* NPYBase::getAux() const 
{
    return m_aux ; 
}






 void NPYBase::setVerbose(bool verbose)
{
    m_verbose = verbose ; 
}
 void NPYBase::setAllowPrealloc(bool allow)
{
    m_allow_prealloc = allow ; 
}


unsigned long long NPYBase::getValueIndex(int i_, int j_, int k_, int l_, int m_) const  
{
    // -ve indice is relative to that dimension, so -1 for the last 
    unsigned i = i_ < 0 ? m_ni + i_ : i_ ; 
    unsigned j = j_ < 0 ? m_nj + j_ : j_ ; 
    unsigned k = k_ < 0 ? m_nk + k_ : k_ ; 
    unsigned l = l_ < 0 ? m_nl + l_ : l_ ; 
    unsigned m = m_ < 0 ? m_nm + m_ : m_ ; 

    //ULL ni = m_ni == 0 ? 1ull : ULL(m_ni) ;
    ULL nj = m_nj == 0 ? 1ull : ULL(m_nj) ;
    ULL nk = m_nk == 0 ? 1ull : ULL(m_nk) ;
    ULL nl = m_nl == 0 ? 1ull : ULL(m_nl) ;
    ULL nm = m_nm == 0 ? 1ull : ULL(m_nm) ;

    ULL ii = ULL(i) ; 
    ULL jj = ULL(j) ; 
    ULL kk = ULL(k) ; 
    ULL ll = ULL(l) ; 
    ULL mm = ULL(m) ; 

    return  ii*nj*nk*nl*nm + jj*nk*nl*nm + kk*nl*nm + ll*nm + mm ;
}

unsigned long long NPYBase::getNumValues(unsigned int from_dim) const 
{
    ULL numvals = 1 ; 
    for(unsigned int i=from_dim ; i < m_shape.size() ; i++) numvals *= ULL(m_shape[i]) ;
    return numvals ;  
}


bool NPYBase::HasSameItemSize(const NPYBase* a, const NPYBase* b)
{
    unsigned aItemValues = a->getNumValues(1) ;
    unsigned bItemValues = b->getNumValues(1) ;

    bool same = aItemValues == bItemValues ;  
    if(!same)
    {
        LOG(fatal) << "NPYBase::HasSameItemSize MISMATCH "
                  << " aShape " << a->getShapeString()
                  << " bShape " << b->getShapeString()
                  << " aItemValues " << aItemValues 
                  << " bItemValues " << bItemValues 
                  ;
    } 
    return same ; 
}









// depending on sizeoftype

unsigned long long NPYBase::getSizeOfType() const 
{
    return m_sizeoftype;
}
NPYBase::Type_t NPYBase::getType() const 
{
    return m_type;
}

bool NPYBase::isIntegerType() const 
{
    return m_type == SHORT || m_type == INT || m_type == UINT || m_type == CHAR || m_type == UCHAR || m_type == ULONGLONG ; 
}
bool NPYBase::isFloatType() const 
{
    return m_type == FLOAT || m_type == DOUBLE ;
}





 unsigned long long NPYBase::getNumBytes(unsigned int from_dim) const 
{
    ULL numval =  getNumValues(from_dim);
    return m_sizeoftype*numval ; 
}
 unsigned long long NPYBase::getByteIndex(unsigned i, unsigned j, unsigned k, unsigned l, unsigned m) const 
{
    return m_sizeoftype*getValueIndex(i,j,k,l,m);
}

 void NPYBase::setDynamic(bool dynamic)
{
    m_dynamic = dynamic ; 
}
 bool NPYBase::isDynamic() const 
{
    return m_dynamic ; 
}



unsigned int NPYBase::getNumQuads() const 
{
   unsigned int num_quad ;  
   unsigned int ndim = m_shape.size() ;
   unsigned int last_dimension = ndim > 1 ? m_shape[ndim-1] : 0  ;

   if(last_dimension != 4 )
   {
       LOG(fatal) << "NPYBase::getNumQuads last dim expected to be 4  " << getShapeString()  ;
       num_quad = 0 ; 
   } 
   else
   {
       num_quad = 1 ; 
       for(unsigned int i=0 ; i < ndim - 1 ; i++ ) num_quad *= m_shape[i] ; 
   } 
   return num_quad ;
}


bool NPYBase::hasSameShape(const NPYBase* other, unsigned fromdim) const 
{
    const std::vector<int>& a = getShapeVector();
    const std::vector<int>& b = other->getShapeVector();
    if(a.size() != b.size()) return false ; 
    unsigned int n = a.size();
    for(unsigned int i=fromdim ; i < n ; i++) if(a[i] != b[i]) return false ;
    return true ; 
}


bool NPYBase::HasShape(NPYBase* a , int ni, int nj, int nk, int nl, int nm) 
{
    return a ? a->hasShape(ni, nj, nk, nl, nm) : true ;  
}

bool NPYBase::hasShape(int ni, int nj, int nk, int nl, int nm) const 
{
    return 
           ( ni == -1 || int(m_ni) == ni) && 
           ( nj == -1 || int(m_nj) == nj) && 
           ( nk == -1 || int(m_nk) == nk) && 
           ( nl == -1 || int(m_nl) == nl) && 
           ( nm == -1 || int(m_nm) == nm) ;
}

bool NPYBase::hasItemShape(int nj, int nk, int nl, int nm) const 
{
    return 
           ( nj == -1 || int(m_nj) == nj) && 
           ( nk == -1 || int(m_nk) == nk) && 
           ( nl == -1 || int(m_nl) == nl) && 
           ( nm == -1 || int(m_nm) == nm) ;
}

bool NPYBase::hasItemSpec(const NPYSpec* item_spec) const 
{
    return m_item_spec->isEqualTo(item_spec); 
}

bool NPYBase::hasShapeSpec(const NPYSpec* shape_spec) const 
{
    return m_shape_spec->isEqualTo(shape_spec); 
}





void NPYBase::setNumItems(unsigned int ni)
{
    unsigned int orig = m_shape[0] ;
    //assert(ni >= orig);

    if(ni >= orig)
    {
       LOG(verbose)
                  << " increase from " << orig << " to " << ni 
                  ; 
    }
    else
    {
       LOG(verbose)
                  << " decrease from " << orig << " to " << ni 
                  ; 
    }
  
    m_shape[0] = ni ; 
    m_ni = ni ; 
}



/**
NPYBase::reshape
------------------

TODO: somehow make it possible to re-view an array of a different type
with shape changing accordingly 

See tests/NPY4Test.cc:test_getQuad_crossType_cast_FAILS_RESHAPE

**/

void NPYBase::reshape(int ni_, unsigned int nj, unsigned int nk, unsigned int nl, unsigned int nm)
{
    unsigned int nvals = std::max(1u,m_ni)*std::max(1u,m_nj)*std::max(1u,m_nk)*std::max(1u,m_nl)*std::max(1u,m_nm) ; 


    unsigned int njklm = std::max(1u,nj)*std::max(1u,nk)*std::max(1u,nl)*std::max(1u,nm) ; // NB excludes ni_
    unsigned int ni    = ni_ < 0 ? nvals/njklm : ni_ ;    // auto resizing of 1st dimension, when -ve
    unsigned int nvals2 = std::max(1u,ni)*std::max(1u,nj)*std::max(1u,nk)*std::max(1u,nl)*std::max(1u,nm) ; 

    // HMM: the -1 should bo need to be in the first slot : just needs to be only one of them within 


    if(nvals != nvals2) 
    {
         LOG(fatal) << "NPYBase::reshape INVALID AS CHANGES COUNTS " 
                              << " nvals " << nvals
                              << " nvals2 " << nvals2
                              ;
    }

    assert(nvals == nvals2 && "NPYBase::reshape cannot change number of values, just their addressing");

    LOG(debug) << "NPYBase::reshape (0 means no-dimension) "
              << "(" << m_ni << "," << m_nj << "," << m_nk << "," << m_nl << "," << m_nm << ")"
              << " --> "
              << "(" <<   ni << "," <<   nj << "," <<   nk << "," <<   nl << "," << nm << ")"
              ;

    m_shape.clear();
    if(ni > 0) m_shape.push_back(ni);
    if(nj > 0) m_shape.push_back(nj);
    if(nk > 0) m_shape.push_back(nk);
    if(nl > 0) m_shape.push_back(nl);
    if(nm > 0) m_shape.push_back(nm);

    updateDimensions();
}



std::string NPYBase::getDigestString() const 
{
    return getDigestString(getBytes(), getNumBytes(0));
}


std::string NPYBase::getDigestString(void* bytes, unsigned int nbytes) 
{
    return SDigest::digest(bytes, nbytes);
}


std::string NPYBase::getItemDigestString(unsigned i) const 
{
    assert( i < getNumItems() );

    unsigned long long bufSize =  getNumBytes(0);  // buffer size   
    unsigned long long itemSize = getNumBytes(1);  // item size in bytes (from dimension d)  

    assert( i*itemSize < bufSize );

    char* bytes = (char*)getBytes();
    assert(sizeof(char) == 1);

    return getDigestString( bytes + i*itemSize, itemSize );
}





bool NPYBase::isEqualTo(NPYBase* other) 
{
    return isEqualTo(other->getBytes(), other->getNumBytes(0));
}

bool NPYBase::isEqualTo(void* bytes, unsigned long long nbytes) 
{
    std::string self = getDigestString();
    std::string other = getDigestString(bytes, nbytes);

    bool same = self.compare(other) == 0 ; 

    if(!same)
    {
         LOG(warning) << "NPYBase::isEqualTo NO "
                      << " self " << self 
                      << " other " << other
                      ;
    }
 

    return same ; 
}




std::string NPYBase::getShapeString(unsigned int ifr) const 
{
    return getItemShape(ifr);
}

std::string NPYBase::getItemShape(unsigned int ifr) const 
{
    std::stringstream ss ; 
    for(size_t i=ifr ; i < m_shape.size() ; i++)
    {
        ss << m_shape[i]  ;
        if( i < m_shape.size() - 1) ss << "," ;
    }
    return ss.str(); 
}


void NPYBase::Summary(const char* msg) const 
{
    std::string desc = description(msg);
    LOG(info) << desc ; 
}   

std::string NPYBase::description(const char* msg) const 
{
    std::stringstream ss ; 

    ss << msg << " (" ;

    for(size_t i=0 ; i < m_shape.size() ; i++)
    {
        ss << m_shape[i]  ;
        if( i < m_shape.size() - 1) ss << "," ;
    }
    ss << ") " ;

    //ss << " ni " << m_ni ;
    //ss << " nj " << m_nj ;
    //ss << " nk " << m_nk ;
    //ss << " nl " << m_nl ;

    ss << " NumBytes(0) " << getNumBytes(0) ;
    ss << " NumBytes(1) " << getNumBytes(1) ;
    ss << " NumValues(0) " << getNumValues(0) ;
    ss << " NumValues(1) " << getNumValues(1) ;

    ss << m_metadata  ;

    return ss.str();
}


void NPYBase::saveMeta( const char* path, const char* ext) const 
{
    if(!m_meta) return ; 
    if(m_meta->size() == 0) return ; 

    std::string metapath_ = BFile::ChangeExt(path, ext);  
    const char* metapath = metapath_.c_str(); 

    LOG(debug) << " save to " << metapath ;      
    m_meta->save(metapath); 
}

BMeta* NPYBase::LoadMeta( const char* path, const char* ext) // static 
{
    std::string _metapath = BFile::ChangeExt(path, ext );  
    const char* metapath = _metapath.c_str(); 

    return BFile::ExistsFile(metapath) ? BMeta::Load(metapath) : NULL ;      
}





template NPY_API void NPYBase::setMeta(const char* key, char value);
template NPY_API void NPYBase::setMeta(const char* key, unsigned char value);
template NPY_API void NPYBase::setMeta(const char* key, unsigned value);
template NPY_API void NPYBase::setMeta(const char* key, unsigned long long value);
template NPY_API void NPYBase::setMeta(const char* key, short value);
template NPY_API void NPYBase::setMeta(const char* key, int value);
template NPY_API void NPYBase::setMeta(const char* key, float value);
template NPY_API void NPYBase::setMeta(const char* key, double value);
template NPY_API void NPYBase::setMeta(const char* key, std::string value);

template NPY_API char          NPYBase::getMeta(const char* key, const char* fallback) const ; 
template NPY_API unsigned char NPYBase::getMeta(const char* key, const char* fallback) const ; 
template NPY_API unsigned      NPYBase::getMeta(const char* key, const char* fallback) const ; 
template NPY_API unsigned long long  NPYBase::getMeta(const char* key, const char* fallback) const ; 
template NPY_API short         NPYBase::getMeta(const char* key, const char* fallback) const ; 
template NPY_API int           NPYBase::getMeta(const char* key, const char* fallback) const ; 
template NPY_API float         NPYBase::getMeta(const char* key, const char* fallback) const ; 
template NPY_API double        NPYBase::getMeta(const char* key, const char* fallback) const ; 
template NPY_API std::string   NPYBase::getMeta(const char* key, const char* fallback) const ; 


