#pragma once
/**
CustomART : Modifies G4OpBoundaryProcess with Custom calc of A/R/T coeffs
===========================================================================

* CustomART<JPMT> is instanciated in G4OpBoundaryProcess ctor
* aims to provides customization with minimal code change to G4OpBoundaryProcess 
* aims to make few detector specific assumptions by moving such specifics into 
  the template type that provides PMT parameter access

What detector/Geant4 geometry specific assumptions are made ?
---------------------------------------------------------------

1. ART customized for surfaces with names starting with '@' on local_z > 0. 
2. pmtid obtained from Geant4 volume ReplicaNumber
3. templated PMT Parameter accessor type fulfils an API::

    get_pmtcat(int pmtid) const ; 
    get_pmtid_qe( int pmtid, double energy) const ;  
    get_stackspec( std::array<double, 16>& a_spec, int pmtcat, double energy_eV ) const ; 
    // HMM: thats assuming 4 layer stack 
 

Overview
----------

Trying to do less than CustomBoundary, instead just calculate::

    theTransmittance
    theReflectivity
    theEfficiency 

TODO: should also probably be setting::
   
   type = dielectric_dielectric 
   theFinish = polished 

With everything else (deciding on ARTD, changing mom, pol, theStatus)
done by the nearly "standard" G4OpBoundaryProcess.   


Is 2-layer (Pyrex,Vacuum) polarization direction calc applicable to 4-layer (Pyrex,ARC,PHC,Vacuum) situation ? 
-----------------------------------------------------------------------------------------------------------------

The Geant4 calculation of polarization direction for a simple
boundary between two layers is based on continuity of E and B fields
in S and P directions at the boundary, essentially Maxwells Eqn boundary conditions.
Exactly the same thing yields Snells law::

    n1 sin t1  = n2 sin t2 

My thinking on this is that Snell's law with a bunch of layers would be::

    n1 sin t1 = n2 sin t2 =  n3 sin t3 =  n4 sin t4 

So the boundary conditions from 1->4 where n1 and n4 are real still gives::

    n1 sin t1 = n4 sin t4

Even when n2,t2,n3,t3 are complex.

So by analogy that makes me think that the 2-layer polarization calculation 
between layers 1 and 4 (as done by G4OpBoundaryProcess) 
should still be valid even when there is a stack of extra layers 
inbetween layer 1 and 4. 

Essentially the stack calculation changes A,R,T so it changes
how much things happen : but it doesnt change what happens. 
So the two-layer polarization calculation from first and last layer 
should still be valid to the situation of the stack.

Do you agree with this argument ? 

**/

#include "G4ThreeVector.hh"

#include "IPMTAccessor.h"
#include "Layr.h"  
#include "U4Touchable.h"

struct CustomART
{
    int    count ; 
    double zlocal ; 
    double lposcost ; 

    const IPMTAccessor* accessor ; 

    G4double& theTransmittance ;
    G4double& theReflectivity ;
    G4double& theEfficiency ;

    const G4ThreeVector& theGlobalPoint ; 
    const G4ThreeVector& OldMomentum ; 
    const G4ThreeVector& OldPolarization ; 
    const G4ThreeVector& theRecoveredNormal ; 
    const G4double& thePhotonMomentum ; 

    CustomART(
        const IPMTAccessor* accessor, 
        G4double& theTransmittance,
        G4double& theReflectivity,
        G4double& theEfficiency,
        const G4ThreeVector& theGlobalPoint,  
        const G4ThreeVector& OldMomentum,  
        const G4ThreeVector& OldPolarization,
        const G4ThreeVector& theRecoveredNormal,
        const G4double& thePhotonMomentum
    );  

    //char maybe_doIt(const char* OpticalSurfaceName, const G4Track& aTrack, const G4Step& aStep) ;  
    double local_z( const G4Track& aTrack ); 
    void doIt(const G4Track& aTrack, const G4Step& aStep ); 
}; 

inline CustomART::CustomART(
    const IPMTAccessor* accessor_, 
          G4double& theTransmittance_,
          G4double& theReflectivity_,
          G4double& theEfficiency_,
    const G4ThreeVector& theGlobalPoint_,
    const G4ThreeVector& OldMomentum_,
    const G4ThreeVector& OldPolarization_,
    const G4ThreeVector& theRecoveredNormal_,
    const G4double&      thePhotonMomentum_
    )
    :
    count(0),
    zlocal(-1.),
    lposcost(-2.),
    accessor(accessor_),
    theTransmittance(theTransmittance_),
    theReflectivity(theReflectivity_),
    theEfficiency(theEfficiency_),
    theGlobalPoint(theGlobalPoint_),
    OldMomentum(OldMomentum_),
    OldPolarization(OldPolarization_),
    theRecoveredNormal(theRecoveredNormal_),
    thePhotonMomentum(thePhotonMomentum_) 
{
}

/**
CustomART::is_upper_z
------------------------

Q:What is lposcost for ?  

A:Preparing for doing this on GPU, as lposcost is available there already but zlocal is not, 
  so want to check the sign of lposcost is following that of zlocal. It looks 
  like it should:: 

    157 inline double Hep3Vector::cosTheta() const {
    158   double ptot = mag();
    159   return ptot == 0.0 ? 1.0 : dz/ptot;
    160 }


**/

inline double CustomART::local_z( const G4Track& aTrack )
{
    const G4AffineTransform& transform = aTrack.GetTouchable()->GetHistory()->GetTopTransform();
    G4ThreeVector localPoint = transform.TransformPoint(theGlobalPoint);
    zlocal = localPoint.z() ; 
    lposcost = localPoint.cosTheta() ;  
    return zlocal  ; 
}


/**
CustomART::maybe_doIt
------------------------

As need to also handle traditional POM it avoids duplication 
for the maybe_doIt branching to happen at a higher level inside CustomG4OpBoundaryProcess

inline char CustomART::maybe_doIt(const char* OpticalSurfaceName, const G4Track& aTrack, const G4Step& aStep )
{
    if( OpticalSurfaceName == nullptr || OpticalSurfaceName[0] != '@') return 'N' ; 

    return doIt(aTrack, aStep) ;
}

**/



/**
CustomART<J>::doIt
--------------------

NB stack is flipped for minus_cos_theta > 0. so:

* stack.ll[0] always incident side
* stack.ll[3] always transmission side 

**/

inline void CustomART::doIt(const G4Track& aTrack, const G4Step& aStep )
{
    G4double minus_cos_theta = OldMomentum*theRecoveredNormal ; 

    G4double energy = thePhotonMomentum ; 
    G4double wavelength = twopi*hbarc/energy ;
    G4double energy_eV = energy/eV ;
    G4double wavelength_nm = wavelength/nm ; 

    const G4VTouchable* touch = aTrack.GetTouchable();    
    int pmtid = U4Touchable::ReplicaNumber(touch);

    int pmtcat = accessor->get_pmtcat( pmtid ) ; 
    double _qe = minus_cos_theta > 0. ? 0.0 : accessor->get_pmtid_qe( pmtid, energy ) ;  
    // following the old junoPMTOpticalModel with "backwards" _qe always zero 

    std::array<double,16> a_spec ; 
    accessor->get_stackspec(a_spec, pmtcat, energy_eV ); 
    StackSpec<double,4> spec ; 
    spec.import( a_spec ); 

    bool dump = false ; 

    if(dump) std::cerr << "CustomART::doIt" << " spec " << std::endl << spec << std::endl ; 

    // SUSPECT: there will be significant repetition of get_stackspec 
    // calls with same pmtcat and energy_eV : so could try caching ?

    Stack<double,4> stack(wavelength_nm, minus_cos_theta, spec );  

    const double _si = stack.ll[0].st.real() ; 
    const double _si2 = sqrtf( 1. - minus_cos_theta*minus_cos_theta ); 

    double E_s2 = _si > 0. ? (OldPolarization*OldMomentum.cross(theRecoveredNormal))/_si : 0. ; 
    E_s2 *= E_s2;      

    // E_s2 : S-vs-P power fraction : signs make no difference as squared
    // E_s2 matches E1_perp*E1_perp see sysrap/tests/stmm_vs_sboundary_test.cc 

    double one = 1.0 ; 
    double S = E_s2 ; 
    double P = one - S ; 

    if(dump) std::cerr
        << "CustomART::doIt"
        << " count " << count 
        << " pmtid " << pmtid
        << " _si " << std::fixed << std::setw(10) << std::setprecision(5) << _si 
        << " _si2 " << std::fixed << std::setw(10) << std::setprecision(5) << _si2 
        << " theRecoveredNormal " << theRecoveredNormal 
        << " OldPolarization*OldMomentum.cross(theRecoveredNormal) " << OldPolarization*OldMomentum.cross(theRecoveredNormal) 
        << " E_s2 " << std::fixed << std::setw(10) << std::setprecision(5) << E_s2 
        << std::endl 
        ;    

    double T = S*stack.art.T_s + P*stack.art.T_p ;  // matched with TransCoeff see sysrap/tests/stmm_vs_sboundary_test.cc
    double R = S*stack.art.R_s + P*stack.art.R_p ;
    double A = one - (T+R);

    theTransmittance = T ; 
    theReflectivity  = R ; 

    if(dump) std::cerr
        << "CustomART::doIt"
        << " count " << count 
        << " S " << std::fixed << std::setw(10) << std::setprecision(5) << S 
        << " P " << std::fixed << std::setw(10) << std::setprecision(5) << P
        << " T " << std::fixed << std::setw(10) << std::setprecision(5) << T 
        << " R " << std::fixed << std::setw(10) << std::setprecision(5) << R
        << " A " << std::fixed << std::setw(10) << std::setprecision(5) << A
        << std::endl 
        ;    

    // stackNormal is not flipped (as minus_cos_theta is fixed at -1.) presumably this is due to _qe definition
    Stack<double,4> stackNormal(wavelength_nm, -1. , spec ); 

    double An = one - (stackNormal.art.T + stackNormal.art.R) ; 
    double D = _qe/An;   // LOOKS STRANGE TO DIVIDE BY An  : HMM MAYBE NEED TO DIVIDE BY A TOO ? 
 
    theEfficiency = D ; 

    bool expect = D <= 1. ; 
    if(!expect) std::cerr
        << "CustomART::doIt"
        << " FATAL "
        << " ERR: D > 1. : " << D 
        << " _qe " << _qe
        << " An " << An
        << std::endl 
        ;

    assert( expect ); 
}


