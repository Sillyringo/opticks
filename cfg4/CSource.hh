#pragma once

class Opticks ; 
class CRecorder ; 

class G4Event ; 
class G4PrimaryVertex ; 
template<typename T> class NPY ; 

#include "CFG4_API_EXPORT.hh"
#include "CFG4_HEAD.hh"

/**
CSource(G4VPrimaryGenerator) : common functionality of the various source types
=======================================================================================

* abstract base class of CTorchSource, CGunSource, CInputPhotonSource 
* subclass of G4VPrimaryGenerator

The specialized prime method GeneratePrimaryVertex 
is invoked from CPrimaryGeneratorAction::GeneratePrimaries
by the Geant4 framework.

::

    [blyth@localhost cfg4]$ grep public\ CSource *.hh
    CGenstepSource.hh     :class CFG4_API CGenstepSource: public CSource
    CGunSource.hh         :class CFG4_API CGunSource: public CSource
    CInputPhotonSource.hh :class CFG4_API CInputPhotonSource: public CSource
    CPrimarySource.hh     :class CFG4_API CPrimarySource: public CSource
    CTorchSource.hh       :class CFG4_API CTorchSource: public CSource


**/

#include "G4VPrimaryGenerator.hh"

class CFG4_API CSource : public G4VPrimaryGenerator
{
  public:
    friend class CTorchSource ; 
    friend class CGunSource ; 
  public:
    CSource(Opticks* ok );
    void setRecorder(CRecorder* recorder);
    virtual ~CSource();
  public:
    virtual void GeneratePrimaryVertex(G4Event *evt) = 0 ;
  public:
    virtual NPY<float>* getSourcePhotons() const ;  // default implementation returning NULL  
  public:
     // to CPrimaryCollector
    void collectPrimaryVertex(const G4PrimaryVertex* vtx);
  protected: 
    Opticks*              m_ok ;  
    CRecorder*            m_recorder ; 
    unsigned              m_vtx_count ; 
};
#include "CFG4_TAIL.hh"

