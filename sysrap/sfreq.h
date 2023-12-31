#pragma once
/**
sfreq.h : count occurrence frequencies of strings and sorts by frequencies
============================================================================

Canonical usage is for geometry progeny digests 

* subs are collected by stree::classifySubtrees


**/

#include <cassert>
#include <vector>
#include <string>
#include <cstring>
#include <algorithm>
#include <sstream>
#include <iostream>
#include <iomanip>
#include "NPFold.h"

struct sfreq_matchkey 
{
    const char* query ; 
    sfreq_matchkey(const char* query_) : query(query_) {}

    bool operator()(const std::pair<std::string, int>& p) const 
    { 
        return strcmp(query, p.first.c_str()) == 0 ;
    }
};

struct sfreq
{
    typedef std::pair<std::string,int> SU ;   
    typedef typename std::vector<SU>        VSU  ; 
    typedef typename VSU::const_iterator    IT ;   

    VSU vsu ; 

    unsigned get_num() const ; 
    void get_keys(std::vector<std::string>& keys, int freq_cut) const ; 

    const char* get_key(unsigned idx) const ; 

    int get_freq(unsigned idx) const ; 
    int get_freq(const char* key) const ; 

    int find_index(const char* key) const ; 
    void add(const char* key ); 

    bool is_disqualify( const char* key) const ; 
    void set_disqualify(const char* key) ; 
    void set_disqualify(const std::vector<std::string>& disqualify) ; 


    static bool ascending_freq( const SU& a, const SU& b) ; 
    static bool descending_freq(const SU& a, const SU& b) ; 
    void sort(bool descending=true);  

    std::string desc(const char* sub) const ; 
    std::string desc(unsigned idx) const ; 
    std::string desc() const ; 

    static constexpr const char* KEY = "key.npy" ; 
    static constexpr const char* VAL = "val.npy" ; 
    size_t get_maxkeylen() const ; 
    int get_total() const ; 

    NP* make_key() const ; 
    NP* make_val() const ; 
    void import_key_val( const NP* key, const NP* val); 

    void save(const char* dir) const ; 
    void save(const char* dir, const char* reldir) const ; 

    void load(const char* dir); 
    void load(const char* dir, const char* reldir); 

    NPFold* serialize() const ; 
    void import(const NPFold* fold); 

};


inline unsigned sfreq::get_num() const
{
    return vsu.size();  
} 

inline void sfreq::get_keys(std::vector<std::string>& keys, int freq_cut) const
{
    for(unsigned i=0 ; i < vsu.size() ; i++)
    {
        const char* key = get_key(i); 
        int freq  = get_freq(i); 
        if(freq > freq_cut) keys.push_back(key) ;  
    }
}


inline const char* sfreq::get_key(unsigned idx) const
{
    assert( idx < vsu.size() ); 
    return vsu[idx].first.c_str() ; 
}
inline int sfreq::get_freq(unsigned idx) const
{
    assert( idx < vsu.size() ); 
    return vsu[idx].second ; 
}

inline int sfreq::get_freq(const char* key) const
{
    int idx = find_index(key); 
    return idx == -1 ? -1 : int(vsu[idx].second) ; 
}

inline int sfreq::find_index(const char* key) const 
{
    sfreq_matchkey mk(key);  
    IT it = std::find_if( vsu.begin(), vsu.end(), mk ); 
    return it == vsu.end() ? -1 : std::distance( vsu.begin(), it ); 
}

/**
sfreq::add : maybe "add_unique_count" would be a better name 
---------------------------------------------------------------

Adding sub keys and counts to a vector of pairs::

   (subtree_digest, freq)

**/
inline void sfreq::add(const char* key)
{
    int idx = find_index(key); 
    if( idx == -1 ) vsu.push_back(SU(key, 1u)) ; 
    else vsu[idx].second += 1 ;  
}


inline bool sfreq::is_disqualify(const char* key) const 
{
   int freq = get_freq(key); 
   return freq < 0 ;    
}

inline void sfreq::set_disqualify(const char* key)
{
    if(is_disqualify(key)) return ; 
    int idx = find_index(key); 
    assert( idx > -1 && vsu[idx].second > 0 ); 
    vsu[idx].second = -vsu[idx].second ; 
}

inline void sfreq::set_disqualify(const std::vector<std::string>& disqualify)
{
    for(unsigned i=0 ; i < disqualify.size() ; i++)
    {
        const char* sub = disqualify[i].c_str(); 
        set_disqualify(sub); 
    }
}





inline bool sfreq::ascending_freq(const SU& a, const SU& b)  // static
{
    return b.second > a.second ;
}
inline bool sfreq::descending_freq(const SU& a, const SU& b) // static 
{
    return a.second > b.second ;
}
inline void sfreq::sort(bool descending) 
{
    std::sort(vsu.begin(), vsu.end(), descending ? descending_freq : ascending_freq );
}

inline std::string sfreq::desc(const char* sub) const 
{
    int idx = find_index(sub); 
    return idx > -1 ? desc(unsigned(idx)) : "-" ; 
}

inline std::string sfreq::desc(unsigned idx) const 
{
    std::stringstream ss ; 
    const SU& su = vsu[idx] ;  
    const std::string& k = su.first ; 
    int v = su.second ; 
    ss << std::setw(5) << idx 
       << " : " 
       << std::setw(32) << k.c_str()
       << " : " 
       << std::setw(5) << v
       ;  
    std::string s = ss.str(); 
    return s ; 
}

inline std::string sfreq::desc() const 
{
    int total = get_total(); 
    std::stringstream ss ; 
    for(unsigned idx=0 ; idx < vsu.size() ; idx++) ss << desc(idx) << std::endl ; 
    ss << std::setw(5) << "" << " : " << std::setw(32) << "" << " : " << std::setw(5) << total << std::endl ;  
    std::string s = ss.str(); 
    return s ; 
}

inline size_t sfreq::get_maxkeylen() const 
{
    size_t mx = 0 ; 
    for(unsigned i=0 ; i < vsu.size() ; i++) mx = std::max(mx, strlen(vsu[i].first.c_str())) ; 
    return mx ;  
}

inline int sfreq::get_total() const
{
    int total = 0 ; 
    for(unsigned i=0 ; i < vsu.size() ; i++) total += std::abs(vsu[i].second) ; 
    return total ; 
}





/**
sfreq::make_key
-----------------

HMM: this uses an awkward approach of using a char array.
Alternate would be to just use set_names on the val array.  

In [5]: t.key.view("|S5").ravel()
Out[5]: array([b'blue', b'red', b'green'], dtype='|S5')

In [6]: t.key.shape
Out[6]: (3, 5)
**/


inline NP* sfreq::make_key() const 
{
    if(vsu.size()==0) return nullptr ; 
    size_t mkl = get_maxkeylen() ; 
    NP* key = NP::Make<char>( vsu.size(), mkl ) ;
    char* kdat = key->values<char>(); 

    for(unsigned i=0 ; i < vsu.size() ; i++)
    {
        const std::pair<std::string, int> su = vsu[i] ;  
        const char* k = su.first.c_str() ; 
        for(unsigned j=0 ; j < strlen(k) ; j++) kdat[i*mkl+j] = k[j] ; 
    }
    return key ; 
}

inline NP* sfreq::make_val() const 
{
    if(vsu.size()==0) return nullptr ; 
    NP* val = NP::Make<int>( vsu.size() ) ; 
    int* vdat = val->values<int>(); 

    for(unsigned i=0 ; i < vsu.size() ; i++)
    {
        const std::pair<std::string, int> su = vsu[i] ;  
        vdat[i] = su.second ; 
    }
    return val ;
} 

inline void sfreq::import_key_val( const NP* key, const NP* val)
{
    if(key == nullptr || val == nullptr) return ; 
    assert( key->shape.size() ); 

    unsigned keylen = key->shape[1] ; 
    const char* kdat = key->cvalues<char>(); 
    const int* vdat = val->cvalues<int>(); 

    assert( key->shape[0] == val->shape[0]) ; 
    unsigned num_kv = key->shape[0] ; 
 
    for(unsigned i=0 ; i < num_kv ; i++)
    {
        const char* kptr = kdat+i*keylen ; 
        std::string k(kptr, kptr+keylen) ; 
        int v = vdat[i] ; 
        vsu.push_back(std::pair<std::string, int>(k,v) );  
    }
}



inline void sfreq::save(const char* dir) const 
{
    if(vsu.size() == 0) return ; 

    NPFold* fold = serialize() ; 
    fold->save(dir); 
}

inline void sfreq::save(const char* dir, const char* reldir) const 
{
    if(vsu.size() == 0) return ; 
    NPFold* fold = serialize() ; 
    fold->save(dir, reldir); 
}

inline void sfreq::load(const char* dir)
{
    NPFold* fold = NPFold::Load(dir) ; 
    import(fold); 
}
inline void sfreq::load(const char* dir, const char* reldir)
{
    NPFold* fold = NPFold::Load(dir, reldir); 
    import(fold); 
}


inline NPFold* sfreq::serialize() const 
{
    NPFold* fold = new NPFold ; 
    fold->add( KEY, make_key() ); 
    fold->add( VAL, make_val() ); 
    return fold ; 
}

inline void sfreq::import(const NPFold* fold)
{
    const NP* key = fold->get(KEY) ; 
    const NP* val = fold->get(VAL);  
    import_key_val(key, val); 
}



