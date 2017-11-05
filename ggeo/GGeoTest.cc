#include <iomanip>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include "BStr.hh"

// npy-
#include "NSlice.hpp"
#include "NCSG.hpp"
#include "NCSGList.hpp"
#include "GLMFormat.hpp"
#include "NGLMExt.hpp"
#include "NLODConfig.hpp"

// okc-
#include "Opticks.hh"
#include "OpticksResource.hh"
#include "OpticksEventAna.hh"
#include "OpticksConst.hh"


#include "GVector.hh"
#include "GGeoBase.hh"
#include "GGeoLib.hh"
#include "GBndLib.hh"
#include "GPmtLib.hh"
#include "GMergedMesh.hh"
#include "GPmt.hh"
#include "GSolid.hh"

#include "GNodeLib.hh"
#include "GSolidList.hh"

#include "GMaker.hh"
#include "GItemList.hh"
#include "GParts.hh"
#include "GTransforms.hh"
#include "GIds.hh"

#include "GGeoTestConfig.hh"
#include "GGeoTest.hh"

#include "PLOG.hh"



GSolidList*     GGeoTest::getSolidList(){       return m_solist ; }  // <-- TODO: elim, use GNodeLib

NCSGList*       GGeoTest::getCSGList() const  { return m_csglist ;  }
GGeoTestConfig* GGeoTest::getConfig() {         return m_config ; }
NCSG* GGeoTest::findEmitter() const  {          return m_csglist ? m_csglist->findEmitter() : NULL ; }


// pass along from basis
GScintillatorLib* GGeoTest::getScintillatorLib(){ return m_basis->getScintillatorLib() ; }
GSourceLib*       GGeoTest::getSourceLib(){       return m_basis->getSourceLib() ; }

// local copy of m_basis pointer
GPmtLib*          GGeoTest::getPmtLib(){          return m_pmtlib ; }
GBndLib*          GGeoTest::getBndLib(){          return m_bndlib ;  }

// locally customized 
const char*       GGeoTest::getIdentifier(){               return "GGeoTest" ; }
GMergedMesh*      GGeoTest::getMergedMesh(unsigned index){ return m_geolib->getMergedMesh(index) ; }
GGeoLib*          GGeoTest::getGeoLib(){                   return m_geolib ; }
GNodeLib*         GGeoTest::getNodeLib(){                  return m_nodelib ; }



GGeoTest::GGeoTest(Opticks* ok, GGeoBase* basis) 
    : 
    m_ok(ok),
    m_config(new GGeoTestConfig(ok->getTestConfig())),
    m_resource(ok->getResource()),
    m_dbganalytic(m_ok->isDbgAnalytic()),
    m_lodconfig(ok->getLODConfig()),
    m_lod(ok->getLOD()),
    m_analytic(m_config->getAnalytic()),
    m_test(true),
    m_basis(basis),
    m_bndlib(basis->getBndLib()),
    m_pmtlib(basis->getPmtLib()),
    m_geolib(new GGeoLib(m_ok,m_analytic,m_bndlib)),
    m_nodelib(new GNodeLib(m_ok, m_analytic, m_test)),
    m_maker(new GMaker(m_ok, m_bndlib)),
    m_csglist(NULL),
    m_solist(NULL),
    m_verbosity(0)
{
    init();
}


void GGeoTest::dump(const char* msg)
{
    LOG(info) << msg  ; 
}


void GGeoTest::init()
{
    assert(m_basis); 

    const char* csgpath = m_config->getCsgPath();
    if(csgpath) assert(m_analytic == true);

    GMergedMesh* tmm_ = create();

    GMergedMesh* tmm = m_lod > 0 ? GMergedMesh::MakeLODComposite(tmm_, m_lodconfig->levels ) : tmm_ ;         

    char geocode =  m_analytic ? OpticksConst::GEOCODE_ANALYTIC : OpticksConst::GEOCODE_TRIANGULATED ;  // message to OGeo

    tmm->setGeoCode( geocode );

    if(tmm->isTriangulated()) 
    { 
        tmm->setITransformsBuffer(NULL); // avoiding FaceRepeated complications 
    } 

    m_geolib->setMergedMesh( 0, tmm );  // TODO: create via standard GGeoLib::create ?

    // tmm->save("$TMP", "GMergedMesh", "GGeoTest_init") ;


}


GMergedMesh* GGeoTest::create()
{
    const char* csgpath = m_config->getCsgPath();
    const char* mode = m_config->getMode();
    unsigned nelem = m_config->getNumElements();

    assert( mode );

    if(csgpath == NULL)
    { 
        if(nelem == 0 )
        {
            LOG(fatal) << " NULL csgpath and config nelem zero  " ; 
            m_config->dump("GGeoTest::create ERROR csgpath==NULL && nelem==0 " ); 
        }
        assert(nelem > 0);
    }


    LOG(info) << "GGeoTest::create START " << " mode " << mode ;

    GMergedMesh* tmm = NULL ; 
    //std::vector<GSolid*> solids ; 

    m_solist = new GSolidList() ; 
    std::vector<GSolid*>& solids = m_solist->getList();

    if(m_config->isNCSG())
    {
        assert( csgpath && strlen(csgpath) > 3 && "unreasonable csgpath strlen");  
        loadCSG(csgpath, solids);
        assert( m_csglist );

        tmm = combineSolids(solids, NULL);

        m_resource->setTestCSGPath(csgpath); // take note of path, for inclusion in event metadata
    }
    else if(m_config->isBoxInBox()) 
    {
        createBoxInBox(solids); 
        labelPartList(solids) ;
        tmm = combineSolids(solids, NULL);
    }
    else if(m_config->isPmtInBox())
    {
        tmm = createPmtInBox(); 
    }
    else 
    { 
        LOG(fatal) << "GGeoTest::create mode not recognized [" << mode << "]" ; 
        assert(0);
    }

    assert(tmm);
    LOG(info) << "GGeoTest::create DONE " << " mode " << mode ;
    return tmm ; 
}




NCSG* GGeoTest::getTree(unsigned index) const 
{
    return m_csglist->getTree(index);
}
unsigned GGeoTest::getNumTrees() const 
{
    return m_csglist->getNumTrees();
}


void GGeoTest::anaEvent(OpticksEvent* evt)
{
    int dbgnode = m_ok->getDbgNode();
    //NCSG* csg = getTree(dbgnode);

    LOG(info) << "GGeoTest::anaEvent " 
              << " dbgnode " << dbgnode
              << " numTrees " << getNumTrees()
              << " evt " << evt
              ;

    assert( m_csglist ) ;  

    OpticksEventAna ana(m_ok, evt, m_csglist);
    ana.dump("GGeoTest::anaEvent");
}




void GGeoTest::loadCSG(const char* csgpath, std::vector<GSolid*>& solids)
{
    int verbosity = m_config->getVerbosity();

    assert( m_csglist == NULL );

    m_csglist = NCSGList::Load(csgpath, verbosity );
    assert( m_csglist );

    unsigned ntree = m_csglist->getNumTrees() ;
   
    LOG(info) << "GGeoTest::loadCSG START " 
             << " csgpath " << csgpath 
             << " ntree " << ntree 
             << " verbosity " << verbosity
             ; 

    int primIdx(-1) ; 


    // assuming tree order from outermost to innermost volume 
    GSolid* prior = NULL ; 

    for(unsigned i=0 ; i < ntree ; i++)
    {
        primIdx++ ; // each tree is separate OptiX primitive, with own line in the primBuffer 

        NCSG* tree = m_csglist->getTree(i) ; 
        GSolid* solid = m_maker->makeFromCSG(tree, verbosity );

        if(prior)
        {
            solid->setParent(prior);
            prior->addChild(solid);
        }
        prior = solid ; 


        GParts* pts = solid->getParts();
        pts->setIndex(0u, i);
        if(pts->isPartList())  // not doing this for NodeTree
        {
            pts->setNodeIndexAll(primIdx ); 
        }
        pts->setBndLib(m_bndlib);


        solids.push_back(solid);  // <-- TODO: eliminate 
        m_nodelib->add(solid);
    }


    LOG(info) << "GGeoTest::loadCSG DONE " ; 

}

void GGeoTest::labelPartList( std::vector<GSolid*>& solids )
{
    // PartList geometry (the precursor to proper CSG Trees, usually defined in python CSG) 
    // is implemented by allowing a single "primitive" to be composed of multiple
    // "parts", the association from part to prim being 
    // controlled via the primIdx attribute of each part.
    //
    // collected pts are converted into primitives in GParts::makePrimBuffer
  
    for(unsigned i=0 ; i < solids.size() ; i++)
    {
        GSolid* solid = solids[i];
        GParts* pts = solid->getParts();
        assert(pts);
        assert(pts->isPartList());

        OpticksCSG_t csgflag = solid->getCSGFlag(); 
        int flags = csgflag ;

        pts->setIndex(0u, i);
        pts->setNodeIndex(0u, 0 );  
        //
        // for CSG_FLAGPARTLIST the nodeIndex is crucially used to associate parts to their prim 
        // setting all to zero is structuring all parts into a single prim ... 
        // can get away with that for BoxInBox (for now)
        // but would definitely not work for PmtInBox 
        //

        pts->setTypeCode(0u, flags);

        pts->setBndLib(m_bndlib);

        LOG(info) << "GGeoTest::labelPartList"
                  << " i " << std::setw(3) << i 
                  << " csgflag " << std::setw(5) << csgflag 
                  << std::setw(20) << CSGName(csgflag)
                  << " pts " << pts 
                  ;
    }
}

GSolid* GGeoTest::makeSolidFromConfig( unsigned i ) // setup nodeIndex here ?
{
    std::string node = m_config->getNodeString(i);
    OpticksCSG_t type = m_config->getTypeCode(i);

    const char* spec = m_config->getBoundary(i);
    glm::vec4 param = m_config->getParameters(i);
    glm::mat4 trans = m_config->getTransform(i);
    unsigned boundary = m_bndlib->addBoundary(spec);

    LOG(info) << "GGeoTest::makeSolidFromConfig" 
              << " i " << std::setw(2) << i 
              << " node " << std::setw(20) << node
              << " type " << std::setw(2) << type 
              << " csgName " << std::setw(15) << CSGName(type)
              << " spec " << spec
              << " boundary " << boundary
              << " param " << gformat(param)
              << " trans " << gformat(trans)
              ;

    bool oktype = type < CSG_UNDEFINED ;  
    if(!oktype) LOG(fatal) << "GGeoTest::makeSolidFromConfig configured node not implemented " << node ;
    assert(oktype);

    GSolid* solid = m_maker->make(i, type, param, spec );   
    GParts* pts = solid->getParts();
    assert(pts);
    pts->setPartList(); // setting primFlag to CSG_FLAGPARTLIST
    pts->setBndLib(m_bndlib) ; 

    return solid ; 
}

void GGeoTest::createBoxInBox(std::vector<GSolid*>& solids)
{
    unsigned nelem = m_config->getNumElements();
    for(unsigned i=0 ; i < nelem ; i++)
    {
        GSolid* solid = makeSolidFromConfig(i);
        solids.push_back(solid);  // <-- TODO: eliminate

        m_nodelib->add(solid);
    }
}


GMergedMesh* GGeoTest::createPmtInBox()
{
    assert( m_config->getNumElements() == 1 && "GGeoTest::createPmtInBox expecting single container " );

    GSolid* container = makeSolidFromConfig(0); 
    const char* spec = m_config->getBoundary(0);
    const char* container_inner_material = m_bndlib->getInnerMaterialName(spec);
    const char* medium = m_ok->getAnalyticPMTMedium();
    assert( strcmp( container_inner_material, medium ) == 0 );

    int verbosity = m_config->getVerbosity();

    //GMergedMesh* mmpmt = loadPmtDirty();
    GMergedMesh* mmpmt = m_pmtlib->getPmt() ;
    assert(mmpmt);

    unsigned pmtNumSolids = mmpmt->getNumSolids() ; 
    container->setIndex( pmtNumSolids );   // <-- HMM: MAYBE THIS SHOULD FEED INTO GParts::setNodeIndex ?

    LOG(info) << "GGeoTest::createPmtInBox " 
              << " spec " << spec 
              << " container_inner_material " << container_inner_material
              << " pmtNumSolids " << pmtNumSolids
              ; 


    GMesh* mesh = const_cast<GMesh*>(container->getMesh()); // TODO: reorg to avoid 
    mesh->setIndex(1000);
    
    GParts* cpts = container->getParts() ;

    cpts->setPrimFlag(CSG_FLAGPARTLIST);  // PmtInBox uses old partlist, not the default CSG_FLAGNODETREE
    cpts->setAnalyticVersion(mmpmt->getParts()->getAnalyticVersion()); // follow the PMT version for the box
    cpts->setNodeIndex(0, pmtNumSolids);   // NodeIndex used to associate parts to their prim, fixed 5-4-2-1-1 issue yielding 4-4-2-1-1-1


    GMergedMesh* triangulated = GMergedMesh::combine( mmpmt->getIndex(), mmpmt, container, verbosity );   

    // hmm this is putting the container at the end... does that matter ?

    //if(verbosity > 1)
    triangulated->dumpSolids("GGeoTest::createPmtInBox GMergedMesh::dumpSolids combined (triangulated) ");

    // needed by OGeo::makeAnalyticGeometry
    NPY<unsigned int>* idBuf = mmpmt->getAnalyticInstancedIdentityBuffer();
    NPY<float>* itransforms = mmpmt->getITransformsBuffer();

    assert(idBuf);
    assert(itransforms);

    triangulated->setAnalyticInstancedIdentityBuffer(idBuf);
    triangulated->setITransformsBuffer(itransforms);

    return triangulated ; 
}

GMergedMesh* GGeoTest::combineSolids(std::vector<GSolid*>& solids, GMergedMesh* mm0)
{
    // TODO: eliminate, instead use GNodeLib::createMergeMesh 

    LOG(info) << "GGeoTest::combineSolids START " ; 

    unsigned verbosity = 3 ; 
    GMergedMesh* tri = GMergedMesh::combine( 0, mm0, solids, verbosity );

    unsigned nelem = solids.size() ; 
    GTransforms* txf = GTransforms::make(nelem); // identities
    GIds*        aii = GIds::make(nelem);        // placeholder (n,4) of zeros

    tri->setAnalyticInstancedIdentityBuffer(aii->getBuffer());  
    tri->setITransformsBuffer(txf->getBuffer());

    GParts* pts0 = solids[0]->getParts();
    GParts* pts = tri->getParts();

    if(pts0->isPartList())
    {
        pts->setPartList();  // not too late, needed only for primBuffer creation which happens last 
    } 

    //  OGeo::makeAnalyticGeometry  requires AII and IT buffers to have same item counts

    if(m_dbganalytic)
    {
        GParts* pts = tri->getParts();
        pts->setName(m_config->getName());
        const char* msg = "GGeoTest::combineSolids --dbganalytic" ;
        pts->Summary(msg);
        pts->dumpPrimInfo(msg); // this usually dumps nothing as solid buffer not yet created
    }
    // collected pts are converted into primitives in GParts::makePrimBuffer


    LOG(info) << "GGeoTest::combineSolids DONE " ; 

    return tri ; 
}


