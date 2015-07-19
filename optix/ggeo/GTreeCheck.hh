#pragma once

#include <string>
#include <map>
#include <vector>

class GGeo ; 
class GNode ; 
class GSolid ; 
template<class T> class Counts ;

class GTreeCheck {
   public:
        typedef std::map<std::string, std::vector<GNode*> > MSVN ; 
   public:
        GTreeCheck(GGeo* ggeo);
   public:
        void init();
        void traverse();
        unsigned int getRepeatIndex(const std::string& pdig );
        void labelTree();
   public:
        bool operator()(const std::string& dig) ;
   private:
        void traverse( GNode* node, unsigned int depth );
        void findRepeatCandidates(unsigned int minrep);
        void dumpRepeatCandidates();
        void dumpRepeatCandidate(unsigned int index, bool verbose=false);
        bool isContainedRepeat( const std::string& pdig, unsigned int levels ) const ;
        void labelTree( GNode* node, unsigned int depth);
   private:
       GGeo*                     m_ggeo ; 
       GSolid*                   m_root ; 
       unsigned int              m_count ;  
       unsigned int              m_labels ;  
       Counts<unsigned int>*     m_digest_count ; 
       std::vector<std::string>  m_repeat_candidates ; 
 
};


inline GTreeCheck::GTreeCheck(GGeo* ggeo) 
       :
       m_ggeo(ggeo),
       m_root(NULL),
       m_count(0),
       m_labels(0)
       {
          init();
       }




