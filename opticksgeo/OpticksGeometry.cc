
// npy-
#include "Timer.hpp"
#include "NGLM.hpp"
#include "GLMFormat.hpp"
#include "GLMPrint.hpp"
#include "NSlice.hpp"

// okc-
#include "Opticks.hh"
#include "Composition.hh"
#include "OpticksConst.hh"
#include "OpticksResource.hh"
#include "OpticksAttrSeq.hh"
#include "OpticksCfg.hh"

// okg-
#include "OpticksHub.hh"

// ggeo-
#include "GGeoLib.hh"
#include "GBndLib.hh"
#include "GMaterialLib.hh"
#include "GSurfaceLib.hh"
#include "GPmt.hh"
#include "GParts.hh"
#include "GMergedMesh.hh"
#include "GNodeLib.hh"
#include "GGeo.hh"

// assimpwrap
#include "AssimpGGeo.hh"

// openmeshrap-
#include "MFixer.hh"
#include "MTool.hh"


// opticksgeo-
#include "OpticksGeometry.hh"


#include "PLOG.hh"



// TODO: move to OK_PROFILE 
#define TIMER(s) \
    { \
       if(m_ok)\
       {\
          Timer& t = *(m_ok->getTimer()) ;\
          t((s)) ;\
       }\
    }


OpticksGeometry::OpticksGeometry(OpticksHub* hub)
   :
   m_hub(hub),
   m_ok(m_hub->getOpticks()),
   m_composition(m_hub->getComposition()),
   m_fcfg(m_ok->getCfg()),
   m_ggeo(NULL),

   m_verbosity(m_ok->getVerbosity())
{
    init();
}

GGeo* OpticksGeometry::getGGeo()
{
   return m_ggeo ; 
}


void OpticksGeometry::init()
{
    bool geocache = !m_fcfg->hasOpt("nogeocache") ;
    bool instanced = !m_fcfg->hasOpt("noinstanced") ; // find repeated geometry 

    LOG(debug) << "OpticksGeometry::init"
              << " geocache " << geocache 
              << " instanced " << instanced
              ;

    m_ok->setGeocache(geocache);
    m_ok->setInstanced(instanced); // find repeated geometry 

    m_ggeo = new GGeo(m_ok);
    m_ggeo->setLookup(m_hub->getLookup());
}






void OpticksGeometry::loadGeometry()
{

    LOG(info) << "OpticksGeometry::loadGeometry START "  ; 

    loadGeometryBase(); //  usually from cache

    if(!m_ggeo->isValid())
    {
        LOG(warning) << "OpticksGeometry::loadGeometry finds invalid geometry, try creating geocache with --nogeocache/-G option " ; 
        m_ok->setExit(true); 
        return ; 
    }


    // modifyGeometry moved up to OpticksHub

    fixGeometry();

    //registerGeometry moved up to OpticksHub

    if(!m_ok->isGeocache())
    {
        LOG(info) << "OpticksGeometry::loadGeometry early exit due to --nogeocache/-G option " ; 
        m_ok->setExit(true); 
    }


    LOG(info) << "OpticksGeometry::loadGeometry DONE " ; 
    TIMER("loadGeometry");
}


void OpticksGeometry::loadGeometryBase()
{
    LOG(error) << "OpticksGeometry::loadGeometryBase START " ; 
    OpticksResource* resource = m_ok->getResource();

    if(m_ok->hasOpt("qe1"))
        m_ggeo->getSurfaceLib()->setFakeEfficiency(1.0);


    m_ggeo->setLoaderImp(&AssimpGGeo::load);    // setting GLoaderImpFunctionPtr


    m_ggeo->setMeshJoinImp(&MTool::joinSplitUnion);
    m_ggeo->setMeshVerbosity(m_fcfg->getMeshVerbosity());    
    m_ggeo->setMeshJoinCfg( resource->getMeshfix() );

    std::string meshversion = m_fcfg->getMeshVersion() ;;
    if(!meshversion.empty())
    {
        LOG(warning) << "OpticksGeometry::loadGeometry using debug meshversion " << meshversion ;  
        m_ggeo->getGeoLib()->setMeshVersion(meshversion.c_str());
    }

    m_ggeo->loadGeometry();   // potentially from cache : for gltf > 0 loads both tri and ana geometry 
        
    if(m_ggeo->getMeshVerbosity() > 2)
    {
        GMergedMesh* mesh1 = m_ggeo->getMergedMesh(1);
        if(mesh1)
        {
            mesh1->dumpSolids("OpticksGeometry::loadGeometryBase mesh1");
            mesh1->save("$TMP", "GMergedMesh", "baseGeometry") ;
        }
    }

    LOG(error) << "OpticksGeometry::loadGeometryBase DONE " ; 
    TIMER("loadGeometryBase");
}


void OpticksGeometry::fixGeometry()
{
    if(m_ggeo->isLoaded())
    {
        LOG(debug) << "OpticksGeometry::fixGeometry needs to be done precache " ;
        return ; 
    }
    LOG(info) << "OpticksGeometry::fixGeometry" ; 

    MFixer* fixer = new MFixer(m_ggeo);
    fixer->setVerbose(m_ok->hasOpt("meshfixdbg"));
    fixer->fixMesh();
 
    bool zexplode = m_ok->hasOpt("zexplode");
    if(zexplode)
    {
       // for --jdyb --idyb --kdyb testing : making the cleave OR the mend obvious
        glm::vec4 zexplodeconfig = gvec4(m_fcfg->getZExplodeConfig());
        print(zexplodeconfig, "zexplodeconfig");

        GMergedMesh* mesh0 = m_ggeo->getMergedMesh(0);
        mesh0->explodeZVertices(zexplodeconfig.y, zexplodeconfig.x ); 
    }
    TIMER("fixGeometry"); 
}




