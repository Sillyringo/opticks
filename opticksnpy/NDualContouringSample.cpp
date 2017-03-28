#include <sstream>

#include "NDualContouringSample.hpp"
#include "NTrianglesNPY.hpp"
#include "NGLM.hpp"
#include "GLMFormat.hpp"

#include "DualContouringSample/mesh.h"
#include "DualContouringSample/octree.h"

#include "Timer.hpp"
#include "TimesTable.hpp"

#include "NSphere.hpp"
#include "NNode.hpp"
#include "NBox.hpp"
#include "NFieldCache.hpp"

#include "PLOG.hh"

NDualContouringSample::NDualContouringSample(int level, float threshold, float scale_bb)
  :
   m_timer(new Timer),
   m_level(level),
   m_octreeSize(1 << level),
   m_threshold(threshold),
   m_scale_bb(scale_bb)
{
   m_timer->start();
}

std::string NDualContouringSample::desc()
{
   std::stringstream ss ; 
   ss << "NDualContouringSample"
      << " level " << m_level
      << " octreeSize " << m_octreeSize
      << " threshold " << m_threshold
      << " scale_bb " << m_scale_bb
      ;
   return ss.str(); 
}


void NDualContouringSample::profile(const char* s)
{
   (*m_timer)(s);
}

void NDualContouringSample::report(const char* msg)
{
    LOG(info) << msg ; 
    LOG(info) << desc() ; 
    TimesTable* tt = m_timer->makeTable();
    tt->dump();
    //tt->save("$TMP");
}



NTrianglesNPY* NDualContouringSample::operator()(nnode* node)
{
    nbbox bb = node->bbox();  // overloaded method 
    bb.scale(m_scale_bb);     // kinda assumes centered at origin, slightly enlarge


    bb.side = bb.max - bb.min ; // TODO: see why this not set previously 


    nvec4     bbce = bb.center_extent();

    float xyzExtent = bbce.w  ;
    float ijkExtent = m_octreeSize/2 ;      // eg 64.f
    float ijk2xyz = xyzExtent/ijkExtent ;   // octree -> real world coordinates

    nvec4 ce = make_nvec4(bbce.x, bbce.y, bbce.z, ijk2xyz );

    LOG(info) << "NDualContouringSample"
              << " xyzExtent " << xyzExtent
              << " ijkExtent " << ijkExtent
              << " bbce " << bbce.desc()
              << " ce " << ce.desc()
              ;



    VertexBuffer vertices;
    IndexBuffer indices;

    vertices.clear();
    indices.clear();

    std::function<float(float,float,float)> f = node->sdf();


    profile("_BuildOctree");
    OctreeNode* octree = BuildOctree(m_level, m_threshold, &f, bb, ce, m_timer ) ;
    profile("BuildOctree");



    NTrianglesNPY* tris = NULL ; 

    if(octree == NULL)
    {   
        LOG(warning) << "NDualContouringSample : NULL octree  "
                     << " for node " << CSGName(node->type)
                     << " MAKING PLACEHOLDER BBOX TRIS "  
                     ;
        tris = NTrianglesNPY::box(bb);
        return tris ; 
    }

    profile("_GenerateMeshFromOctree");
    GenerateMeshFromOctree(octree, vertices, indices, bb, ce);
    profile("GenerateMeshFromOctree");


    LOG(info) << " vertices " << vertices.size() ;
    LOG(info) << " indices  " << indices.size() ;


    unsigned npol = indices.size() ; 

    assert( npol % 3 == 0) ;
    unsigned ntri = npol / 3 ; 

    IndexBuffer::iterator  pmin = std::min_element(std::begin(indices), std::end(indices));
    IndexBuffer::iterator  pmax = std::max_element(std::begin(indices), std::end(indices));

    size_t imin = std::distance(std::begin(indices), pmin) ;
    size_t imax = std::distance(std::begin(indices), pmax) ;

    LOG(debug) << "min element at: " << imin << " " << indices[imin] ; 
    LOG(debug) << "max element at: " << imax << " " << indices[imax] ;

    profile("_CollectTriangles");
    tris = new NTrianglesNPY();
    for(unsigned t=0 ; t < ntri ; t++)
    {
         assert( t*3+2 < npol );

         unsigned i0 = indices[t*3 + 0];
         unsigned i1 = indices[t*3 + 1];
         unsigned i2 = indices[t*3 + 2];
          
         MeshVertex& v0 = vertices[i0] ;
         MeshVertex& v1 = vertices[i1] ;
         MeshVertex& v2 = vertices[i2] ;

        /*
         LOG(info)
             << " t " << std::setw(5) << t 
             << " i0 " << std::setw(5) << i0  << " " << gformat(v0.xyz)  << " " << gformat(v0.normal) 
             << " i1 " << std::setw(5) << i1  << " " << gformat(v1.xyz)  << " " << gformat(v1.normal) 
             << " i2 " << std::setw(5) << i2  << " " << gformat(v2.xyz)  << " " << gformat(v2.normal) 
             ;
         */

         tris->add( v0.xyz, v1.xyz, v2.xyz );
         tris->addNormal( v0.normal, v1.normal, v2.normal );
    }
    profile("CollectTriangles");

    report("NDualContouringSample::");

    return tris ; 
}

