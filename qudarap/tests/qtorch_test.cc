/**
qtorch_test.cc : CPU tests of qtorch.h CUDA code using mocking 
================================================================

Standalone compile and run with::

   ./qtorch_test.sh 

**/
#include <numeric>
#include <vector>

#include "scuda.h"
#include "squad.h"
#include "qcurand.h"    // this brings in s_mock_curand.h for CPU 
#include "qsim.h"
#include "qtorch.h"
#include "NP.hh"

const char* FOLD = "/tmp/qtorch_test" ; 

void fill_torch_genstep( torch& gs, unsigned genstep_id, unsigned numphoton_per_genstep )
{
   // TODO: string configured gensteps, rather the the currently fixed and duplicated one  
    float3 mom = make_float3( 1.f, 1.f, 1.f ); 
    gs.wavelength = 501.f ; 
    gs.mom = normalize(mom); 
    gs.radius = 100.f ; 
    gs.zenith = make_float2( 0.f, 1.f ); 
    gs.azimuth = make_float2( 0.f, 1.f ); 
    gs.type = torchtype::Type("disc");  
    gs.mode = 255 ;    //torchmode::Type("...");  
    gs.numphoton = numphoton_per_genstep  ; 
}

NP* make_torch_gs(unsigned num_gs, unsigned numphoton_per_genstep )
{
    NP* gs = NP::Make<float>(num_gs, 6, 4 ); 
    torch* tt = (torch*)gs->bytes() ; 
    for(unsigned i=0 ; i < num_gs ; i++ ) fill_torch_genstep( tt[i], i, numphoton_per_genstep ) ; 
    return gs ;  
}

NP* make_seed(const NP* gs)
{
    assert( gs->has_shape(-1,6,4) ); 
    int num_gs = gs->shape[0] ; 
    const torch* tt = (torch*)gs->bytes() ; 

    std::vector<int> gsp(num_gs) ; 
    for(int i=0 ; i < num_gs ; i++ ) gsp[i] = tt[i].numphoton ;

    int tot_photons = 0 ; 
    for(int i=0 ; i < num_gs ; i++ ) tot_photons += gsp[i] ; 
    printf("//tot_photons %d \n", tot_photons ) ; 

    NP* se = NP::Make<int>( tot_photons ); 
    int* sev = se->values<int>();  

    int offset = 0 ; 
    for(int i=0 ; i < num_gs ; i++) 
    {   
        int np = gsp[i] ; 
        for(int p=0 ; p < np ; p++) sev[offset+p] = i ; 
        offset += np ; 
    }   
    return se ; 
}   

NP* make_torch_photon( const NP* gs, const NP* se )
{
    const torch* gg = (torch*)gs->bytes() ;  
    const int*   seed = (int*)se->bytes() ;  

    const qsim<float>* sim = new qsim<float>() ; 
    curandStateXORWOW rng(1u); 

    int tot_photon = se->shape[0] ; 
    NP* ph = NP::Make<float>( tot_photon, 4, 4); 
    photon* pp = (photon*)ph->bytes() ; 

    for(int i=0 ; i < tot_photon ; i++ )
    {
        unsigned photon_id = i ; 
        unsigned genstep_id = seed[photon_id] ; 

        photon& p = pp[photon_id] ; 
        const torch& g = gg[genstep_id] ;  
        
        qtorch::generate(p, rng, g, photon_id, genstep_id ); 

        std::cout << p.desc() << std::endl;  
    }
    return ph ; 
}

int main(int argc, char** argv)
{
    unsigned num_gs = 10 ; 
    unsigned numphoton_per_gs = 100 ; 

    NP* gs = make_torch_gs(num_gs, numphoton_per_gs ) ; 
    NP* se = make_seed(gs) ; 
    NP* ph = make_torch_photon(gs, se); 

    printf("save to %s\n", FOLD );
    gs->save(FOLD, "gs.npy"); 
    se->save(FOLD, "se.npy"); 
    ph->save(FOLD, "ph.npy"); 

    return 0 ; 
}

