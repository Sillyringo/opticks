#pragma once
/**
U4HitGet.h
=================

See: u4/tests/U4HitTest.cc


**/

#include "scuda.h"
#include "sphoton.h"
#include "sphit.h"

#include "SEvt.hh"
#include "U4Hit.h"
#include "U4ThreeVector.h"

struct U4HitGet
{
    static void ConvertFromPhoton(U4Hit& hit, const sphoton& global, const sphoton& local, const sphit& ht ); 
    static void FromEvt(U4Hit& hit, unsigned idx );  
}; 

inline void U4HitGet::ConvertFromPhoton(U4Hit& hit,  const sphoton& global, const sphoton& local, const sphit& ht )
{
    hit.zero(); 

    U4ThreeVector::FromFloat3( hit.global_position,      global.pos ); 
    U4ThreeVector::FromFloat3( hit.global_direction,     global.mom ); 
    U4ThreeVector::FromFloat3( hit.global_polarization,  global.pol ); 

    hit.time = double(global.time) ; 
    hit.weight = 1. ; 
    hit.wavelength = double(global.wavelength); 

    U4ThreeVector::FromFloat3( hit.local_position,      local.pos ); 
    U4ThreeVector::FromFloat3( hit.local_direction,     local.mom ); 
    U4ThreeVector::FromFloat3( hit.local_polarization,  local.pol ); 

    hit.sensorIndex = ht.sensor_index ;   
    hit.sensor_identifier = ht.sensor_identifier ; 
    hit.nodeIndex = -1 ; 

    hit.boundary = global.boundary() ; 
    hit.photonIndex = global.idx() ; 
    hit.flag_mask = global.flagmask ; 
    hit.is_cerenkov = global.is_cerenkov() ; 
    hit.is_reemission = global.is_reemit() ; 
}

inline void U4HitGet::FromEvt(U4Hit& hit, unsigned idx )
{
    sphoton global, local  ;
    SEvt* sev = SEvt::Get(); 
    sev->getHit( global, idx);

    sphit ht ; 
    sev->getLocalHit( ht, local,  idx);

    ConvertFromPhoton(hit, global, local, ht ); 
}



