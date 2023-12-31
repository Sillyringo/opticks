#include "G4SystemOfUnits.hh"
#include "G4PhysicalConstants.hh"
#include "G4Electron.hh"

#include "SEvt.hh"
#include "SPath.hh"
#include "NP.hh"
#include "OPTICKS_LOG.hh"
#include "U4.hh"

#include "DsG4Scintillation.h"


struct DsG4ScintillationTest
{
    static const char* FOLD ; 

    SEvt*              evt ; 
    G4Material*        material ; 
    DsG4Scintillation* proc ; 
    G4VParticleChange* change  ; 

    DsG4ScintillationTest(int opticksMode);  
    void dump() const ; 
    void PostStepDoIt() ; 
    void save() const ; 
};

const char* DsG4ScintillationTest::FOLD = SPath::Resolve("$TMP/DsG4ScintillationTest", DIRPATH) ; 

DsG4ScintillationTest::DsG4ScintillationTest(int opticksMode)
    :
    evt(new SEvt),
    material(U4::MakeScintillator()),
    proc(new DsG4Scintillation(opticksMode)),
    change(nullptr)
{
}

void DsG4ScintillationTest::dump() const 
{
    assert(material); 
    G4MaterialPropertiesTable* mpt = material->GetMaterialPropertiesTable(); 
    G4double ScintillationYield = mpt->GetConstProperty("SCINTILLATIONYIELD");
    std::cout << "ScintillationYield " << ScintillationYield << std::endl ; 


    G4PhysicsTable* slow = proc->getSlowIntegralTable();
    G4PhysicsTable* fast = proc->getFastIntegralTable();
    G4PhysicsTable* reem = proc->getReemissionIntegralTable();

    std::cout 
         << " proc " << proc << std::endl
         << " slow " << slow << std::endl
         << " fast " << fast << std::endl
         << " reem " << reem << std::endl
         ; 
    //proc->DumpPhysicsTable(); 
}

void DsG4ScintillationTest::PostStepDoIt() 
{
    G4double BetaInverse = 1.5 ; 
    G4double en = 1.*MeV ;           // HMM: what about consistency here, does it matter for scintillation ?
    G4double step_length = 1.0*mm  ; 


    G4double beta = 1./BetaInverse ; 
    G4double pre_beta = beta ; 
    G4double post_beta = beta ; 

    G4ParticleMomentum momentum(0., 0., 1.); 
    G4DynamicParticle* particle = new G4DynamicParticle(G4Electron::Definition(),momentum);
    particle->SetPolarization(0., 0., 1. );  
    particle->SetKineticEnergy(en); 

    G4ThreeVector position(0., 0., 0.); 
    G4double time(0.); 

    G4Track* track = new G4Track(particle,time,position);

    G4StepPoint* pre = new G4StepPoint ; 
    G4StepPoint* post = new G4StepPoint ; 

    G4ThreeVector pre_position(0., 0., 0.);
    G4ThreeVector post_position(0., 0., 1.);

    pre->SetPosition(pre_position); 
    post->SetPosition(post_position); 

    pre->SetVelocity(pre_beta*c_light); 
    assert( pre->GetBeta() == pre_beta );  

    post->SetVelocity(post_beta*c_light); 
    assert( post->GetBeta() == post_beta );  

    // G4Track::GetMaterial comes from current step preStepPoint 
    pre->SetMaterial(material); 
    post->SetMaterial(material); 

    G4Step* step = new G4Step ; 
    step->SetPreStepPoint(pre); 
    step->SetPostStepPoint(post); 
    step->SetStepLength(step_length); 

    G4double TotalEnergyDeposit = 1.0*MeV ;   
    step->SetTotalEnergyDeposit(TotalEnergyDeposit);  
   
    track->SetStep(step); 

    change = proc->PostStepDoIt(*track, *step) ; 
}

void DsG4ScintillationTest::save() const 
{
    NP* p = U4::CollectOpticalSecondaries(change);  
    LOG(info) << " save to " << FOLD ; 
    p->save(FOLD, "p.npy"); 
    evt->saveGenstep(FOLD); 
}

int main(int argc, char** argv)
{
    OPTICKS_LOG(argc, argv); 

    int opticksMode = 3 ; 
    DsG4ScintillationTest t(opticksMode); 
    t.PostStepDoIt(); 
    t.save(); 

    return 0 ; 
}
