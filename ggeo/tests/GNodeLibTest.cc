#include "OPTICKS_LOG.hh"
#include "NGLM.hpp"
#include "NBBox.hpp"
#include "Opticks.hh"
#include "OpticksIdentity.hh"
#include "GNodeLib.hh"

/**
GNodeLibTest
=============

See also ana/GNodeLib.py 

**/

void test_dump(const GNodeLib* nlib)
{
    LOG(info) << "nlib " << nlib ; 
    nlib->Dump("GNodeLibTest"); 

    std::map<std::string, int> targets ;  
    nlib->dumpVolumes(targets); 
}

void test_transforms(const GNodeLib* nlib)
{
    unsigned num_transforms = nlib->getNumTransforms(); 
    LOG(info) << " num_transforms " << num_transforms ; 

    glm::mat4 tr0 = nlib->getTransform(0); 
    LOG(info) << " tr(0) " << glm::to_string(tr0) ;  

    glm::mat4 tr1 = nlib->getTransform(num_transforms-1); 
    LOG(info) << " tr(N-1) " << glm::to_string(tr1) ;  
}

void test_findContainerVolumeIndex(const GNodeLib* nlib, unsigned pick_vol)
{
    LOG(info) ; 
    glm::vec4 pick_ce = nlib->getCE(pick_vol);  // pick a point from the center of the mid-volume 

    glm::vec3 p(pick_ce); 

    unsigned find_vol = nlib->findContainerVolumeIndex(p.x, p.y, p.z);  
    std::cout 
        << " pick_vol : descVolume " << nlib->descVolume(pick_vol)
        << " find_vol : descVolume " << nlib->descVolume(find_vol) 
        ; 

    glm::vec4 find_ce = nlib->getCE(find_vol);  
    glm::vec3 delta = glm::vec3(find_ce - pick_ce) ; 

    std::cout 
        << " delta "
        << glm::to_string(delta)
        << std::endl 
        ;

    nbbox find_bb = nlib->getBBox(pick_vol); 
    assert( find_bb.contains(p) ); 

}

void test_getBB(const GNodeLib* nlib, unsigned pick_vol)
{
    LOG(info); 
    glm::vec4 mn ; 
    glm::vec4 mx ; 
    nlib->getBB(pick_vol, mn, mx ); 

    std::cout 
        << " pick_vol " << pick_vol
        << " mn " << glm::to_string(mn)
        << " mx " << glm::to_string(mx)
        << std::endl 
        ;
}

void test_getBBox(const GNodeLib* nlib, unsigned pick_vol)
{
    LOG(info); 
    nbbox bb = nlib->getBBox(pick_vol); 
    std::cout 
        << " pick_vol " << pick_vol
        << " bb " << bb.desc()
        << std::endl 
        ;
}

void test_getIdentity(const GNodeLib* nlib, unsigned nidx0, unsigned nidx1)
{
    LOG(info); 
    for(unsigned nidx=nidx0 ; nidx < nidx1 ; nidx++)
    {
        glm::uvec4 nid = nlib->getIdentity(nidx); 
        std::cout 
            << " nidx " << nidx 
            << OpticksIdentity::Desc(" nid", nid)
            << std::endl
            ;

        assert( nid.x == nidx ); 
    }
 
}

void test_dumpSensorVolumes(const GNodeLib* nlib)
{
    nlib->dumpSensorVolumes();
}

void test_getNodeIndicesForLVName(const GNodeLib* nlib) 
{
    const Opticks* ok = nlib->getOpticks(); 
    const char* lvname = ok->getGDMLAuxTargetLVName() ;
    if( lvname == NULL ) return ;  
    std::vector<unsigned> nidxs ; 
    nlib->getNodeIndicesForLVName(nidxs, lvname); 
    LOG(info) << " lvname " << lvname ;  
    nlib->dumpNodes(nidxs, "test_getNodeIndicesForLVName" ); 
}

void test_getNodeIndicesForPVName(const GNodeLib* nlib) 
{
    const Opticks* ok = nlib->getOpticks(); 
    const char* pvname = ok->getPVName(); 
    if( pvname == NULL ) return ;  
    std::vector<unsigned> nidxs ; 
    nlib->getNodeIndicesForPVName(nidxs, pvname); 
    LOG(info) << " pvname " << pvname ;  
    nlib->dumpNodes(nidxs, "test_getNodeIndicesForPVName" ); 
}


void test_getNodeIndicesForPVNameStarting(const GNodeLib* nlib) 
{
    const Opticks* ok = nlib->getOpticks(); 
    const char* targetpvn = ok->getTargetPVN(); 
    LOG(info) << " targetpvn " << targetpvn ; 
    if( targetpvn == NULL ) return ;  
    std::vector<unsigned> nidxs ; 
    nlib->getNodeIndicesForPVNameStarting(nidxs, targetpvn ); 
    LOG(info) 
       << " targetpvn " << targetpvn 
       << " nidxs.size " << nidxs.size() 
       ;  

    nlib->dumpNodes(nidxs, "test_getNodeIndicesForPVNameStarting" ); 
}


int main(int argc, char** argv)
{
    OPTICKS_LOG(argc, argv); 
   
    Opticks ok(argc, argv);
    ok.configure();


 
    GNodeLib* nlib = GNodeLib::Load(&ok); 
    assert(nlib);  
    
    //test_transforms(nlib); 
    //test_dump(nlib);


    /*
    unsigned num_volumes = nlib->getNumVolumes(); 
    unsigned  pick_vol = num_volumes/2 ;    // mid-volume  
    test_findContainerVolumeIndex(nlib, pick_vol); 
    test_getBB(nlib, pick_vol); 
    test_getBBox(nlib, pick_vol); 


    unsigned nidx0 = 3199 ; 
    unsigned nidx1 = nidx0+20  ; 
    test_getIdentity(nlib, nidx0, nidx1 ); 

    */

    //test_dumpSensorVolumes(nlib); 

    //test_getNodeIndicesForLVName(nlib) ;
    //test_getNodeIndicesForPVName(nlib) ;
    test_getNodeIndicesForPVNameStarting(nlib) ;

    return 0 ; 
}

// om-;TEST=GNodeLibTest om-t

