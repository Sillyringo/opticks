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




#include <string>
#include "plog/Severity.h"
#include "G4Types.hh"
#include "CGenstep.hh"

class OpticksGenstep ; 
class NLookup ; 
template <typename T> class NPY ;

/**
CGenstepCollector : methods for collection of gensteps
=================================================================

Canonical CG4.m_collector is instanciated at postinitialize, 
and G4Opticks.m_genstep_collector instance is instanciated at G4Opticks::setGeometry/G4Opticks::createCollectors


G4 Independency 
----------------

NB there are no G4 classes used here, just a few type definitions
which could be removed easily. Users of this such as CSource should 
convert from G4 class instances into basic types for collection here.


Gensteps (item shape 6*4, 6 quads) 
-------------------------------------

First 3 quads are common between scintillation and
Cerenkov but the last 3 differ.

Furthermore the precise details of which quanties are included 
in the last three quads depends on the inner photon loop 
implementation details of the scintillation and Cerenkov processes.
They need to be setup whilst developing a corresponding GPU/OptiX 
implementation of the inner photon loop and doing 
equivalence comparisons.

Each implementation will need slightly different genstep and OptiX port.

Effectively the genstep can be regarded as the "stack" just 
prior to the photon generation loop.

**/

#include "CFG4_API_EXPORT.hh"

class CFG4_API CGenstepCollector 
{
        friend struct CGenstepCollector2Test ; 
        friend class  CG4 ;      // for addGenstep 
        friend class  OKG4Mgr ;  // for addGenstep 
    public:
        static const plog::Severity LEVEL ; 
        static CGenstepCollector* INSTANCE ;      
        static CGenstepCollector* Get();
    public:
        CGenstepCollector(const NLookup* lookup);  
    public:
        void setArrayContentIndex(unsigned eventId); 
        unsigned getArrayContentIndex() const ; 
        unsigned getNumGensteps() const ; 

        unsigned getNumPhotonsSum() const ; // result should be the same as getNumPhotons, but more slowly with large numbers of gensteps
        unsigned getNumPhotons() const ;    // m_photon_count

    public:
        unsigned getNumPhotons( unsigned gs_idx) const ; 
        char     getGentype(unsigned gs_idx) const ;
        unsigned getPhotonOffset(unsigned gs_idx) const ;
        const CGenstep& getGenstep(unsigned gs_idx) const ; 

    public:
        NPY<float>*  getGensteps() const ;
    public:
        std::string description() const ;
        std::string desc() const ;
        void Summary(const char* msg="CGenstepCollector::Summary") const  ;
        int translate(int acode) const ;
    public:
        void reset();          
        void save(const char* path);          
        void load(const char* path);          
    private:
        void consistencyCheck() const ;
        void import(); 
    public:
        void setReservation(int items);
        int getReservation() const ; 
    public:
        CGenstep collectScintillationStep(
            G4int                gentype, 
            G4int                parentId,
            G4int                materialId,
            G4int                numPhotons,
            
            G4double             x0_x,  
            G4double             x0_y,  
            G4double             x0_z,  
            G4double             t0, 

            G4double             deltaPosition_x, 
            G4double             deltaPosition_y, 
            G4double             deltaPosition_z, 
            G4double             stepLength, 

            G4int                pdgCode, 
            G4double             pdgCharge, 
            G4double             weight, 
            G4double             meanVelocity, 

            G4int                i40_scntId,  //  CAUTION: THE MEANINGS OF THESE LAST 8 VARY WITH IMPLEMENTATION/gentype
            G4double             d41_slowerRatio,
            G4double             d42_slowTimeConstant,
            G4double             d43_slowerTimeConstant,

            G4double             d50_scintillationTime,
            G4double             d51_scintillationIntegrationMax,
            G4double             d52_spare1,
            G4double             d53_spare2
        );
   public:
        CGenstep collectCerenkovStep(
            G4int                gentype, 
            G4int                parentId,
            G4int                materialId,
            G4int                numPhotons,
            
            G4double             x0_x,  
            G4double             x0_y,  
            G4double             x0_z,  
            G4double             t0, 

            G4double             deltaPosition_x, 
            G4double             deltaPosition_y, 
            G4double             deltaPosition_z, 
            G4double             stepLength, 

            G4int                pdgCode, 
            G4double             pdgCharge, 
            G4double             weight, 
            G4double             preVelocity, 

            G4double             betaInverse,
            G4double             pmin,
            G4double             pmax,
            G4double             maxCos,

            G4double             maxSin2,
            G4double             meanNumberOfPhotons1,
            G4double             meanNumberOfPhotons2,
            G4double             postVelocity
        );
   public:
         CGenstep collectMachineryStep(unsigned code);
         CGenstep collectTorchGenstep(const OpticksGenstep* gs);
   private:
         CGenstep addGenstep(unsigned numPhotons, char gentype);
   private:
         const NLookup*    m_lookup ; 
         NPY<float>*       m_genstep ;

         unsigned          m_genstep_itemsize ; 
         float*            m_genstep_values ;  

         unsigned          m_scintillation_count ; 
         unsigned          m_cerenkov_count ; 
         unsigned          m_torch_count ; 
         unsigned          m_torch_emitsource_count ;   // subset of m_torch_count 
         unsigned          m_machinery_count ; 
         unsigned          m_photon_count ; 

         std::vector<unsigned> m_gs_photons ; 
         std::vector<unsigned> m_gs_offset ; 
         std::vector<char>     m_gs_type ; 
         std::vector<CGenstep> m_gs ;   // huh: doesnt this duplicate the info of the above three ? 
};
