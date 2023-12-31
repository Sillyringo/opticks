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


#include <cassert>

#ifdef WITH_OPTICKS
#include "PLOG.hh"
#include "SSys.hh"
#include "G4TransportationManager.hh"
#include "G4Opticks.hh"
#include "G4OpticksRecorder.hh"
#endif

#include "Ctx.hh"
#include "RunAction.hh"

RunAction::RunAction(Ctx* ctx_) 
    :   
    G4UserRunAction(),
    ctx(ctx_)
{
}
void RunAction::BeginOfRunAction(const G4Run* run)
{
#ifdef WITH_OPTICKS
    G4cout << "\n\n###[ RunAction::BeginOfRunAction G4Opticks.setGeometry\n\n" << G4endl ; 
    G4VPhysicalVolume* world = G4TransportationManager::GetTransportationManager()->GetNavigatorForTracking()->GetWorldVolume() ; 
    assert( world ) ; 

    G4Opticks* g4ok = G4Opticks::Get(); 
    bool standardize_geant4_materials = false ;   

    //const char* embedded_commandline_extra = "--skipaheadstep 1000" ; // see ~/opticks/notes/issues/raja_repeated_photons.rst 

    const char* embedded_commandline_extra = SSys::getenvvar("CKM_OPTICKS_EXTRA", "" );   

    LOG(info) << "embedded_commandline_extra (CKM_OPTICKS_EXTRA)" << embedded_commandline_extra ; 

    g4ok->setEmbeddedCommandLineExtra(embedded_commandline_extra);
    g4ok->setGeometry(world, standardize_geant4_materials );    

    const std::vector<G4PVPlacement*>& sensor_placements = g4ok->getSensorPlacements() ;
    for(unsigned i=0 ; i < sensor_placements.size()  ; i++)
    {
        float efficiency_1 = 0.5f ; 
        float efficiency_2 = 1.0f ; 
        int sensor_cat = -1 ;                   // -1:means no angular efficiency info 
        int sensor_identifier = 0xc0ffee + i ;  // mockup a detector specific identifier
        unsigned sensorIndex = 1+i ;            // 1-based
        g4ok->setSensorData( sensorIndex, efficiency_1, efficiency_2, sensor_cat, sensor_identifier );  
    }

    G4cout << "\n\n###] RunAction::BeginOfRunAction G4Opticks.setGeometry\n\n" << G4endl ; 

    if( ctx->_recorder )
    {
        ctx->_recorder->BeginOfRunAction(run); 
    }
#endif
}
void RunAction::EndOfRunAction(const G4Run* run)
{
#ifdef WITH_OPTICKS

    if( ctx->_recorder )
    {
        ctx->_recorder->EndOfRunAction(run); 
    }

    G4cout << "\n\n###[ RunAction::EndOfRunAction G4Opticks.Finalize\n\n" << G4endl ; 
    G4Opticks::Finalize();
    G4cout << "\n\n###] RunAction::EndOfRunAction G4Opticks.Finalize\n\n" << G4endl ; 
#endif
}

