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
#include <vector>
#include <map>
#include "plog/Severity.h"
#include "G4ThreeVector.hh"

#include "G4OK_API_EXPORT.hh"

#include "CGenstep.hh"

template <typename T> class NPY ; 
class NLookup ; 
class Opticks;
class OpticksEvent;
class OpMgr;
class GGeo ; 
class GPho ; 
class GBndLib ; 

class SensorLib ; 

class CTraverser ; 
class CMaterialTable ; 
class CGenstepCollector ; 
class CPrimaryCollector ; 
class CPhotonCollector ; 
class C4PhotonCollector ; 

class G4Run;
class G4Event; 
class G4Track; 
class G4Step; 
class G4VPhysicalVolume ;
class G4VParticleChange ; 
class G4PVPlacement ; 

struct G4OpticksHit ; 
struct G4OpticksHitExtra ; 
struct G4OpticksRecorder ; 


#include "G4Types.hh"

/**
G4Opticks : interface for steering an Opticks instance embedded within a Geant4 application
==============================================================================================

setGeometry 
   called from BeginOfRunAction passing the world pointer over 
   in order to translate and serialize the geometry and copy it
   to the GPU  

addGenstep
   called from every step of modified Scintillation and Cerenkov processees
   in place of the optical photon generation loop, all the collected 
   gensteps are propagated together in a single GPU launch when propagate
   is called

propagateOpticalPhotons and getHits
   called from EndOfEventAction 

collectSecondaryPhotons
   invoked for example from L4Cerenkov::PostStepDoIt 


Overview
----------

1. Collect methods such as G4Opticks::collectGenstep_G4Cerenkov_1042  are invoked 
   to collect gensteps from within the G4Cerenkov/G4Scintillation processes 

2. When G4Opticks::propagateOpticalPhotons is called it passes the gensteps collected 
   to the underlying OpMgr which uploads them to the GPU and runs the propagation

3. Hits are then copied from GPU to CPU  

4. Once finished with accessing the hits and copying then into hit collections 
   G4Opticks::reset must be called to clear the OpticksEvents including the gensteps.

   * it is essential to call G4Opticks::reset to avoid duplicating the
     simulation of the same photons over and over, and also to avoid leaking 
     memory drastically 

5. At this stage after G4Opticks::reset you can continue with collecting Gens



Notes
-------

* :doc:`notes/issues/G4OK`

**/

class G4OK_API G4Opticks   
{
        friend class G4OKTest ; 
    private:
        static const plog::Severity LEVEL ;

        static const char* OPTICKS_EMBEDDED_COMMANDLINE ; 
        static const char* OPTICKS_EMBEDDED_COMMANDLINE_EXTRA ; 
        static const char* fEmbeddedCommandLine_dev ;
        static const char* fEmbeddedCommandLine_pro ;
        static std::string EmbeddedCommandLine(const char* extra1, const char* extra2); 
        static Opticks* InitOpticks(const char* keyspec, const char* commandline_extra, bool parse_argv );
    public:
        // workaround for getting config to the deferred Opticks instanciation
        void        setEmbeddedCommandLineExtra(const char* extra); 
        const char* getEmbeddedCommandLineExtra() const ; 
    public:
        static G4Opticks* Get();
        static void Initialize(const char* gdmlpath, bool standardize_geant4_materials);
        static void Initialize(const G4VPhysicalVolume* world, bool standardize_geant4_materials);
    public:
        G4Opticks();
        virtual ~G4Opticks();
    public:
        std::string desc() const ;  
        const char* dbgdesc() const ;  
        std::string dbgdesc_() const ;
        void setProfile(bool profile);
        void setProfilePath(const char* path);
        void setProfileLeakMB(float profile_leak_mb);
    public:
        // workflow methods
        int propagateOpticalPhotons(G4int eventID);
    public:
        NPY<float>* getHits() const ; 
        void saveHits(const char* path) const ; 
        void saveHits(const char* dir, const char* name) const ; 
        void saveHits(const char* dir=nullptr, const char* name_prefix="ht_", int name_index=1, const char* ext=".npy") const ; 
        void dumpHits(const char* msg="G4Opticks::dumpHits") const ;
    public:
        NPY<float>* getGensteps() const ; 
        void saveGensteps(const char* path) const ; 
        void saveGensteps(const char* dir, const char* name) const ; 
        void saveGensteps(const char* dir=nullptr, const char* name_prefix="gs_", int name_index=1, const char* ext=".npy") const ; 
    public:
        void reset(); 
        void setAlignIndex(int align_idx) const ; 

        static void Finalize();
        void finalize() const ; 
    private:
        void finalizeProfile() const ;
    public:
        bool isLoadedFromCache() const ;
        unsigned getNumSensorVolumes() const ; 
        unsigned getSensorIdentityStandin(unsigned sensorIndex) const ;        // pre-cache and post-cache 
        const std::vector<G4PVPlacement*>& getSensorPlacements() const ;  // pre-cache live running only 
        unsigned getNumDistinctPlacementCopyNo() const  ; 

    public:
        // via SensorLib 
        void setSensorData(unsigned sensorIndex, float efficiency_1, float efficiency_2, int sensor_category, int sensor_identifier);
        void getSensorData(unsigned sensorIndex, float& efficiency_1, float& efficiency_2, int& category, int& identifier) const ;
        int  getSensorIdentifier(unsigned sensorIndex) const ;
    public:
        // via SensorLib
        void setSensorAngularEfficiency( const std::vector<int>& shape, const std::vector<float>& values, 
                                         int theta_steps=181, float theta_min=0.f, float theta_max=180.f, 
                                         int phi_steps=1,   float phi_min=0.f, float phi_max=360.f );
        void setSensorAngularEfficiency( const NPY<float>* sensor_angular_efficiency );
        void saveSensorLib(const char* dir, const char* reldir=NULL) const ;
        //void uploadSensorLib() ;
        void render_snap() ;
    public:
        void setGeometry(const G4VPhysicalVolume* world); 
        void setGeometry(const char* gdmlpath);
        void setGeometry(const G4VPhysicalVolume* world, bool standardize_geant4_materials); 
        void loadGeometry(); 
        //void saveGParts() const ; 
    public:
        bool isWayEnabled() const ; 
        unsigned getWayMask() const ; 
    public:
        void setStandardizeGeant4Materials(bool standardize_geant4_materials);
        void setPlacementOuterVolume(bool outer_volume);  // TODO: eliminate
    private:
        void setGeometry(const GGeo* ggeo); 
    private:
        GGeo* translateGeometry( const G4VPhysicalVolume* top );

        void standardizeGeant4MaterialProperties();
        void createCollectors();
        void resetCollectors(); 
        void setupMaterialLookup();
    public:
        unsigned getNumPhotons() const ;
        unsigned getNumPhotonsSum() const ;
        unsigned getNumGensteps() const ;
        unsigned getMaxGensteps() const ; // default of zero, means no limit

        void setGenstepReservation(int max_gensteps_expected);
        int  getGenstepReservation() const ;

    public:
        CGenstep collectGenstep_G4Cerenkov_1042(  
             const G4Track*  aTrack, 
             const G4Step*   aStep, 
             G4int       numPhotons,

             G4double    betaInverse,
             G4double    pmin,
             G4double    pmax,
             G4double    maxCos,

             G4double    maxSin2,
             G4double    meanNumberOfPhotons1,
             G4double    meanNumberOfPhotons2
            );

        CGenstep collectGenstep_G4Scintillation_1042(  
             const G4Track* aTrack, 
             const G4Step* aStep, 
             G4int    numPhotons, 
             G4int    ScintillationType,
             G4double ScintillationTime, 
             G4double ScintillationRiseTime
             );

        CGenstep collectGenstep_DsG4Scintillation_r3971(  
             const G4Track* aTrack, 
             const G4Step* aStep, 
             G4int    numPhotons, 
             G4int    scnt,          //  1:fast 2:slow
             G4double slowerRatio,
             G4double slowTimeConstant,
             G4double slowerTimeConstant,
             G4double ScintillationTime
            );

        CGenstep collectGenstep_DsG4Scintillation_r4695(  
             const G4Track* aTrack, 
             const G4Step* aStep, 
             G4int    numPhotons, 
             G4int    scnt,         
             G4double ScintillationTime
            );


    private:
        CGenstep collectDefaultTorchStep(unsigned num_photons=0, int node_index=-1, unsigned originTrackID=44 );  // zero -> default num_photons
    public:
        void collectSecondaryPhotons(const G4VParticleChange* pc);
    public:
        void collectHit(
            G4double             pos_x,  
            G4double             pos_y,  
            G4double             pos_z,  
            G4double             time ,

            G4double             dir_x,  
            G4double             dir_y,  
            G4double             dir_z,  
            G4double             weight ,

            G4double             pol_x,  
            G4double             pol_y,  
            G4double             pol_z,  
            G4double             wavelength ,

            G4int                flags_x, 
            G4int                flags_y, 
            G4int                flags_z, 
            G4int                flags_w
         ); 

        unsigned getNumHit() const ; 
        void getHit(unsigned i, G4OpticksHit* hit, G4OpticksHitExtra* hit_extra ) const ; 
     private:
        void initSkipGencode() ;
        void dumpSkipGencode() const ;
        bool isSkipGencode(unsigned gencode) const ;

     public:
        // for debugging  
        void setInputPhotons(const char* dir, const char* name, int repeat=0, const char* wavelength=nullptr, int eventID=0) ;
        void setInputPhotons(const char* path, int repeat=0, const char* wavelength=nullptr, int eventID=0) ;
        void setInputPhotons(NPY<float>* input_photons, int repeat=0, const char* wavelength=nullptr, int eventID=0 ) ;
     public:
        void setSave(bool save=true);
       
     private:
        bool                       m_standardize_geant4_materials ; 
        bool                       m_placement_outer_volume ; 
        const G4VPhysicalVolume*   m_world ; 
        const GGeo*                m_ggeo ; 
        const GBndLib*             m_blib ; 
        GPho*                      m_hits_wrapper ; // geometry aware hits wrapper
        G4OpticksRecorder*         m_recorder ; 
        
        const char*                m_embedded_commandline_extra ; 
        Opticks*                   m_ok ;
        bool                       m_way_enabled ; 
        unsigned                   m_way_mask ; 
        CTraverser*                m_traverser ; 
        CMaterialTable*            m_mtab ; 
        CGenstepCollector*         m_genstep_collector ; 
        CPrimaryCollector*         m_primary_collector ; 
        NLookup*                   m_lookup ; 
        OpMgr*                     m_opmgr;
    private:
        // transient pointers borrowed from the collectors 
        NPY<float>*                m_gensteps ; 
        NPY<float>*                m_genphotons ; 
        NPY<float>*                m_hits ; 
        NPY<float>*                m_hiys ; 
        unsigned                   m_num_hits ; 
        unsigned                   m_num_hiys ; 
    private:
        // minimal instrumentation from the G4 side of things 
        CPhotonCollector*          m_g4hit_collector ; 
        C4PhotonCollector*         m_g4photon_collector ; 
        unsigned                   m_genstep_idx ; 
    private:
        OpticksEvent*              m_g4evt ; 
        NPY<float>*                m_g4hit ; 
    private:
        std::vector<G4PVPlacement*> m_sensor_placements ;
        SensorLib*                  m_sensorlib ; 
        std::vector<int>            m_skip_gencode ; 
        unsigned                    m_skip_gencode_count ;  
        std::map<int, int>          m_skip_gencode_totals ; 
        bool                        m_profile ; 
        const char*                 m_profile_path ; 
        float                       m_profile_leak_mb ; 
        std::vector<float>          m_profile_stamps ; 
    private:
        static G4Opticks*          fInstance;

};



