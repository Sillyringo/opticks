#pragma once

#include <map>
#include <string>
#include <vector>

class Opticks ; 

class GVolume ; 
class GNode ; 
class GNodeLib ; 
class GItemList ; 

class GTreePresent ; 


#include "GGEO_API_EXPORT.hh"

/*

GNodeLib
===========

Collection of GVolume/GNode instances with access by index.
NB only pv/lv names are persisted, not the volumes/nodes.
The analytic and test ctor arguments determine the name of the 
persisting directory.

Initially was primarily a pre-cache operator, but access to pv/lv names also 
relevant post-cache.

There are several canonical m_nodelib instances:

*GGeo::init precache non-analytic*

     874 void GGeo::add(GVolume* volume)
     875 {
     876     m_nodelib->add(volume);
     877 }


*GScene::GScene analytic*

     m_nodelib(loaded ? GNodeLib::Load(m_ok, m_analytic ) : new GNodeLib(m_ok, m_analytic)), 

     893 void GScene::addNode(GVolume* node, nd* n)
     894 {
     895     unsigned node_idx = n->idx ;
     896     assert(m_nodes.count(node_idx) == 0);
     897     m_nodes[node_idx] = node ;
     898 
     899     // TODO ... get rid of above, use the nodelib 
     900     m_nodelib->add(node);
     901 }

*/

class GGEO_API GNodeLib 
{
        friend class GGeo   ;  // for save 
        friend class GScene ;  // for save 
    public:
        static const char* GetRelDir(bool analytic, bool test);
        static GNodeLib* Load(Opticks* ok, bool analytic, bool test);
        void loadFromCache();
    public:
        GNodeLib(Opticks* opticks, bool analytic, bool test, GNodeLib* basis=NULL ); 
        std::string desc() const ; 
        void dump(const char* msg="GNodeLib::dump") const ;
    private:
        void save() const ;
        void init();
        GItemList*   getPVList(); 
        GItemList*   getLVList(); 
    public:
        GNodeLib* getBasis() const ;    
        unsigned getNumPV() const ;
        unsigned getNumLV() const ;
        void add(GVolume*    volume);
        GNode* getNode(unsigned index) const ; 
        GVolume* getVolume(unsigned int index) const ;  
        GVolume* getVolumeSimple(unsigned int index);  
        unsigned getNumVolumes() const ;
    public:
        const char* getPVName(unsigned int index) const ;
        const char* getLVName(unsigned int index) const ;
    private:
        Opticks*                           m_ok ;  
        bool                               m_analytic ; 
        bool                               m_test ; 
        GNodeLib*                          m_basis ; 
        const char*                        m_reldir ; 

        GItemList*                         m_pvlist ; 
        GItemList*                         m_lvlist ; 
        GTreePresent*                      m_treepresent ; 
    private:
        std::map<unsigned int, GVolume*>    m_volumemap ; 
        std::vector<GVolume*>               m_volumes ; 
};
 

