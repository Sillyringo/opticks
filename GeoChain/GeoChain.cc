#include "G4VSolid.hh"

#include "GeoChain.hh"

#include <cstring>

#include "SGeo.hh"
#include "SSys.hh"
#include "SSim.hh"
#include "SPath.hh"

#include "NNode.hpp"
#include "NCSG.hpp"
#include "NP.hh"

#include "GMesh.hh"
#include "GGeo.hh"
#include "X4PhysicalVolume.hh"
#include "X4SolidTree.hh"

#include "CSGFoundry.h"
#include "CSGGeometry.h"
#include "CSG_GGeo_Convert.h"
#include "SLOG.hh"

const plog::Severity GeoChain::LEVEL = SLOG::EnvLevel("GeoChain", "DEBUG") ; 

GeoChain::GeoChain(Opticks* ok_)
    :
    ok(ok_), 
    ggeo(new GGeo(ok, true)),  // live=true to initLibs and not load from cache
    mesh(nullptr),
    volume(nullptr),
    sim(SSim::Create()),
    fd(new CSGFoundry),
    vv(nullptr),
    lvIdx(0),  
    soIdx(0)  
{
    LOG(LEVEL); 
}

/**
GeoChain::convertSolid
-----------------------

Geometry conversions from 1->2->3

1. G4VSolid
2. GMesh 
3. CSGFoundry 

The meta param is for example from X4SolidMaker
which then gets passed into CSGFoundry meta.

**/

void GeoChain::convertSolid(const G4VSolid* solid, std::string* meta )
{
    LOG(LEVEL) << "[" ;  
    LOG(LEVEL) << " meta " << std::endl << ( meta && !meta->empty() ? *meta : " no-meta"  )  ;  
    LOG(LEVEL) << "vv.descValues" << std::endl << ( vv ? vv->descValues() : "no-values" ) ; 

    LOG(LEVEL) << "X4SolidTree::Desc : original G4VSolid tree" << std::endl << X4SolidTree::Desc(solid) ; 

    G4String solidname = solid->GetName(); 
    const char* soname = strdup(solidname.c_str()); 
    const char* lvname = strdup(solidname.c_str()); 

    mesh = X4PhysicalVolume::ConvertSolid(ok, lvIdx, soIdx, solid, soname, lvname ) ; 
    LOG(LEVEL) << " mesh " << mesh ; 
    convertMesh(mesh); 
    
    LOG(LEVEL) << "CSGGeometry::Desc : converted CSGNode tree" << std::endl << CSGGeometry::Desc(fd) ;  

    if(meta && !meta->empty())  // pass metadata from the solid creation into the CSGFoundry meta.txt
    {
        fd->meta = strdup(meta->c_str());  
    }


    LOG(LEVEL) << "]" ;  
}


/**
GeoChain::convertNodeTree
---------------------------

TODO: "const nnode* nd" argument prevented by NCSG::Adopt,  
       maybe need to clone the node tree to support const argument  ?

DONE : factorised X4PhysicalVolume::ConvertSolid_ in order to enable
       kicking off the geochain with nnode and GMesh placeholder 
       without adding much code

**/

void GeoChain::convertNodeTree(nnode* raw)
{
    LOG(LEVEL) << "[" ;  
    const G4VSolid* const solid = nullptr ; 
    bool balance_deep_tree = false ;  
    const char* soname = "convertNodeTree" ; 
    const char* lvname = "convertNodeTree" ; 

    mesh = X4PhysicalVolume::ConvertSolid_FromRawNode(ok, lvIdx, soIdx, solid, soname, lvname, balance_deep_tree, raw ); 

    convertMesh(mesh); 
    LOG(LEVEL) << "]" ;  
}

void GeoChain::convertMesh(GMesh* mesh)
{
    LOG(LEVEL) << "[" ;  
    ggeo->add(mesh); 

    // standin for X4PhysicalVolume::convertStructure
    volume = X4PhysicalVolume::MakePlaceholderNode(); 
    ggeo->setRootVolume(volume);

    ggeo->prepareVolumes();   // creates GMergedMesh 

    ggeo->deferredCreateGParts(); 

    CSG_GGeo_Convert conv(fd, ggeo) ; 
    conv.convert();

    fd->addTranPlaceholder(); 
    LOG(LEVEL) << "]" ;  
}


/**
GeoChain::convertName
-----------------------

TODO: integrate with DemoGeo so can handle geometry created at CSG level just like geometry created at G4VSolid level 

Doing something similar in CSG/tests/CSGMakerTest.cc

**/

void GeoChain::convertName(const char* name)
{
    LOG(fatal) << "NOTE THAT SOME GEOMETRY IS IMPLEMENTED ONLY AT CSG LEVEL : see CSG/CSGMakerTest.sh " ; 
    assert(0); 
}




/**
GeoChain::convertPV
---------------------

see okg4/tests/OKX4Test.cc as its doing much the same thing, maybe 
avoid duplication with some static methods 

**/

void GeoChain::convertPV( const G4VPhysicalVolume* top )
{
    LOG(LEVEL) << "[" ;  
    X4PhysicalVolume xtop(ggeo, top) ; 

    // ggeo->postDirectTranslation();  tries to save which fails with no idpath 
    // ggeo->prepareVolumes();   // just prepareVolumes  misses prepareOpticks which prevcents --skipsolidname from working 
    ggeo->prepare(); 

    ggeo->deferredCreateGParts();

    CSG_GGeo_Convert conv(fd, ggeo) ;  

    conv.convert();   // populates fd:CSGFoundry 

    std::cout 
        << "[ fd.descPrimSpec " << std::endl 
        << fd->descPrimSpec()
        << "] fd.descPrimSpec " << std::endl 
        << "[ fd.descMeshName " << std::endl 
        << fd->descMeshName()
        << "] fd.descMeshName " << std::endl 
        ; 


    LOG(LEVEL) << "]" ;  
}



const char* GeoChain::getCFBase(const char* name, const char* base_) const
{
    const char* base = base_ ? base_ : "$DefaultGeometryBase" ;  
    const char* fold = SPath::Resolve(base, name, DIRPATH );   
    const char* cfbase = SSys::getenvvar("CFBASE", fold  );
    return cfbase ; 
}

void GeoChain::save(const char* name, const char* base_) const 
{
    const char* cfbase = getCFBase(name, base_); 

    LOG(info) << " name " << name << " cfbase " << cfbase ; 

    fd->save(cfbase, CSGFoundry::RELDIR );    // expects existing directory $CFBASE/CSGFoundry 
    checkSaveWithLoad(cfbase); 

    if(vv)
    {
        vv->save(cfbase, "Values", "values.npy");  
    }
}

void GeoChain::checkSaveWithLoad(const char* cfbase) const 
{
    CSGFoundry* lfd = CSGFoundry::Load(cfbase, CSGFoundry::RELDIR );  // load foundary and check identical bytes
    int rc = CSGFoundry::Compare(fd, lfd ) ; 
    if(rc)
    {
        LOG(fatal) << " CSGFoundry::Compare(fd, lfd )  failure " << rc ;   
        LOG(error) << CSGFoundry::DescCompare(fd, lfd ) ; 
    }
    assert( 0 == rc );  
}





