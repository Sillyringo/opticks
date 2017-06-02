#include "NPolygonizer.hpp"

#include "NParameters.hpp"
#include "NSphere.hpp"
#include "NPlane.hpp"
#include "NPrism.hpp"
#include "NPart.hpp"

#include "NTrianglesNPY.hpp"
#include "NCSG.hpp"

#include "NMarchingCubesNPY.hpp"


#ifdef WITH_DualContouringSample
#include "NDualContouringSample.hpp"
#endif

#ifdef WITH_ImplicitMesher
#include "NImplicitMesher.hpp"
#endif

#include "NHybridMesher.hpp"



#include "PLOG.hh"

NPolygonizer::NPolygonizer(NCSG* csg)
    :
    m_csg(csg), 
    m_root(csg->getRoot()),
    m_bbox(new nbbox(m_root->bbox())),
    m_meta(csg->getMetaParameters()),
    m_verbosity(m_meta->get<int>("verbosity", "0" )),
    m_index(m_csg->getIndex()),
    m_poly(NULL)
{
    assert(m_root);
    assert(m_meta);

    std::string poly = m_meta->get<std::string>("poly", "DCS");
    m_poly = strdup(poly.c_str());

    if(m_verbosity > 0)
    m_meta->dump("NPolygonizer::NPolygonizer(meta)");

}



NTrianglesNPY* NPolygonizer::polygonize()
{
    if(m_verbosity > 0)
    LOG(info) << "NPolygonizer::polygonize"
              << " treedir " << m_csg->getTreeDir()
              << " poly " << m_poly 
              << " verbosity " << m_verbosity 
              << " index " << m_index
              ;


    NTrianglesNPY* tris = NULL ; 

    if( strcmp(m_poly, "MC") == 0)
    {   
        tris = marchingCubesNPY();
    }   
    else if(strcmp(m_poly, "DCS") == 0)
    {   
        tris = dualContouringSample(); 
    }   
    else if(strcmp(m_poly, "IM") == 0)
    {
        tris = implicitMesher(); 
    }
    else if(strcmp(m_poly, "HY") == 0)
    {
        tris = hybridMesher(); 
    }
    else
    {
        assert(0);
    }

    bool valid = checkTris(tris);

    if(!valid)
    {   
        if(m_verbosity > 0)
        LOG(warning) << "INVALID NPolygonizer tris with " << m_poly ; 
        delete tris ; 
        tris = NTrianglesNPY::box(*m_bbox);
        tris->setMessage("PLACEHOLDER");
    }   
    else
    {
        unsigned numTris = tris ? tris->getNumTriangles() : 0 ;
        LOG(info) << "NPolygonizer::polygonize OK " 
                  << " numTris " << numTris 
                  ; 
    }

    return tris ;
}


bool NPolygonizer::checkTris(NTrianglesNPY* tris)
{
    unsigned numTris = tris ? tris->getNumTriangles() : 0 ;

    nbbox* tris_bb = tris && numTris > 0 ? tris->findBBox() : NULL ;

    bool poly_valid = tris_bb ? m_bbox->contains(*tris_bb) : false  ;

    if(!poly_valid && m_verbosity > 0)
    {
        LOG(warning) << "NPolygonizer::checkTris INVALID POLYGONIZATION "
                     << " poly " << m_poly
                     << " index " << m_index 
                     << " numTris " << numTris
                     ;

        std::cout << " node_bb " << m_bbox->desc() << std::endl ; 
        std::cout << " tris_bb " << ( tris_bb ? tris_bb->desc() : "bb:NULL" ) << std::endl ; 
    }
    return poly_valid ;
}



NTrianglesNPY* NPolygonizer::marchingCubesNPY()
{
    int nx = m_meta->get<int>("nx", "15" );
    NMarchingCubesNPY poly(nx) ;
    NTrianglesNPY* tris = poly(m_root);
    return tris ; 
}

NTrianglesNPY* NPolygonizer::dualContouringSample()
{
    NTrianglesNPY* tris = NULL ; 
#ifdef WITH_DualContouringSample
    float threshold = m_meta->get<float>("threshold", "0.1" );
    int   nominal = m_meta->get<int>("nominal", "7" );  // 1 << 5 = 32, 1 << 6 = 64, 1 << 7 = 128  
    int   coarse  = m_meta->get<int>("coarse", "6" );  
    NDualContouringSample poly(nominal, coarse, m_verbosity, threshold ) ; 
    tris = poly(m_root);
#else
    assert(0 && "installation does not have DualContouringSample support" );
#endif
    return tris ;
}

NTrianglesNPY* NPolygonizer::implicitMesher()
{
    NTrianglesNPY* tris = NULL ; 
#ifdef WITH_ImplicitMesher
    int   resolution = m_meta->get<int>("resolution", "100" );
    int   ctrl = m_meta->get<int>("ctrl", "0" );
    float expand_bb = 1e-4 ; 
    std::string seeds = m_meta->get<std::string>("seeds", "" );
    NImplicitMesher poly(m_root, resolution, m_verbosity, expand_bb, ctrl, seeds ) ; 
    tris = poly();
#else
    assert(0 && "installation does not have ImplicitMesher support" );
#endif
    return tris ;
}


NTrianglesNPY* NPolygonizer::hybridMesher()
{
    NTrianglesNPY* tris = NULL ; 
    int   level = m_meta->get<int>("level", "5" );
    int   ctrl = m_meta->get<int>("ctrl", "0" );
    NHybridMesher poly(m_root, level, m_verbosity, ctrl ) ; 
    tris = poly();
    return tris ;
}


