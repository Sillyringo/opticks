#pragma once

#include <string>
#include <map>

class NMeta ; 

#include "NPY_API_EXPORT.hh"

/**
NPYMeta : integer keyed map of NMeta dicts used for node metadata
=====================================================================

Primary usage so far is as the m_meta instance of NCSG node trees,
providing per-node metadata for the trees.

**/

class NPY_API NPYMeta
{
    public:
        static NMeta*       LoadMetadata(const char* treedir, int item=-1);
        static bool         ExistsMeta(const char* treedir, int item=-1);
    private:
        static const char*  META ; 
        static const char*  ITEM_META ; 
        static std::string  MetaPath(const char* treedir, int item=-1);
        enum { NUM_ITEM = 16 } ;  // default number of items to look for
    public:
        // item -1 corresponds to global metadata 
        NPYMeta(); 
        NMeta*  getMeta(int item=-1) const ;   
        bool          hasMeta(int idx) const ;
    public:
        int                       getIntFromString(const char* key, const char* fallback, int item=-1 ) const ;
        template<typename T> T    getValue(const char* key, const char* fallback, int item=-1 ) const ;
        template<typename T> void setValue(const char* key, T value, int item=-1);
    public:
        void load(const char* dir, int num_item = NUM_ITEM ) ;
        void save(const char* dir) const ;
    private:
        std::map<int, NMeta*>    m_meta ;    
        // could be a complete binary tree with loadsa nodes, so std::array not appropriate

};



