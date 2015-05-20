#pragma once
#include <string>

class NPY ;
class MultiVecNPY ;

class NumpyEvt {
   public:
       NumpyEvt();
      
   public:
       void setGenstepData(NPY* genstep_data);

   public:
       NPY*         getGenstepData();
       NPY*         getPhotonData();
       MultiVecNPY* getGenstepAttr();
       MultiVecNPY* getPhotonAttr();

   public:
       unsigned int getNumPhotons();
       std::string description(const char* msg);

   public:
       void         dumpPhotonData();
       static void  dumpPhotonData(NPY* photon_data);
   private:
       void setPhotonData(NPY* photon_data);

   private:
       NPY*           m_genstep_data ;
       NPY*           m_photon_data ;
       MultiVecNPY*   m_genstep_attr ;
       MultiVecNPY*   m_photon_attr  ;
       unsigned int   m_num_photons ; 

};


inline NumpyEvt::NumpyEvt() 
          :
          m_genstep_data(NULL),
          m_photon_data(NULL),
          m_genstep_attr(NULL),
          m_photon_attr(NULL),
          m_num_photons(0)
{
}

inline NPY* NumpyEvt::getGenstepData()
{
    return m_genstep_data ;
}
inline MultiVecNPY* NumpyEvt::getGenstepAttr()
{
    return m_genstep_attr ;
}

inline NPY* NumpyEvt::getPhotonData()
{
    return m_photon_data ;
}
inline MultiVecNPY* NumpyEvt::getPhotonAttr()
{
    return m_photon_attr ;
}


inline unsigned int NumpyEvt::getNumPhotons()
{
    return m_num_photons ; 
}





