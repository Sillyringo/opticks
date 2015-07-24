#pragma once
#include "stdlib.h"
#include <string>
#include <cstring>

class GCache {
    public:
         static const char* JUNO ; 
         static const char* DAYABAY ; 
    public:
         GCache(const char* envprefix);
         const char* getIdPath();
         const char* getEnvPrefix();
         bool idPathContains(const char* s); 
         void Summary(const char* msg="GCache::Summary");
    public:
         const char* getDetector();
         bool        isJuno();
         bool        isDayabay();

    private:
          void init();
          void readEnvironment();  
    private:
          const char* m_envprefix ; 
          const char* m_geokey ;
          const char* m_path ;
          const char* m_query ;
          const char* m_ctrl ;
          const char* m_idpath ;
          const char* m_digest ;
    private:
          bool        m_dayabay ; 
          bool        m_juno ; 
          const char* m_detector ;
};


inline GCache::GCache(const char* envprefix)
       :
       m_envprefix(strdup(envprefix)),
       m_geokey(NULL),
       m_path(NULL),
       m_query(NULL),
       m_ctrl(NULL),
       m_idpath(NULL),
       m_digest(NULL),
       m_dayabay(false),
       m_juno(false),
       m_detector(NULL)
{
       init();
}

inline const char* GCache::getIdPath()
{
    return m_idpath ;
}
inline const char* GCache::getEnvPrefix()
{
    return m_envprefix ;
}


inline const char* GCache::getDetector()
{
    return m_detector ;
}
inline bool GCache::isJuno()
{
   return m_juno ; 
}
inline bool GCache::isDayabay()
{
   return m_dayabay ; 
}




inline bool GCache::idPathContains(const char* s)
{
    std::string idp(m_idpath);
    std::string ss(s);
    return idp.find(ss) != std::string::npos ;
}





