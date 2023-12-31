/**
QCKTest.cc
=============

Loads ICDF created and persisted by QCerenkovIntegralTest 
and uses it to create Cerenkov energy samples for a variety of BetaInverse 
by both lookup sampling and for comparison by the traditional rejection sampling approach.

**/

#include <iostream>
#include <iomanip>
#include <random>

#include "SStr.hh"
#include "SRng.hh"
#include "SPath.hh"
#include "QCK.hh"
#include "NP.hh"
#include "OPTICKS_LOG.hh"

void test_energy_lookup_one( const QCK<double>* qck, double BetaInverse, double u  )
{
    double dt ; 
    bool icdf_ = true ; 
    double en = qck->energy_lookup_( BetaInverse, u, dt, icdf_ ); 
    int p = 7 ; 
    int w = p + 10 ; 
    LOG(info)
        << " BetaInverse " << std::fixed << std::setw(w) << std::setprecision(p) << BetaInverse 
        << " u " << std::fixed << std::setw(w) << std::setprecision(p) << u 
        << " en " << std::fixed << std::setw(w) << std::setprecision(p) << en
        << " dt " << std::fixed << std::setw(w) << std::setprecision(p) << dt
        ;
}

void test_energy_lookup_many( const QCK<double>* qck, double BetaInverse, unsigned ni, const char* base, bool icdf_ )
{
    bool biok = qck->is_permissable(BetaInverse); 
    if(biok == false)
    {
        LOG(fatal) 
            << " BetaInverse not permitted as no photons " << std::fixed << std::setw(10) << std::setprecision(4) << BetaInverse
            ; 
        return ; 
    }

    NP* tt = NP::Make<double>( ni ); 
    NP* uu = NP::MakeUniform<double>( ni ) ; 

    NP* en = qck->energy_lookup(BetaInverse, uu, tt, icdf_ ) ; 
    const char* path = SPath::MakePath<double>(base, nullptr, BetaInverse, icdf_ ? "test_energy_lookup_many_icdf.npy" :  "test_energy_lookup_many_s2cn.npy" ); 
    en->save(path);     

    const char* tt_path = SPath::MakePath<double>(base, nullptr, BetaInverse, "test_energy_lookup_many_tt.npy" ); 
    tt->save(tt_path);     
}


void test_energy_sample_one( const QCK<double>* qck, double BetaInverse )
{
    double dt ; 
    unsigned seed = 0u ; 
    SRng<double> rng(seed) ;  

    double emin = 1.55 ;  
    double emax = 15.5 ;  
    unsigned count ; 
    double en = qck->energy_sample_( emin, emax, BetaInverse, rng, dt, count ); 

    int p = 7 ; 
    int w = p + 10 ; 

    LOG(info)
        << " BetaInverse " << std::fixed << std::setw(w) << std::setprecision(p) << BetaInverse 
        << " en " << std::fixed << std::setw(w) << std::setprecision(p) << en
        << " dt " << std::fixed << std::setw(w) << std::setprecision(p) << dt
        << " count " << std::setw(7) << count 
        ;
}

void test_energy_sample_many( const QCK<double>* qck, double BetaInverse, unsigned ni, const char* base )
{
    bool biok = qck->is_permissable(BetaInverse); 
    if(biok == false)
    {
        LOG(fatal) 
            << " BetaInverse not permitted as no photons " << std::fixed << std::setw(10) << std::setprecision(4) << BetaInverse
            ; 
        return ; 
    }

    unsigned seed = 0u ; 
    SRng<double> rng(seed) ;  
    
    NP* tt = NP::Make<double>( ni ); 
    NP* en = qck->energy_sample( BetaInverse, rng, ni, tt ) ; 
    const char* path = SPath::MakePath<double>( base, nullptr, BetaInverse, "test_energy_sample_many.npy" ); 
    en->save(path) ;     

    const char* tt_path = SPath::MakePath<double>( base, nullptr, BetaInverse, "test_energy_sample_many_tt.npy" ); 
    tt->save(tt_path) ;     
}


int main(int argc, char** argv)
{
    OPTICKS_LOG(argc, argv); 

    char d = 'S' ; 
    char t = argc > 1 ? argv[1][0] : d ; 

    const char* reldir = nullptr ; 
    switch(t)
    {
        case 'S': reldir = "test_makeICDF_SplitBin" ; break ; 
        case 'U': reldir = "test_makeICDF_UpperCut" ; break ; 
    } 
    assert( reldir && "require S or U argument to pick ICDF reldir" ); 
    const char* icdf_base = SPath::Resolve("$TMP/QCerenkovIntegralTest", false);
    LOG(info) << " t " << t << " icdf_base " << icdf_base << " reldir " << reldir ; 

    // hmm: save samples into ICDF subdir to explicitly record the vital connection 
 
    QCK<double>* qck = QCK<double>::Load(icdf_base, reldir); 
    if( qck == nullptr ) return 0 ; 

    qck->init(); 
    const char* lfold = qck->lfold.c_str() ; 

    LOG(info) << " qck.lfold " << lfold ; 
    LOG(info) << " qck.bis  " << qck->bis->desc(); 
    LOG(info) << " qck.bis->meta  " << qck->bis->meta; 

    LOG(info) << " qck.s2c  " << qck->s2c->desc(); 
    LOG(info) << " qck.s2cn " << qck->s2cn->desc(); 

    int create_dirs = 2 ;  //2:dirpath
    const char* sample_base = SPath::Resolve(lfold , "QCKTest", create_dirs ) ; 
    LOG(info) << " sample_base " << sample_base ; 

    std::vector<double> vbis ;  
    for( double bi=1.0 ; bi < qck->rmx ; bi+=0.05 ) vbis.push_back(bi); 
    vbis.push_back(1.792);  // extreme peak : some tiny fraction of a photon  
    //bis.push_back(1.45);   // pdomain assert, from going slightly non-monotonic for _UpperCut not _SplitBin

    NP* bis = NP::Make<double>(vbis);

    std::stringstream ss ; 
    ss << "qck.lfold:" << lfold
       << std::endl 
       << "qck.bis.meta:" << qck->bis->meta 
       << std::endl 
       ;
    bis->meta = ss.str();   // metadata about this test, used for annotations from python 
  

    int nbis =  bis->shape[0] ; 
    unsigned num_gen = 1000000 ; 

    LOG(info)
        << " num_gen " << num_gen
        << " nbis " << nbis
        ;


    double emin, emax ; 
    for(int i=0 ; i < nbis ; i++)
    {
        double BetaInverse = bis->get<double>(i) ; 
        //qck->energy_range_s2cn( emin, emax, BetaInverse, true ); 
        qck->energy_range_avph( emin, emax, BetaInverse, true ); 
    }


    for(int i=0 ; i < nbis ; i++)
    {
        double BetaInverse = bis->get<double>(i) ; 
        LOG(info) << " lookup BetaInverse icdf:1 " << std::fixed << std::setw(10) << std::setprecision(4) << BetaInverse ;
        test_energy_lookup_many( qck, BetaInverse, num_gen, sample_base, true ); 
    }

    for(int i=0 ; i < nbis ; i++)
    {
        double BetaInverse = bis->get<double>(i) ; 
        LOG(info) << " lookup BetaInverse icdf:0 " << std::fixed << std::setw(10) << std::setprecision(4) << BetaInverse ;
        test_energy_lookup_many( qck, BetaInverse, num_gen, sample_base, false ); 
    }

    for(int i=0 ; i < nbis ; i++)
    {
        double BetaInverse = bis->get<double>(i) ; 
        LOG(info) << " sampling BetaInverse " << std::fixed << std::setw(10) << std::setprecision(4) << BetaInverse ;
        test_energy_sample_many( qck, BetaInverse, num_gen, sample_base ); 
    }

    bis->save(sample_base, "bis.npy" );  

    return 0 ; 
}

