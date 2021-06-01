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

#include <sstream>

#include "G4OpticalPhoton.hh"
#include "G4Track.hh"
#include "G4Event.hh"

#include "OpticksFlags.hh"
#include "OpticksGenstep.hh"
#include "OpticksEvent.hh"
#include "Opticks.hh"

#include "CBoundaryProcess.hh"
#include "CProcessManager.hh"
#include "CEvent.hh"
#include "CTrack.hh"
#include "CG4Ctx.hh"
#include "CEventInfo.hh"
#include "CTrackInfo.hh"

#include "PLOG.hh"


const plog::Severity CG4Ctx::LEVEL = PLOG::EnvLevel("CG4Ctx", "DEBUG") ; 

const unsigned CG4Ctx::CK = OpticksGenstep::SourceCode("G4Cerenkov_1042");   
const unsigned CG4Ctx::SI = OpticksGenstep::SourceCode("G4Scintillation_1042"); 
const unsigned CG4Ctx::TO = OpticksGenstep::SourceCode("fabricated");   


CG4Ctx::CG4Ctx(Opticks* ok)
    :
    _ok(ok),
    _pindex(ok->getPrintIndex(0)),
    _print(false),
    _genstep_num_photons(0),
    _genstep_index(-1)
{
    init();

    _dbgrec = ok->isDbgRec() ;   // machinery debugging 
    _dbgseq = ok->getDbgSeqhis() || ok->getDbgSeqmat() ;  // content debugging 
    _dbgzero = ok->isDbgZero() ; 
}


Opticks*  CG4Ctx::getOpticks() const
{
    return _ok ; 
}


bool CG4Ctx::is_dbg() const 
{
    return _dbgrec || _dbgseq || _dbgzero ;
}


/**
CG4Ctx::step_limit
-------------------

*step_limit* is used by CRec::addStp (recstp) the "canned" step collection approach, 
which just collects steps and makes sense of them later...
This has the disadvantage of needing to collect StepTooSmall steps (eg from BR turnaround)  
that are subsequently thrown : this results in the step limit needing to 
be twice the size you might expect to handle hall-of-mirrors tboolean-truncate.

This also has disadvantage that tail consumption as checked with "--utaildebug" 
does not match between Opticks and Geant4, see 

* notes/issues/ts-box-utaildebug-decouple-maligned-from-deviant.rst


**/

unsigned CG4Ctx::step_limit() const 
{
    assert( _ok_event_init ); 
    return 1 + 2*( _steps_per_photon > _bounce_max ? _steps_per_photon : _bounce_max ) ;
}

/**
CG4Ctx::point_limit
---------------------

Returns the larger of the below:

_steps_per_photon
     number of photon step points to record into record array
_bounce_max
     number of bounces before truncation, often 1 less than _steps_per_photon but need not be  

*point_limit* is used by CRec::addPoi (recpoi) the "live" point collection approach, 
which makes sense of the points as they arrive, 
this has advantage of only storing the needed points. 

DO NOT ADD +1 LEEWAY HERE : OTHERWISE TRUNCATION BEHAVIOUR IS CHANGED
see notes/issues/cfg4-point-recording.rst

**/

unsigned CG4Ctx::point_limit() const 
{
    assert( _ok_event_init ); 
    return ( _steps_per_photon > _bounce_max ? _steps_per_photon : _bounce_max ) ;
    //return _bounce_max  ;
}



void CG4Ctx::init()
{
    _dbgrec = false ; 
    _dbgseq = false ; 
    _dbgzero = false ; 

    _photons_per_g4event = 0 ; 
    _steps_per_photon = 0 ; 
    _record_max = 0 ;
    _bounce_max = 0 ; 

    _ok_event_init = false ; 
    _event = NULL ; 
    _event_id = -1 ; 
    _event_total = 0 ; 
    _event_track_count = 0 ; 
    _gen = 0 ; 
    _genflag = 0 ; 
    

    _track = NULL ; 
    _process_manager = NULL ; 
    _track_id = -1 ; 
    _track_total = 0 ;
    _track_step_count = 0 ;  

    _parent_id = -1 ; 
    _optical = false ; 
    _pdg_encoding = 0 ; 

    _primary_id = -1 ; 
    _photon_id = -1 ; 
    _record_id = -1 ; 
    _mask_index = -1 ; 

    _reemtrack = false ; 

    _rejoin_count = 0 ; 
    _primarystep_count = 0 ; 
    _stage = CStage::UNKNOWN ; 

    _record_id = -1 ; 
    _debug = false ; 
    _other = false ; 
    _dump = false ; 
    _dump_count = 0 ; 


    _step = NULL ; 
    _noZeroSteps = -1 ; 
    _step_id = -1 ;
    _step_total = 0 ; 

#ifdef USE_CUSTOM_BOUNDARY
    _boundary_status = Ds::Undefined ; 
    _prior_boundary_status = Ds::Undefined ; 
#else
    _boundary_status = Undefined ; 
    _prior_boundary_status = Undefined ; 
#endif

}


std::string CG4Ctx::desc_stats() const 
{
    std::stringstream ss ; 
    ss << "CG4Ctx::desc_stats"
       << " dump_count " << _dump_count  
       << " event_total " << _event_total 
       << " event_track_count " << _event_track_count
       ;
    return ss.str();
}

/**
CG4Ctx::initEvent
--------------------

Collect the parameters of the OpticksEvent which 
dictate what needs to be collected.

**/

void CG4Ctx::initEvent(const OpticksEvent* evt)
{
    _ok_event_init = true ;
    _photons_per_g4event = evt->getNumPhotonsPerG4Event() ; 
    _steps_per_photon = evt->getMaxRec() ;   // number of points to be recorded into record buffer   
    _record_max = evt->getNumPhotons();      // from the genstep summation

    _bounce_max = evt->getBounceMax();       // maximum bounce allowed before truncation will often be 1 less than _steps_per_photon but need not be 
    unsigned bounce_max_2 = evt->getMaxBounce();    
    assert( _bounce_max == bounce_max_2 ) ; // TODO: eliminate or rename one of those


    const char* typ = evt->getTyp();
    //  _gen = OpticksFlags::SourceCode(typ);   MOVED TO FINER LEVEL OF BELOW setEvent 

    LOG(LEVEL)
        << " _record_max (numPhotons from genstep summation) " << _record_max 
        << " photons_per_g4event " << _photons_per_g4event
        << " _steps_per_photon (maxrec) " << _steps_per_photon
        << " _bounce_max " << _bounce_max
        << " typ " << typ
        ;

}

std::string CG4Ctx::desc_event() const 
{
    std::stringstream ss ; 
    ss << "CG4Ctx::desc_event" 
       << " photons_per_g4event " << _photons_per_g4event
       << " steps_per_photon " << _steps_per_photon
       << " record_max " << _record_max
       << " bounce_max " << _bounce_max
       << " _gen " << _gen
       << " _genflag " << _genflag
       ;
    return ss.str();
}


/**
CG4Ctx::setEvent
-----------------

Invoked by CManager::BeginOfEventAction

**/

void CG4Ctx::setEvent(const G4Event* event) 
{
     //OKI_PROFILE("CG4Ctx::setEvent") ; 

    _event = const_cast<G4Event*>(event) ; 
    _event_id = event->GetEventID() ;

    _event_total += 1 ; 
    _event_track_count = 0 ; 
    _photon_count = 0 ; 

    _genstep_index = -1 ; 


    CEventInfo* eui = (CEventInfo*)event->GetUserInformation(); 
    //assert(eui && "expecting event UserInfo set by eg CGenstepSource "); 
    if(eui)
    {
        //assert(0) ; // doesnt really make sense to do this at event level 
        unsigned gen = eui->gencode ;
        setGen(gen); 
    }

    _number_of_input_photons = CEvent::NumberOfInputPhotons(event); 
    LOG(LEVEL) << "_number_of_input_photons " << _number_of_input_photons ; 

    // when _number_of_input_photons is greater than 0 
    // CManager "mocks" a genstep by calling CG4Ctx::setGenstep
    // in normal running that gets called from CManager::BeginOfGenstep
}


/**
CG4Ctx::setGenstep
--------------------

HMM: in general cannot set this at event level, 
it can only be set by checking on the generating process of first photon 
So using this setGen from setEvent only makes sense with artifical gensteps  

Perhaps the ProcessSubType can help with doing this more generally:: 

   104   SetProcessSubType(fCerenkov);


BUT: Opticks (with suitable opticksMode to do both G4 and OK) 
is active at genstep collection in viscinity of photon generation.
So can consult G4Opticks to know what genstep is currently active.
Will need to bookend the photon generation corresponding 
to the genstep collected.  So can plant a genstep index. 



HMM: where to invoke this with normal G4Opticks S+C running ?

**/


void CG4Ctx::BeginOfGenstep(char gentype, int num_photons)
{
    setGentype(gentype); 
    _genstep_num_photons = num_photons ; 
    _genstep_index += 1 ;    // _genstep_index starts at -1 and is reset to -1 by CG4Ctx::setEvent, so it becomes a zero based event local index

    LOG(fatal) 
        << " _gentype [" << _gentype << "]"
        << " _genstep_num_photons " << _genstep_num_photons 
        << " _genstep_index " << _genstep_index 
        ;
}

void CG4Ctx::EndOfGenstep()
{
    LOG(fatal) 
        << " _gentype [" << _gentype << "]"
        << " _genstep_num_photons " << _genstep_num_photons 
        << " _genstep_index " << _genstep_index 
        ;

}



void CG4Ctx::setGentype(char gentype)
{
    switch(gentype)
    {
        case 'S':setGen(SI) ; break ; 
        case 'C':setGen(CK) ; break ; 
        case 'T':setGen(TO) ; break ; 
        default: { LOG(fatal) << " gentype invalid [" << gentype << "]" ; assert(0) ; }   ;
    } 
    _gentype = gentype ; 
}


/**
CG4Ctx::setGen
----------------

**/

void CG4Ctx::setGen(unsigned gen)
{
    _gen = gen ;
    _genflag = OpticksGenstep::GenstepToPhotonFlag(_gen); 

    bool valid = OpticksGenstep::IsValid(_gen) ; 

    LOG(LEVEL) 
        << " gen " << _gen
        << " OpticksGenstep::GenType " << OpticksGenstep::Gentype(_gen) 
        << " OpticksFlags::SourceType " << OpticksFlags::SourceType(_gen)
        << " OpticksFlags::Flag " << OpticksFlags::Flag(_genflag)
        << " valid " << valid 
        ;

    assert( valid );
}



/**
CG4Ctx::setTrack
------------------

Invoked by CTrackingAction::setTrack

**/

void CG4Ctx::setTrack(const G4Track* track) 
{
    G4ParticleDefinition* particle = track->GetDefinition();

    _track = const_cast<G4Track*>(track) ; 
    _track_id = CTrack::Id(track) ;    // 0-based id 

    _process_manager = CProcessManager::Current(_track);

    _track_step_count = 0 ; 
    _event_track_count += 1 ; 
    _track_total += 1 ;
    
    _parent_id = CTrack::ParentId(track) ;
    _optical = particle == G4OpticalPhoton::OpticalPhotonDefinition() ;
    _pdg_encoding = particle->GetPDGEncoding();

    _step = NULL ; 
    _step_id = -1 ; 
    _step_id_valid = -1 ; 

    LOG(LEVEL) 
        << " _track_id " << _track_id
        << " track.GetGlobalTime " << track->GetGlobalTime()
        << " _parent_id " << _parent_id
        << " _pdg_encoding " << _pdg_encoding
        << " _optical " << _optical 
        << " _process_manager " << CProcessManager::Desc(_process_manager)
        ;

    if(_optical) setTrackOptical();
}


/**
CG4Ctx::setTrackOptical
--------------------------

Invoked by CG4Ctx::setTrack

Hmm: seems natural to adopt CTrackInfo::opticks_photon_id() to keep track 
of photon lineage thru RE-emission generations as it has direct correspondence
to the GPU photon index and to gensteps.

But the opticks_photon_id should always be available and >= 0, so 
it cannot be used to give _reemtrack ?

So, what exactly was the old::

    CTrack::PrimaryPhotonID(_track) 
    DsPhotonTrackInfo::GetPrimaryPhotonID()  (aka as this)

Think the old way was to rely on the UserTrackInfo being set only 
for reemitted scintillation photons, hence presence of it was 
used to identify _reemtrack with the default fPrimaryPhotonID(-1) signally 
not reemission.  That seems an obtuse way of yielding _reemtrack

**/

void CG4Ctx::setTrackOptical() 
{
    LOG(debug) << "CTrackingAction::setTrack setting UseGivenVelocity for optical " ; 

    _track->UseGivenVelocity(true);

    // NB without this BoundaryProcess proposed velocity to get correct GROUPVEL for material after refraction 
    //    are trumpled by G4Track::CalculateVelocity 

    // _primary_id = CTrack::PrimaryPhotonID(_track) ;    // layed down in trackinfo by custom Scintillation process
    // _photon_id = _primary_id >= 0 ? _primary_id : _track_id ; 
    // _reemtrack = _primary_id >= 0 ? true : false ; // <-- critical input to _stage set by subsequent CG4Ctx::setStepOptical 

    // dynamic_cast gives NULL when using the wrong type for the pointer
    CTrackInfo* tkui = dynamic_cast<CTrackInfo*>(_track->GetUserInformation());   // NEW ADDITION : NEEDS INTEGRATING 

    // lack of tkui should be only with artifically input/generated photons 
    // as both C+S photons should be always be labelled

    _primary_id = tkui ? tkui->photon_id() : _track_id ;   // 0-based
    char tkui_gentype = tkui ? tkui->gentype() : 'T'  ;    // TODO: maybe not needed, following addition of setGensteps ?

    // assert( _primary_id >= 0 && tkui_gentype != '?' );   // require all optical tracks to have been annotated with CTrackInfo 
    _photon_id = _primary_id  ; 
    _reemtrack = tkui ? tkui->reemission() : false ; // <-- critical input to _stage set by subsequent CG4Ctx::setStepOptical 

    _photon_count += 1 ;   // CAREFUL : DOES NOT ACCOUNT FOR RE-JOIN 

     // retaining original photon_id from prior to reemission effects the continuation
    _record_id = _photons_per_g4event*_event_id + _photon_id ;   
     //  THIS WAS TO WORKAROUND Geant4 PROBLEM WITH MILLIONS OF PHOTONS IN ONE EVENT
     //  BY SPLITTING INTO MULTIPLE EVENTS 
     //
    _record_fraction = double(_record_id)/double(_record_max) ;  

    LOG(LEVEL) 
        << " _record_id " << _record_id 
        << " _primary_id " << _primary_id 
        << " _reemtrack " << _reemtrack
        << " tkui_gentype " << tkui_gentype
        << " _track.GetGlobalTime " << _track->GetGlobalTime()
        ;
    setGentype(tkui_gentype);     // TODO: maybe not needed, following addition of setGensteps ?




    _mask_index = _ok->hasMask() ?_ok->getMaskIndex( _record_id ) : -1 ;   // "original" index 

    //if(_record_id % 1000 == 0) OKI_PROFILE("CG4Ctx::setTrackOptical_1k"); 

    // moved from CTrackingAction::setTrack
    _debug = _ok->isDbgPhoton(_record_id) ; // from option: --dindex=1,100,1000,10000 
    _other = _ok->isOtherPhoton(_record_id) ; // from option: --oindex=1,100,1000,10000 
    _dump = _debug || _other ; 

    _print = _pindex > -1 && _record_id == _pindex ; 


    if(_dump) _dump_count += 1 ; 


    // moved from  CSteppingAction::setPhotonId
    // essential for clearing counts otherwise, photon steps never cleared 
    _rejoin_count = 0 ; 
    _primarystep_count = 0 ; 
}

unsigned CG4Ctx::getNumTrackOptical() const 
{
    return _photon_count ; 
}




/**
CG4Ctx::setStep
----------------

Invoked by CSteppingAction::setStep

**/

void CG4Ctx::setStep(const G4Step* step, int noZeroSteps) 
{
    _step = const_cast<G4Step*>(step) ; 
    _noZeroSteps = noZeroSteps ;  
    _step_id = CTrack::StepId(_track);
    if(_noZeroSteps == 0) _step_id_valid += 1;

    _step_total += 1 ; 
    _track_step_count += 1 ; 

    const G4StepPoint* pre = _step->GetPreStepPoint() ;
    const G4StepPoint* post = _step->GetPostStepPoint() ;

    _step_pre_status = pre->GetStepStatus();
    _step_post_status = post->GetStepStatus();

    if(_step_id == 0)
    {
        _step_origin = pre->GetPosition();
    }

    if(_optical) setStepOptical();
}


/**
CG4Ctx::setStepOptical
-----------------------

Invoked by CG4Ctx::setStep 

1. sets _stage to START/COLLECT/REJOIN/RECOLL depending on _reemtrack and prior step counts
2. sets _boundary_status

**/

void CG4Ctx::setStepOptical() 
{
    if( !_reemtrack )     // primary photon, ie not downstream from reemission 
    {
        _stage = _primarystep_count == 0  ? CStage::START : CStage::COLLECT ;
        _primarystep_count++ ; 
    } 
    else 
    {
        _stage = _rejoin_count == 0  ? CStage::REJOIN : CStage::RECOLL ;   
        _rejoin_count++ ; 
        // rejoin count is zeroed in setTrackOptical, so each remission generation trk will result in REJOIN 
    }

    _prior_boundary_status = _boundary_status ; 
    _boundary_status = CBoundaryProcess::GetOpBoundaryProcessStatus() ;

    LOG(LEVEL) 
        <<  " _prior_boundary_status " << std::setw(35) << CBoundaryProcess::OpBoundaryString(_prior_boundary_status)
        <<  " _boundary_status " << std::setw(35) << CBoundaryProcess::OpBoundaryString(_boundary_status)
        ;
}




std::string CG4Ctx::desc_step() const 
{
    G4TrackStatus track_status = _track->GetTrackStatus(); 

    std::stringstream ss ; 
    ss << "CG4Ctx::desc_step" 
       << " step_total " << _step_total
       << " event_id " << _event_id
       << " track_id " << _track_id
       << " track_step_count " << _track_step_count
       << " step_id " << _step_id
       << " trackStatus " << CTrack::TrackStatusString(track_status)
       ;

    return ss.str();
}


std::string CG4Ctx::brief() const 
{
    std::stringstream ss ; 
    ss 
        << " record_id " << _record_id
        ;
 
    return ss.str();
}
 

std::string CG4Ctx::desc() const 
{
    std::stringstream ss ; 
    ss 
        << ( _dbgrec ? " [--dbgrec] " : "" )
        << ( _dbgseq ? " [--dbgseqmat 0x.../--dbgseqhis 0x...] " : "" )
        << ( _debug ? " --dindex " : "" )
        << ( _other ? " --oindex " : "" )
        << " record_id " << _record_id
        << " event_id " << _event_id
        << " track_id " << _track_id
        << " photon_id " << _photon_id
        << " parent_id " << _parent_id
        << " primary_id " << _primary_id
        << " reemtrack " << _reemtrack
        ;
    return ss.str();
}



