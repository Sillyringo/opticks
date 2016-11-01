#include "CFG4_BODY.hh"
// cfg4-

//
//  ggv-;ggv-pmt-test --cfg4
//  ggv-;ggv-pmt-test --cfg4 --load 1
//

#include <map>

// okc-
#include "OpticksHub.hh"
#include "Opticks.hh"

// npy-
#include "NGLM.hpp"
#include "GLMFormat.hpp"

// ggeo-
#include "GMaker.hh"
#include "GCSG.hh"
#include "GMaterial.hh"
#include "GGeoTestConfig.hh"

// g4-
#include "CFG4_PUSH.hh"

#include "G4RunManager.hh"
#include "G4NistManager.hh"
#include "G4MaterialTable.hh"
#include "G4Material.hh"
#include "G4LogicalVolume.hh"
#include "G4ThreeVector.hh"
#include "G4PVPlacement.hh"
#include "G4UImanager.hh"

#include "globals.hh"
#include "G4PhysicalConstants.hh"
#include "G4SystemOfUnits.hh"

#include "CFG4_POP.hh"



// cfg4-
#include "CMaker.hh"
#include "CBndLib.hh"
#include "CMaterialLib.hh"
#include "CTestDetector.hh"

#include "PLOG.hh"



CTestDetector::CTestDetector(OpticksHub* hub, GGeoTestConfig* config, OpticksQuery* query)
  : 
  CDetector(hub, query),
  m_config(config),
  m_maker(NULL)
{
    init();
}



void CTestDetector::init()
{
    LOG(trace) << "CTestDetector::init" ; 

    if(m_ok->hasOpt("dbgtestgeo"))
    {
        LOG(info) << "CTestDetector::init --dbgtestgeo upping verbosity" ; 
        setVerbosity(1);
    }

    m_mlib->setGroupvelKludge(m_config->getGroupvel());

    m_maker = new CMaker(m_ok);

    LOG(trace) << "CTestDetector::init CMaker created" ; 

    G4VPhysicalVolume* top = makeDetector();

    LOG(trace) << "CTestDetector::init makeDetector DONE" ; 

    setTop(top) ; 
}

bool CTestDetector::isPmtInBox()
{
    const char* mode = m_config->getMode();
    return strcmp(mode, "PmtInBox") == 0 ;
}
bool CTestDetector::isBoxInBox()
{
    const char* mode = m_config->getMode();
    return strcmp(mode, "BoxInBox") == 0 ;
}


G4VPhysicalVolume* CTestDetector::makeDetector()
{
   // analagous to ggeo-/GGeoTest::CreateBoxInBox
   // but need to translate from a surface based geometry spec into a volume based one
   //
   // creates Russian doll geometry layer by layer, starting from the outermost 
   // hooking up mother volume to prior 
   //

    bool is_pib = isPmtInBox() ;
    bool is_bib = isBoxInBox() ;

    LOG(trace) << "CTestDetector::Construct"
               << " PmtInBox " << is_pib
               << " BoxInBox " << is_bib
              ;

    assert( ( is_pib || is_bib ) && "CTestDetector::Construct mode not recognized");

    if(m_verbosity > 0)
    m_config->dump("CTestDetector::Construct");

    unsigned int n = m_config->getNumElements();

    G4VPhysicalVolume* ppv = NULL ;    // parent pv
    G4VPhysicalVolume* top = NULL ;  
    G4LogicalVolume* mother = NULL ; 

    for(unsigned int i=0 ; i < n ; i++)
    {   
        const char* spec = m_config->getBoundary(i);

        unsigned boundary = m_blib->addBoundary(spec);

        GMaterial* imat = m_blib->getInnerMaterial(boundary); 

       // TODO:
       //    access corresponding GSur and add lvnames and pv1 pv2 indices 
       //    otherwise the surfaces fail to hookup
       //    as the lvn/pv1/pv2 dont match with the modified geometry

        const G4Material* material = m_mlib->convertMaterial(imat);

        glm::vec4 param = m_config->getParameters(i);
        char shapecode = m_config->getShape(i) ;
        const char* shapename = GMaker::ShapeName(shapecode);

        std::string lvn = CMaker::LVName(shapename);
        std::string pvn = CMaker::PVName(shapename);

        if(m_verbosity > 0)
        LOG(info) << "CTestDetector::Construct" 
                  << std::setw(2) << i 
                  << std::setw(2) << shapecode 
                  << std::setw(15) << shapename
                  << std::setw(30) << spec
                  << std::setw(20) << gformat(param)
                  ;   

        G4VSolid* solid = m_maker->makeSolid(shapecode, param);  

        G4LogicalVolume* lv = new G4LogicalVolume(solid, const_cast<G4Material*>(material), lvn.c_str(), 0,0,0);

        G4VPhysicalVolume* pv = new G4PVPlacement(0,G4ThreeVector(), lv, pvn.c_str(),mother,false,0);
        
        m_pvm[pvn] = pv ;  





 
        if(top == NULL) top = pv ; 
        if(ppv == NULL) ppv = pv ; 
        mother = lv ; 
    }   

    if(is_pib)
    {
        makePMT(mother);
    }

    //m_mlib->dumpMaterials("CTestDetector::Construct CPropLib::dumpMaterials");

    
    return top ;  
}


void CTestDetector::makePMT(G4LogicalVolume* container)
{
    // try without creating an explicit node tree 

    LOG(trace) << "CTestDetector::makePMT" ; 

    NSlice* slice = m_config->getSlice();

    GCSG* csg = m_mlib->getPmtCSG(slice);

    if(csg == NULL)
    {
        LOG(fatal) << " CTestDetector::makePMT NULL csg from CPropLib " ;
        setValid(false);
        return ; 
    }   


    if(m_verbosity > 1)
    csg->dump();

    unsigned int ni = csg->getNumItems();

    if(m_verbosity > 0)
    LOG(info) << "CTestDetector::makePMT" 
              << " csg items " << ni 
              ; 

    G4LogicalVolume* mother = container ; 

    std::map<unsigned int, G4LogicalVolume*> lvm ; 

    for(unsigned int index=0 ; index < ni ; index++)
    {
        unsigned int nix = csg->getNodeIndex(index); 
        if(nix == 0) continue ;
        // skip non-lv with nix:0, as those are constituents of the lv that get recursed over

        unsigned int pix = csg->getParentIndex(index); 
        const char* pvn = csg->getPVName(nix-1);

        if(m_verbosity > 0)
        LOG(info) << "CTestDetector::makePMT" 
                  << " csg items " << ni 
                  << " index " << std::setw(3) << index 
                  << " nix " << std::setw(3) << nix 
                  << " pix " << std::setw(3) << pix 
                  << " pvn " << pvn 
                  ; 

        G4LogicalVolume* logvol = makeLV(csg, index );

        lvm[nix-1] = logvol ;

        mother = NULL ; 

        if(nix - 1 == 0)
        { 
            mother = container ;
        }
        else
        {
            assert( pix > 0 && lvm.count(pix-1) == 1  );
            mother = lvm[pix-1] ;
        }
              
        G4RotationMatrix* rot = 0 ; 
        G4ThreeVector tlate(csg->getX(index), csg->getY(index), csg->getZ(index));  

        // pv translation, for DYB PMT only non-zero for pvPmtHemiBottom, pvPmtHemiDynode
        // suspect that G4DAE COLLADA export omits/messes this up somehow, for the Bottom at least

        if(m_verbosity > 0)
        LOG(info) << "CTestDetector::makePMT"
                  << " index " << index 
                  << " x " << tlate.x()
                  << " y " << tlate.y()
                  << " z " << tlate.z()
                  ;

        G4bool many = false ; 
        G4int copyNo = 0 ; 

        G4VPhysicalVolume* physvol = new G4PVPlacement(rot, tlate, logvol, pvn, mother, many, copyNo);

        m_pvm[pvn] = physvol ;  

    }

    kludgePhotoCathode();
}


void CTestDetector::kludgePhotoCathode()
{
   // HMM THIS IS A DIRTY KLUDGE ..
   //
   // Over in CGDMLDetector have implemented the GSurLib/CSurLib machinery
   // for revivifying optical surfaces "properly" ...
   //
   // Can that machinery be used here with test geometry ?
   //
   // Possibly but it would be quite involved as border surfaces need pv volume indices 
   // that change for test geometry.
   // Decided too much effort to do this in general when the only surface needed in test geometry for now is the cathode.
   //
   // See :doc:`notes/issues/geant4_opticks_integration/surlib_with_test_geometry` 
   //

    LOG(info) << "CTestDetector::kludgePhotoCathode" ;

    float effi = 1.f ; 
    float refl = 0.f ; 
    {
        const char* name = "kludgePhotoCathode_PyrexBialkali" ; 
        G4VPhysicalVolume* pv1 = getLocalPV("pvPmtHemi") ; 
        G4VPhysicalVolume* pv2 = getLocalPV("pvPmtHemiCathode") ; 
        assert(pv1 && pv2);
        //G4LogicalBorderSurface* lbs = m_mlib->makeCathodeSurface(name, pv1, pv2, effi, refl );
        G4LogicalBorderSurface* lbs = m_mlib->makeCathodeSurface(name, pv1, pv2);
        assert(lbs);
    }
    {
        const char* name = "kludgePhotoCathode_PyrexVacuum" ; 
        G4VPhysicalVolume* pv1 = getLocalPV("pvPmtHemi") ; 
        G4VPhysicalVolume* pv2 = getLocalPV("pvPmtHemiVacuum") ; 
        assert(pv1 && pv2);
        G4LogicalBorderSurface* lbs = m_mlib->makeConstantSurface(name, pv1, pv2, effi, refl );
        assert(lbs);
    }
}


G4LogicalVolume* CTestDetector::makeLV(GCSG* csg, unsigned int i)
{
    unsigned int ix = csg->getNodeIndex(i); 

    assert(ix > 0);

    const char* matname = csg->getMaterialName(ix - 1) ;

    const char* lvn = csg->getLVName(ix - 1)  ;  

    const G4Material* material = m_mlib->makeMaterial(matname) ;

    G4VSolid* solid = m_maker->makeSolid(csg, i );

    G4LogicalVolume* logvol = new G4LogicalVolume(solid, const_cast<G4Material*>(material), lvn);

    if(m_verbosity > 0)
    LOG(info) 
           << "CTestDetector::makeLV "
           << "  i " << std::setw(2) << i  
           << " ix " << std::setw(2) << ix  
           << " lvn " << std::setw(2) << lvn
           << " matname " << matname  
           ;

    return logvol ; 
}


