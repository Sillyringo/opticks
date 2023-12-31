/**
X4IntersectVolumeTest
========================

Used from script extg4/xxv.sh 

The call to PMTSim::GetPV populates vectors via pointer to vector arguments 
containing the solids and their transforms, relative to the top level PV presumably. 
(Could be just simple one level). These transforms are saved.  

Subsequently the scanning is applied to each solid individually.

The python level plotting then brings together the solid frame intersects
into the volume frame by applying the saved transforms for each solid.  
 

**/

#include <cstdlib>
#include "OPTICKS_LOG.hh"
#include "SSys.hh"
#include "SStr.hh"
#include "SPath.hh"
#include "SVolume.h"

//#include "SIntersect.h"
#include "SSimtrace.h"

#include "G4VSolid.hh"

#ifdef WITH_PMTFASTSIM
#include "PMTFastSim.hh"
#elif WITH_PMTSIM
#include "PMTSim.hh"
#endif




int main(int argc, char** argv)
{
    OPTICKS_LOG(argc, argv); 

    //const char* geom_default = "nnvtBodyPhys" ; 
    const char* geom_default = "hamaBodyPhys" ; 
    const char* geom = SSys::getenvvar("X4IntersectVolumeTest_GEOM", geom_default );  
    int rc = 0 ; 

#if defined(WITH_PMTFASTSIM) || defined(WITH_PMTSIM)

    typedef std::vector<double> VD ;  // curious why not use NP ?
    typedef std::vector<G4VSolid*> VS ; 
    VD* tr = new VD ; 
    VS* so = new VS ; 

    /**
    NB the transform collection within PMTSim, PMTFastSim is 
    provided by sysrap/SVolume.h which is currently 
    limited to a single level of transforms.
 
    Aiming to lift the restriction using stree.h based approach, 
    under development in u4/tests/U4PMTFastSimGeomTest.cc
    **/

#ifdef WITH_PMTFASTSIM
    LOG(info) << "[ PMTFastSim::GetPV geom [" << geom << "]" ; 
    G4VPhysicalVolume* pv = PMTFastSim::GetPV(geom, tr, so );
    LOG(info) << "] PMTFastSim::GetPV geom [" << geom << "]" ; 
#elif WITH_PMTSIM
    LOG(info) << "[ PMTSim::GetPV geom [" << geom << "]" ; 
    G4VPhysicalVolume* pv = PMTSim::GetPV(geom, tr, so );
    LOG(info) << "] PMTSim::GetPV geom [" << geom << "]" ; 
#endif
    assert(pv); 

    unsigned num = so->size(); 
    assert( tr->size() % 16 == 0 ); 
    assert( tr->size() == 16*num );  // expect 16 doubles of the transform matrix for every solid

    const char* base = SPath::Resolve("$FOLD", DIRPATH ) ; 

    SVolume::DumpTransforms(tr, so, "X4IntersectVolumeTest.DumpTransforms"); 
    SVolume::SaveTransforms(tr, so, base, "transforms.npy" ); 


    LOG(info) << " num " << num ; 
    for(unsigned i=0 ; i < num ; i++)
    {
        G4VSolid* solid = (*so)[i] ; 
        G4String soname_ = solid->GetName();  // careful, by value 
        const char* soname = soname_.c_str() ; 

        LOG(info) << "[ X4Intersect::Scan soname [" << soname << "]" ; 
        //SIntersect::Scan(solid, soname, base ); 
     
        SSimtrace::Scan(solid, base) ;   

        LOG(info) << "] X4Intersect::Scan soname [" << soname << "]" ; 
    }
#else
    LOG(fatal) << " not-WITH_PMTSIM OR WITH_PMTFASTSIM : cannot do anything without the volume " << geom ; 
    rc=1 ;   
#endif
    return rc ; 
}

