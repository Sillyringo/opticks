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

#include "CFG4_BODY.hh"

#include "Opticks.hh"
#include "OpticksMode.hh"
#include "OpticksEvent.hh"
#include "OpticksHub.hh"
#include "OpticksRun.hh"
#include "OpticksGen.hh"

#include "CG4.hh"
#include "G4StepNPY.hpp"
#include "CGenstep.hh"

#include "OPTICKS_LOG.hh"

/**
CG4Test
========

Excercises Opticks instrumentation without GPU propagation.

THIS NEEDS RETHINKING FOR THE CManager BASED WORKFLOW
TESTS NEED TO FOLLOW THE CURRENT APPROACH NOT DEMAND LOTS 
OF CODE TO KEEP THEM WORKING

BASIC PROBLEM IS HOW TO INTEGRATE ARTIFICIAL TORCH 
GENSTEPS INTO THE CManager APPROACH WHICH IS 
DRIVEN BY Geant4 run/event/track/step 


**/


int main(int argc, char** argv)
{
    OPTICKS_LOG(argc, argv);

    LOG(info) << argv[0] ;

    Opticks ok(argc, argv, "--save");

    OpticksHub hub(&ok) ; 
    LOG(info) << " post hub " ; 

    OpticksRun* run = ok.getRun();
    LOG(info) << " post run " ; 
    //OpticksGen* gen = hub.getGen();
    
    CG4* g4 = new CG4(&hub) ; 
    LOG(info) << " post CG4 " ; 

    g4->interactive();

    LOG(info) << "  post CG4::interactive"  ;


    if(ok.isFabricatedGensteps())  // eg TORCH running
    { 
        //NPY<float>* gs = gen->getInputGensteps() ;
        //unsigned numPhotons = G4StepNPY::CountPhotons(gs); 
        //LOG(info) << " setting gensteps " << gs << " numPhotons " << numPhotons ; 
        //char ctrl = '=' ; 
        //ok.createEvent(gs, ctrl);
        // ??????  CManager::BeginOfEventAction/CManager::presave creates event 
        //CGenstep cgs = g4->addGenstep(numPhotons, 'T' ); 
        // need to collect the genstep, not just bookkeep it

        unsigned num_photons = 100 ; 
        int node_index = -1 ; 
        unsigned originTrackID = 101 ;  
        CGenstep cgs = g4->collectDefaultTorchStep(num_photons, node_index, originTrackID );
        LOG(info) << " cgs " << cgs.desc() ; 
    }
    else
    {
        LOG(error) << " not setting gensteps " ; 
    }


    g4->propagate();

    LOG(info) << "  CG4 propagate DONE "  ;

    ok.postpropagate();

    OpticksEvent* evt = run->getG4Event();
    assert(evt->isG4()); 
    evt->save();

    LOG(info) << "  evt save DONE "  ;

    g4->cleanup();




    LOG(info) << "exiting " << argv[0] ; 

    return 0 ; 
}

