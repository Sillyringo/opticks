/*
 * Copyright (c) 2019 Opticks Team. All Rights Reserved.
 *
 * This file is part of Opticks
 * (see https://bitbucket.org/simoncblyth/opticks).
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); 
 * you may not use this file except in compliance with the License.  
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software 
 * distributed under the License is distributed on an "AS IS" BASIS, 
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  
 * See the License for the specific language governing permissions and 
 * limitations under the License.
 */


#include <cfloat>
#include <cstdio>
#include <cstring>
#include <sstream>
#include <cassert>
#include <iomanip>
#include <algorithm>

// TODO: eliminate direct use of boost, should always go thru my wrapper instead
#include <boost/filesystem.hpp>

#include "SStr.hh"
#include "BFile.hh"


#include "NGLM.hpp"
#include "NGLMExt.hpp"
#include "GLMFormat.hpp"
#include "NBBox.hpp"
#include "NPY.hpp"
#include "NCSG.hpp"

#include "Opticks.hh"


#include "GMatrix.hh"
#include "GMeshFixer.hh"
#include "GBuffer.hh"
#include "GMesh.hh"

#include "SLOG.hh"

/**
TODO List
---------

1. replace gfloat3/GBuffer with NPoint/NPY approach  


**/


const plog::Severity GMesh::LEVEL = SLOG::EnvLevel("GMesh", "DEBUG" ); 

namespace fs = boost::filesystem;


const char* GMesh::vertices_     = "vertex_vertices" ;
const char* GMesh::normals_      = "vertex_normals" ;
const char* GMesh::colors_       = "vertex_colors" ;
const char* GMesh::texcoords_    = "vertex_texcoords" ;

const char* GMesh::indices_        = "face_indices" ;
const char* GMesh::nodes_          = "face_nodes" ;
const char* GMesh::boundaries_     = "face_boundaries" ;
const char* GMesh::sensors_        = "face_sensors" ;

const char* GMesh::center_extent_  = "volume_center_extent" ;
const char* GMesh::bbox_           = "volume_bbox" ;
const char* GMesh::transforms_     = "volume_transforms" ;
const char* GMesh::meshes_         = "volume_meshes" ;
const char* GMesh::nodeinfo_       = "volume_nodeinfo" ;
const char* GMesh::identity_       = "volume_identity" ;

#ifdef WITH_COMPONENT 
const char* GMesh::components_     = "components" ;
#endif

const char* GMesh::itransforms_    = "placement_itransforms" ;
const char* GMesh::iidentity_      = "placement_iidentity" ;


void GMesh::nameConstituents(std::vector<std::string>& names)
{
    names.push_back(vertices_); 
    names.push_back(normals_); 
    names.push_back(colors_); 
    names.push_back(texcoords_); 

    names.push_back(indices_); 
    names.push_back(nodes_); 
    names.push_back(boundaries_); 
    names.push_back(sensors_); 

    names.push_back(center_extent_); 
    names.push_back(bbox_); 
    names.push_back(transforms_); 
    names.push_back(meshes_); 
    names.push_back(nodeinfo_); 
    names.push_back(identity_); 

    names.push_back(itransforms_); 
    names.push_back(iidentity_); 

#ifdef WITH_COMPONENT 
    names.push_back(components_); 
#endif
}


int GMesh::g_instance_count = 0 ;


bool GMesh::isEmpty() const 
{
    return m_num_vertices == 0 && m_num_faces == 0 ; 
}


void GMesh::setX4SkipSolid(bool x4skipsolid)
{
    m_x4skipsolid = x4skipsolid ; 
}
bool GMesh::isX4SkipSolid() const 
{
    return m_x4skipsolid ; 
}


/**
GMesh::GMesh
--------------

GMesh are created from G4VSolid by  X4PhysicalVolume::convertSolid/X4Mesh::Convert
which uses GMeshMaker::Make to go from NPY arrays into gfloat and calculates the normals.


**/

GMesh::GMesh(unsigned int index, 
             gfloat3* vertices, 
             unsigned num_vertices, 
             guint3*  faces, 
             unsigned num_faces, 
             gfloat3* normals, 
             gfloat2* texcoords
            ) 
        :
      GDrawable(),
      m_index(index),

      m_num_vertices(num_vertices), 
      m_num_faces(num_faces),
      m_num_volumes(0),
      m_num_volumes_selected(0),
      m_num_mergedmesh(0),

      m_nodes(NULL),          
      m_boundaries(NULL),
      m_sensors(NULL),

      m_vertices(NULL),
      m_normals(NULL),
      m_colors(NULL),
      m_texcoords(NULL),
      m_faces(NULL),

      m_low(NULL),
      m_high(NULL),
      m_dimensions(NULL),
      m_center(NULL),
      m_extent(0.f),

      m_center_extent(NULL),
      m_bbox(NULL),
      m_transforms(NULL),
      m_itransforms(NULL),
      m_meshes(NULL),
      m_nodeinfo(NULL),
      m_identity(NULL),
      m_iidentity(NULL),

      m_model_to_world(NULL),
      m_name(NULL),
      m_shortname(NULL),
      m_version(NULL),
      m_geocode('T'),   // 'T'/'A' 
      m_islice(NULL),
      m_fslice(NULL),
      m_pslice(NULL),

      m_vertices_buffer(NULL),
      m_normals_buffer(NULL),
      m_colors_buffer(NULL),
      m_texcoords_buffer(NULL),
      m_indices_buffer(NULL),
      m_center_extent_buffer(NULL),
      m_bbox_buffer(NULL),
      m_nodes_buffer(NULL),
      m_boundaries_buffer(NULL),
      m_sensors_buffer(NULL),
      m_transforms_buffer(NULL),
      m_meshes_buffer(NULL),
      m_nodeinfo_buffer(NULL),
      m_identity_buffer(NULL),

      m_itransforms_buffer(NULL),
      m_iidentity_buffer(NULL),

      m_components_buffer(NULL),

      m_facerepeated_identity_buffer(NULL),
      m_facerepeated_iidentity_buffer(NULL),
      m_analytic_geometry_buffer(NULL),

      m_csg(NULL),
      m_alt(NULL),
      m_verbosity(0),
      m_parts(NULL),
      m_pt(NULL),

      m_x4src_vtx(NULL), 
      m_x4src_idx(NULL),
      m_g4vsolid(NULL),
      m_x4skipsolid(false)

{
     init(vertices, faces, normals, texcoords);
}


/**
GMesh::init
------------

Adopts ownership of the input arrays.

**/

void GMesh::init(gfloat3* vertices, guint3* faces, gfloat3* normals, gfloat2* texcoords)
{
   g_instance_count += 1 ; 
   setVertices(vertices);
   setFaces(faces);
   setNormals(normals);
   setTexcoords(texcoords);
   updateBounds();
   nameConstituents(m_names);
}



void GMesh::stealIdentity(GMesh* other)
{
    setParts(other->getParts());
    setITransformsBuffer( other->getITransformsBuffer() );
    setInstancedIdentityBuffer( other->getInstancedIdentityBuffer() );

    // hmm passing everything over is too complicated, maybe better to do LODification inplace
    //     to avoid this        
}






void GMesh::setCSG(const NCSG* csg)
{
    m_csg = csg ; 
}
const NCSG* GMesh::getCSG() const 
{
    return m_csg ; 
}



const nnode* GMesh::getRoot() const 
{
    return m_csg ? m_csg->getRoot() : NULL  ; 
}

void GMesh::setCSGBoundary(const char* spec)
{
    assert( m_csg ); 

    NCSG* csg = const_cast<NCSG*>(m_csg); 
    csg->setBoundary( spec );  
}

void GMesh::setAlt(const GMesh* alt)
{
    m_alt = alt ; 
}
const GMesh* GMesh::getAlt() const 
{
    return m_alt ; 
}




void GMesh::deallocate()
{
    

    delete[] m_vertices ;  
    delete[] m_normals ;  
    delete[] m_colors ;  
    delete[] m_texcoords ;  
    delete[] m_faces ;  

    delete[] m_center_extent ;  
    delete[] m_bbox ;  
    delete[] m_transforms ;  
    delete[] m_itransforms ;  
    delete[] m_meshes ;  
    delete[] m_nodeinfo ;  
    delete[] m_identity ;  
    delete[] m_iidentity ;  

    // NB buffers and the rest are very lightweight 
}


GMesh::~GMesh()
{
    deallocate();
}

void GMesh::setVerbosity(unsigned int verbosity)
{
    m_verbosity = verbosity ; 
}

unsigned int GMesh::getVerbosity() const 
{
    return m_verbosity ; 
}

void GMesh::setName(const char* name)
{
     m_name = name ? strdup(name) : NULL ;
     if(m_name) findShortName();
}  
const char* GMesh::getName() const 
{
     return m_name ; 
}
const char* GMesh::getShortName() const 
{
     return m_shortname ; 
}


void GMesh::setVersion(const char* version)
{
     m_version = version ? strdup(version) : NULL ;
}  
const char* GMesh::getVersion() const 
{
     return m_version ; 
}




unsigned int GMesh::getIndex() const 
{
    return m_index ; 
}
unsigned int GMesh::getNumVertices() const 
{
    return m_num_vertices ; 
}

/**
GMesh::getNumFaces
-------------------

For GMergedMesh instances this is set during mesh merging::

    GMergedMesh::Create
    GMergedMesh::traverse_r
    GMergedMesh::countVolume 
    GMergedMesh::countMesh 

**/

unsigned int GMesh::getNumFaces() const 
{
    return m_num_faces ; 
}

/**
GMesh::getNumVolumes
---------------------

* m_num_volumes is set by setCenterExtentBuffer

**/
unsigned int GMesh::getNumVolumes() const 
{
    return m_num_volumes ; 
}
unsigned int GMesh::getNumVolumesSelected() const 
{
    return m_num_volumes_selected ; 
}




void GMesh::setIndex(unsigned int index)
{
    m_index = index ;
}
void GMesh::setNumVertices(unsigned int num_vertices)
{
    m_num_vertices = num_vertices ; 
}
void GMesh::setNumFaces(unsigned int num_faces)
{
    m_num_faces = num_faces ; 
}




void GMesh::setLow(gfloat3* low)
{
    m_low = low ;
}
void GMesh::setHigh(gfloat3* high)
{
    m_high = high ;
}
bool GMesh::hasTexcoords() const 
{
    return m_texcoords != NULL ;
}







gfloat3* GMesh::getLow()
{
    return m_low ;
}
gfloat3* GMesh::getHigh()
{
    return m_high ;
}
gfloat3* GMesh::getDimensions()
{
    return m_dimensions ; 
}

GMatrix<float>* GMesh::getModelToWorld()
{
    return m_model_to_world ; 
}


gfloat3* GMesh::getVertices() const 
{
    return m_vertices ;
}
gfloat3* GMesh::getNormals() const 
{
    return m_normals ;
}

gfloat3* GMesh::getColors() const 
{
    return m_colors ;
}
gfloat2* GMesh::getTexcoords() const 
{
    return m_texcoords ;
}


guint3*  GMesh::getFaces() const 
{
    return m_faces ;
}


// index is used from subclass
gfloat4 GMesh::getCenterExtent(unsigned index) const 
{
    return m_center_extent[index] ;
}

glm::vec4 GMesh::getCE(unsigned index) const 
{
    gfloat4 ce_ = getCenterExtent(index);
    glm::vec4 ce(ce_.x, ce_.y, ce_.z, ce_.w ) ; 
    return ce  ;
}

float GMesh::getBoundingRadiusCE(unsigned index) const
{
    // radius of the origin centered sphere that contains all of the volume
    // without safety margin
    glm::vec4 ce = getCE(index) ;
    float bounding_radius = glm::length(glm::vec3(ce)) + ce.w ;
    return bounding_radius ; 
}


gbbox GMesh::getBBox(unsigned int index) const 
{
    return m_bbox[index] ;
}
gbbox* GMesh::getBBoxPtr()
{
    return m_bbox ;
}






float GMesh::getExtent()
{
     return m_extent ;  
}



GBuffer*  GMesh::getModelToWorldBuffer()
{
    return (GBuffer*)m_model_to_world ;
}

float* GMesh::getModelToWorldPtr(unsigned int /*index*/)
{
     return (float*)getModelToWorldBuffer()->getPointer() ; 
}


unsigned* GMesh::getNodes() const    // CAUTION ONLY MAKES SENSE FROM GMergedMesh SUBCLASS 
{
    return m_nodes ;
}


unsigned int* GMesh::getMeshIndice() const
{
    return m_meshes ;
}
unsigned int GMesh::getMeshIndice(unsigned int index) const
{
    return m_meshes[index] ;
}




guint4* GMesh::getNodeInfo() const 
{
    return m_nodeinfo ; 
}
guint4 GMesh::getNodeInfo(unsigned int index) const
{
    return m_nodeinfo[index] ; 
}



guint4* GMesh::getIdentity() const 
{
    return m_identity ; 
}
guint4 GMesh::getIdentity(unsigned int index) const 
{
    return m_identity[index] ; 
}
glm::uvec4 GMesh::getIdentity_(unsigned index) const 
{
    guint4 id = m_identity[index] ; 
    return id.as_vec(); 
}



guint4* GMesh::getInstancedIdentity() const
{
    return m_iidentity ; 
}

/**
GMesh::getInstancedIdentity
-----------------------------

All nodes of the geometry tree have a quad of identity uint.
InstancedIdentity exists to rearrange that identity information 
into a buffer that can be used for creation of the GPU instanced geometry,
which requires to access the identity with an instance index, rather 
than the node index.

See notes/issues/identity_review.rst

**/

guint4 GMesh::getInstancedIdentity(unsigned int index) const
{
    return m_iidentity[index] ; 
}

glm::uvec4 GMesh::getInstancedIdentity_(unsigned index) const 
{
    guint4 id = m_iidentity[index]; 
    return id.as_vec(); 
}




unsigned int* GMesh::getBoundaries() const 
{
    return m_boundaries ;
}
unsigned int* GMesh::getSensors() const 
{
    return m_sensors ;
}




GBuffer* GMesh::getVerticesBuffer()
{
    return m_vertices_buffer ;
}
GBuffer* GMesh::getNormalsBuffer()
{
    return m_normals_buffer ;
}
GBuffer* GMesh::getColorsBuffer()
{
    return m_colors_buffer ;
}
GBuffer* GMesh::getTexcoordsBuffer()
{
    return m_texcoords_buffer ;
}
GBuffer*  GMesh::getCenterExtentBuffer()
{
    return m_center_extent_buffer ;
}
GBuffer*  GMesh::getBBoxBuffer()
{
    return m_bbox_buffer ;
}

GBuffer*  GMesh::getTransformsBuffer() const 
{
    return m_transforms_buffer ;
}
NPY<float>*  GMesh::getITransformsBuffer() const 
{
    return m_itransforms_buffer ;
}



GBuffer*  GMesh::getMeshesBuffer() const 
{
    return m_meshes_buffer ;
}
GBuffer*  GMesh::getNodeInfoBuffer() const 
{
    return m_nodeinfo_buffer ;
}
GBuffer*  GMesh::getIdentityBuffer() const 
{
    return m_identity_buffer ;
}
NPY<unsigned>*  GMesh::getInstancedIdentityBuffer() const 
{
    return m_iidentity_buffer ;
}



GBuffer*  GMesh::getIndicesBuffer()
{
    return m_indices_buffer ;
}
GBuffer*  GMesh::getNodesBuffer()
{
    return m_nodes_buffer ;
}
GBuffer*  GMesh::getBoundariesBuffer()
{
    return m_boundaries_buffer ;
}
GBuffer*  GMesh::getSensorsBuffer()
{
    return m_sensors_buffer ;
}

bool GMesh::hasTransformsBuffer()
{
    return m_transforms_buffer != NULL ; 
}
bool GMesh::hasITransformsBuffer()
{
    return m_itransforms_buffer != NULL ; 
}







char GMesh::getGeoCode() const 
{
    return m_geocode ; 
}
void GMesh::setGeoCode(char geocode)
{
    m_geocode = geocode ; 
}


void GMesh::setInstanceSlice(NSlice* slice)
{
    m_islice = slice ; 
}
NSlice* GMesh::getInstanceSlice() const
{
    return m_islice ; 
}


void GMesh::setFaceSlice(NSlice* slice)
{
    m_fslice = slice ; 
}
NSlice* GMesh::getFaceSlice()
{
    return m_fslice ; 
}

void GMesh::setPartSlice(NSlice* slice)
{
    m_pslice = slice ; 
}
NSlice* GMesh::getPartSlice()
{
    return m_pslice ; 
}


GParts* GMesh::getParts() const 
{
    return m_parts ; 
}
void GMesh::setParts(GParts* pts) 
{
    m_parts = pts ; 
}
GPt* GMesh::getPt() const 
{
    return m_pt ; 
}
void GMesh::setPt(GPt* pt) 
{
    m_pt = pt ; 
}



/**
GMesh::allocate
-----------------

Only used from GMergedMesh in GMergedMesh::Create

**/

void GMesh::allocate()
{
    unsigned int numVertices = getNumVertices();
    unsigned int numFaces = getNumFaces();
    unsigned int numVolumes = getNumVolumes();

    bool empty = numVertices == 0 && numFaces == 0 ; 

    LOG_IF(warning, empty) << "GMesh::allocate EMPTY"
              << " numVertices " << numVertices
              << " numFaces " << numFaces
              << " numVolumes " << numVolumes
              ;

    //assert(numVertices > 0 && numFaces > 0 && numVolumes > 0);
    assert(numVolumes > 0);

    if(numVertices > 0 && numFaces > 0)
    {

        setVertices(new gfloat3[numVertices]); 
        setNormals( new gfloat3[numVertices]);
        setColors(  new gfloat3[numVertices]);
        setTexcoords( NULL );  

        setColor(0.5,0.5,0.5);  // starting point mid-grey, change in traverse 2nd pass

        // consolidate into guint4 

        guint3* faces = new guint3[numFaces] ;
        setFacesQty(faces);
    }


    setCenterExtent(new gfloat4[numVolumes]);
    setBBox(new gbbox[numVolumes]);
    //setBBox(new nbbox[numVolumes]);
    setMeshes(new unsigned[numVolumes]);
    setNodeInfo(new guint4[numVolumes]);
    setIdentity(new guint4[numVolumes]);
    setTransforms(new float[numVolumes*16]);

    //LOG(info) << "GMesh::allocate DONE " ;
}



void GMesh::setFacesQty(guint3* faces )
{
    // TODO: consolidate into uint4 with one spare
    if(m_num_faces == 0) return ;      

    if(faces)
    {
        setFaces( faces );
    }

    setNodes(        new unsigned[m_num_faces]);
    setBoundaries(   new unsigned[m_num_faces]);
    setSensors(      new unsigned[m_num_faces]);

    for(unsigned i=0 ; i < m_num_faces ; i++)
    {
        m_nodes[i] = 0u ; 
        m_boundaries[i] = 0u ; 
        m_sensors[i] = 0u ; 
    }
}




GBuffer* GMesh::getBuffer(const char* name) const 
{
    if(isNPYBuffer(name)) return NULL ; 

    if(strcmp(name, vertices_) == 0)     return m_vertices_buffer ; 
    if(strcmp(name, normals_) == 0)      return m_normals_buffer ; 
    if(strcmp(name, colors_) == 0)       return m_colors_buffer ; 
    if(strcmp(name, texcoords_) == 0)    return m_texcoords_buffer ; 

    if(strcmp(name, indices_) == 0)      return m_indices_buffer ; 
    if(strcmp(name, nodes_) == 0)        return m_nodes_buffer ; 
    if(strcmp(name, boundaries_) == 0)   return m_boundaries_buffer ; 
    if(strcmp(name, sensors_) == 0)      return m_sensors_buffer ; 

    if(strcmp(name, center_extent_) == 0)   return m_center_extent_buffer ; 
    if(strcmp(name, bbox_) == 0)            return m_bbox_buffer ; 
    if(strcmp(name, transforms_) == 0)      return m_transforms_buffer ; 
    if(strcmp(name, meshes_) == 0)          return m_meshes_buffer ; 
    if(strcmp(name, nodeinfo_) == 0)        return m_nodeinfo_buffer ; 
    if(strcmp(name, identity_) == 0)        return m_identity_buffer ; 

    return NULL ;
}


void GMesh::setBuffer(const char* name, GBuffer* buffer)
{
    if(isNPYBuffer(name)) return ; 

    if(strcmp(name, vertices_) == 0)     setVerticesBuffer(buffer) ; 
    if(strcmp(name, normals_) == 0)      setNormalsBuffer(buffer) ; 
    if(strcmp(name, colors_) == 0)       setColorsBuffer(buffer) ; 
    if(strcmp(name, texcoords_) == 0)    setTexcoordsBuffer(buffer) ; 

    if(strcmp(name, indices_) == 0)      setIndicesBuffer(buffer) ; 
    if(strcmp(name, nodes_) == 0)        setNodesBuffer(buffer) ; 
    if(strcmp(name, boundaries_) == 0)   setBoundariesBuffer(buffer) ; 
    if(strcmp(name, sensors_) == 0)      setSensorsBuffer(buffer) ; 

    if(strcmp(name, center_extent_) == 0)   setCenterExtentBuffer(buffer) ; 
    if(strcmp(name, bbox_) == 0)            setBBoxBuffer(buffer) ; 
    if(strcmp(name, transforms_) == 0)      setTransformsBuffer(buffer) ; 
    if(strcmp(name, meshes_) == 0)          setMeshesBuffer(buffer) ; 
    if(strcmp(name, nodeinfo_) == 0)        setNodeInfoBuffer(buffer) ; 
    if(strcmp(name, identity_) == 0)        setIdentityBuffer(buffer) ; 
}



void GMesh::applyCentering()
{
    // use analytic bbox center_extent when there is an associated CSG solid

    glm::vec4 ce = m_csg ? m_csg->bbox_center_extent() : getCE(0) ; 

    LOG(debug) << " ce " << gformat(ce) ; 

    glm::vec3 tla(-ce.x, -ce.y, -ce.z); 

    applyTranslation( tla.x, tla.y, tla.z ); 

    if(m_csg)
    {
        const_cast<NCSG*>(m_csg)->set_translation( tla.x, tla.y, tla.z ); 
    }

}

void GMesh::applyTranslation(float x, float y, float z )
{
    glm::mat4 txf(nglmext::make_translate(x, y, z)); 
    GMatrixF* transform = new GMatrix<float>(glm::value_ptr(txf));
    applyTransform(*transform);  
}

void GMesh::applyTransform( GMatrixF& transform )
{
    gfloat3* vertices = getTransformedVertices(transform); 
    gfloat3* normals  = getTransformedNormals(transform); 

    unsigned num_vertices = getNumVertices() ; 

    updateVertices(vertices, num_vertices ); 
    updateNormals(normals, num_vertices ); 
    updateBounds(); 
}


void GMesh::updateVertices(gfloat3* vertices, unsigned num_vertices)
{
    assert( num_vertices == m_num_vertices ); 
 
    delete [] m_vertices ; 
    delete m_vertices_buffer ; 

    setVertices( vertices );   
}
void GMesh::setVertices(gfloat3* vertices)
{
    m_vertices = vertices ;
    m_vertices_buffer = new GBuffer( sizeof(gfloat3)*m_num_vertices, (void*)m_vertices, sizeof(gfloat3), 3 , "vertices") ;

    assert(sizeof(gfloat3) == sizeof(float)*3);
}
void GMesh::setVerticesBuffer(GBuffer* buffer)
{
    m_vertices_buffer = buffer ; 
    if(!buffer) return ; 

    m_vertices = (gfloat3*)buffer->getPointer();
    unsigned int numBytes = buffer->getNumBytes();
    m_num_vertices = numBytes/sizeof(gfloat3);
}



void GMesh::updateNormals(gfloat3* normals, unsigned num_normals)
{
    assert( num_normals == m_num_vertices ); 
 
    delete [] m_normals ; 
    delete m_normals_buffer ; 

    setNormals( normals );   
}
void GMesh::setNormals(gfloat3* normals)
{
    m_normals = normals ;
    m_normals_buffer = new GBuffer( sizeof(gfloat3)*m_num_vertices, (void*)m_normals, sizeof(gfloat3), 3 , "normals") ;
    assert(sizeof(gfloat3) == sizeof(float)*3);
}
void GMesh::setNormalsBuffer(GBuffer* buffer)
{
    m_normals_buffer = buffer ; 
    if(!buffer) return ; 
    m_normals = (gfloat3*)buffer->getPointer();
    unsigned int numBytes = buffer->getNumBytes();
    unsigned int num_normals = numBytes/sizeof(gfloat3);
    assert( m_num_vertices == num_normals );  // must load vertices before normals
}

void GMesh::setColors(gfloat3* colors)
{
    m_colors = colors ;
    m_colors_buffer = new GBuffer( sizeof(gfloat3)*m_num_vertices, (void*)m_colors, sizeof(gfloat3), 3 , "colors") ;
    assert(sizeof(gfloat3) == sizeof(float)*3);
}
void GMesh::setColorsBuffer(GBuffer* buffer)
{
    m_colors_buffer = buffer ; 
    if(!buffer) return ; 
    
    m_colors = (gfloat3*)buffer->getPointer();
    unsigned int numBytes = buffer->getNumBytes();
    unsigned int num_colors = numBytes/sizeof(gfloat3);

    assert( m_num_vertices == num_colors );  // must load vertices before colors
}


void GMesh::setCenterExtent(gfloat4* center_extent)  
{
    m_center_extent = center_extent ;  

    LOG(debug) << "GMesh::setCenterExtent (creates buffer) " 
              << " m_center_extent " << m_center_extent
              << " m_num_volumes " << m_num_volumes 
              ; 

    assert(m_num_volumes > 0);
    m_center_extent_buffer = new GBuffer( sizeof(gfloat4)*m_num_volumes, (void*)m_center_extent, sizeof(gfloat4), 4 , "cen_ext"); 
    assert(sizeof(gfloat4) == sizeof(float)*4);
}

/**
GMesh::setCenterExtentBuffer
----------------------------

Also sets m_num_volumes, based on the buffer size.

**/

void GMesh::setCenterExtentBuffer(GBuffer* buffer) 
{
    m_center_extent_buffer = buffer ;  
    if(!buffer) return ; 

    m_center_extent = (gfloat4*)buffer->getPointer();
    unsigned int numBytes = buffer->getNumBytes();
    m_num_volumes = numBytes/sizeof(gfloat4) ;

    LOG(debug) << "GMesh::setCenterExtentBuffer  (creates array from buffer) " 
              << " m_center_extent " << m_center_extent
              << " m_num_volumes " << m_num_volumes 
              ; 

}


void GMesh::setBBox(gbbox* bb)  
{
    m_bbox = bb ;  
    assert(m_num_volumes > 0);
    m_bbox_buffer = new GBuffer( sizeof(gbbox)*m_num_volumes, (void*)m_bbox, sizeof(gbbox), 6 , "bbox"); 
    assert(sizeof(gbbox) == sizeof(float)*6);
}
void GMesh::setBBoxBuffer(GBuffer* buffer) 
{
    m_bbox_buffer = buffer ;  
    if(!buffer) return ; 

    m_bbox = (gbbox*)buffer->getPointer();
    unsigned int numBytes = buffer->getNumBytes();
    unsigned int numVolumes = numBytes/sizeof(gbbox) ;

    setNumVolumes(numVolumes);
}



void GMesh::setTransforms(float* transforms)  
{
    m_transforms = transforms ;  
    assert(m_num_volumes > 0);

    unsigned int numElements = 16 ; 
    unsigned int size = sizeof(float)*numElements;

    LOG(debug) << "GMesh::setTransforms " 
              << " num_volumes " << m_num_volumes 
              << " size " << size 
              << " fsize " << sizeof(float)
              ;

    m_transforms_buffer = new GBuffer( size*m_num_volumes, (void*)m_transforms, size, numElements , "transforms"); 
}


void GMesh::setTransformsBuffer(GBuffer* buffer) 
{
    m_transforms_buffer = buffer ;  
    if(!buffer) return ; 
    m_transforms = (float*)buffer->getPointer();
}

void GMesh::setITransformsBuffer(NPY<float>* buffer) 
{
    m_itransforms_buffer = buffer ;  
    if(!buffer) return ; 
    m_itransforms = buffer->getValues();
}






unsigned int GMesh::getNumTransforms() const 
{
    return m_transforms_buffer ? m_transforms_buffer->getNumBytes()/(16*sizeof(float)) : 0 ; 
}
unsigned int GMesh::getNumITransforms() const 
{
    if(!m_itransforms_buffer) return 0 ;    
    unsigned int n0 = m_itransforms_buffer->getNumBytes()/(16*sizeof(float)) ; 
    unsigned int n1 = m_itransforms_buffer->getNumItems() ;
    assert(n0 == n1); 
    return n1 ;  
}


float* GMesh::getTransform(unsigned index) const 
{
    if(index >= m_num_volumes)
    {
        LOG(fatal) << "GMesh::getTransform out of bounds " 
                     << " m_num_volumes " << m_num_volumes 
                     << " index " << index
                     ;
        assert(0);
    }
    return index < m_num_volumes ? m_transforms + index*16 : NULL  ;
}

glm::mat4 GMesh::getTransform_(unsigned index) const 
{
    float* transform = getTransform(index) ;
    glm::mat4 tr = glm::make_mat4(transform) ;
    return tr ; 
}

float* GMesh::getITransform(unsigned index) const 
{
    unsigned num_itransforms = getNumITransforms();
    return index < num_itransforms ? m_itransforms + index*16 : NULL  ;
}

glm::mat4 GMesh::getITransform_(unsigned index) const 
{
    float* transform = getITransform(index) ;
    glm::mat4 tr = glm::make_mat4(transform) ;
    return tr ; 
}





void GMesh::setMeshes(unsigned int* meshes)  
{
    LOG(LEVEL) ; 
    m_meshes = meshes ;  
    assert(m_num_volumes > 0);
    unsigned int size = sizeof(unsigned int);
    m_meshes_buffer = new GBuffer( size*m_num_volumes, (void*)m_meshes, size, 1 , "meshes"); 
}

void GMesh::setMeshesBuffer(GBuffer* buffer) 
{
    LOG(LEVEL) ; 
    m_meshes_buffer = buffer ;  
    if(!buffer) return ; 

    m_meshes = (unsigned int*)buffer->getPointer();
    unsigned int numBytes = buffer->getNumBytes();
    unsigned int numElements = 1  ; 
    unsigned int size = sizeof(float)*numElements;
    unsigned int numVolumes = numBytes/size ;
    setNumVolumes(numVolumes);
}



void GMesh::setNodeInfo(guint4* nodeinfo)  
{
    LOG(LEVEL) ; 
    m_nodeinfo = nodeinfo ;  
    assert(m_num_volumes > 0);
    unsigned int size = sizeof(guint4);
    assert(size == sizeof(unsigned int)*4 );
    m_nodeinfo_buffer = new GBuffer( size*m_num_volumes, (void*)m_nodeinfo, size, 4 , "nodeinfo"); 
}
void GMesh::setNodeInfoBuffer(GBuffer* buffer) 
{
    LOG(LEVEL) ; 
    m_nodeinfo_buffer = buffer ;  
    if(!buffer) return ; 

    m_nodeinfo = (guint4*)buffer->getPointer();
    unsigned int numBytes = buffer->getNumBytes();
    unsigned int size = sizeof(guint4);
    assert(size == sizeof(unsigned int)*4 );
    unsigned int numVolumes = numBytes/size ;
    setNumVolumes(numVolumes);
}




void GMesh::setIdentity(guint4* identity)  
{
    LOG(LEVEL) ; 
    m_identity = identity ;  
    assert(m_num_volumes > 0);
    unsigned int size = sizeof(guint4);
    assert(size == sizeof(unsigned int)*4 );
    m_identity_buffer = new GBuffer( size*m_num_volumes, (void*)m_identity, size, 4 , "identity"); 
}
void GMesh::setIdentityBuffer(GBuffer* buffer) 
{
    LOG(LEVEL) ; 
    m_identity_buffer = buffer ;  
    if(!buffer) return ; 

    m_identity = (guint4*)buffer->getPointer();
    unsigned int numBytes = buffer->getNumBytes();
    unsigned int size = sizeof(guint4);
    assert(size == sizeof(unsigned int)*4 );
    unsigned int numVolumes = numBytes/size ;
    setNumVolumes(numVolumes);
}




void GMesh::setInstancedIdentityBuffer(NPY<unsigned int>* buffer) 
{
    assert(buffer); 
    m_iidentity_buffer = buffer ;  
    if(!buffer) return ; 

    LOG(LEVEL) << buffer->getShapeString() ; 
    m_iidentity = (guint4*)buffer->getPointer();


}



#ifdef WITH_COMPONENT 
NPY<unsigned>*  GMesh::getComponentsBuffer() const 
{
    return m_components_buffer ;
}
void GMesh::setComponentsBuffer(NPY<unsigned>* buf)
{
    m_components_buffer = buf ;
}
int GMesh::getNumComponents() const 
{
    return m_components_buffer ? m_components_buffer->getShape(0) : -1 ; 
}
void GMesh::getComponent( glm::uvec4& eidx, unsigned icomp ) const 
{
     eidx = m_components_buffer->getQuad_(icomp);
}
void GMesh::setComponent(const glm::uvec4& eidx, unsigned icomp )
{
    assert( m_num_mergedmesh > 0 && "MUST GMergedMesh::countMergedMesh before GMesh::setComponent ");
    if(!m_components_buffer) 
    {
         NPY<unsigned>* comp = NPY<unsigned>::make(m_num_mergedmesh, 4);
         comp->zero();
         setComponentsBuffer(comp); 
    }
    assert( icomp < m_num_mergedmesh );
    m_components_buffer->setQuad(eidx, icomp );
}
 
void GMesh::dumpComponents(const char* msg) const 
{
    LOG(info) << msg 
              << " numComponents " << getNumComponents() 
               ;

    if(getNumComponents() < 1 ) return ;
    unsigned num_comp = getNumComponents();
    for(unsigned icomp=0 ; icomp < num_comp ; icomp++)
    {
         glm::uvec4 eidx ;
         getComponent( eidx, icomp );
         std::cout << std::setw(4) << icomp
                   << gpresent(eidx)
                   << std::endl 
                   ;

    }

}
#else
int GMesh::getNumComponents() const 
{
    return -1 ; 
}
#endif







void GMesh::setNumVolumes(unsigned int numVolumes)
{
    if(m_num_volumes == 0) 
    {
        m_num_volumes = numVolumes ; 
    }
    else
    {
        assert(numVolumes == m_num_volumes);
    }
}


void GMesh::setTexcoords(gfloat2* texcoords)
{
    if(!texcoords) return ;
    m_texcoords = texcoords ;
    m_texcoords_buffer = new GBuffer( sizeof(gfloat2)*m_num_vertices, (void*)m_texcoords, sizeof(gfloat2), 2 , "texcoords") ;
    assert(sizeof(gfloat2) == sizeof(float)*2);
}
void GMesh::setTexcoordsBuffer(GBuffer* buffer)
{
    m_texcoords_buffer = buffer ; 
    if(!buffer) return ; 

    m_texcoords = (gfloat2*)buffer->getPointer(); 
    unsigned int numBytes = buffer->getNumBytes();
    unsigned int num_texcoords = numBytes/sizeof(gfloat2);
    assert( m_num_vertices == num_texcoords );  // must load vertices before texcoords
}


void GMesh::setFaces(guint3* faces)
{
    assert(sizeof(guint3) == 3*4);
    unsigned int totbytes = sizeof(guint3)*m_num_faces ;
    unsigned int itemsize = sizeof(guint3)/3 ;           // this regards the item as the individual index integer
    unsigned int nelem    = 1 ;                          // number of elements in the item

    m_faces = faces ;
    m_indices_buffer = new GBuffer( totbytes, (void*)m_faces, itemsize, nelem , "indices") ;
    assert(sizeof(guint3) == sizeof(unsigned int)*3);
}
void GMesh::setIndicesBuffer(GBuffer* buffer)
{
    m_indices_buffer = buffer ; 
    if(!buffer) return ;

    m_faces = (guint3*)buffer->getPointer();
    unsigned int numBytes = buffer->getNumBytes();
    m_num_faces = numBytes/sizeof(guint3);    // NB kludge equating "int" buffer to "unsigned int" 
}


void GMesh::setNodes(unsigned* nodes)   // only makes sense to use from single subclasses instances like GMergedMesh 
{
    m_nodes = nodes ;
    m_nodes_buffer = new GBuffer( sizeof(unsigned int)*m_num_faces, (void*)m_nodes, sizeof(unsigned int), 1 , "nodes") ;
    assert(sizeof(unsigned int) == sizeof(unsigned int)*1);
}
void GMesh::setNodesBuffer(GBuffer* buffer)
{
    m_nodes_buffer = buffer ; 
    if(!buffer) return ;

    m_nodes = (unsigned int*)buffer->getPointer();
    unsigned int numBytes = buffer->getNumBytes();
    unsigned int num_nodes = numBytes/sizeof(unsigned int);

    // assert(m_num_faces == num_nodes);   // must load indices before nodes

    bool face_node_mismatch = m_num_faces != num_nodes ;  
    LOG_IF(warning, face_node_mismatch ) << "GMesh::setNodesBuffer allowing face_node_mismatch  " ; 
}


void GMesh::setBoundaries(unsigned* boundaries)
{
    m_boundaries = boundaries ;
    m_boundaries_buffer = new GBuffer( sizeof(unsigned)*m_num_faces, (void*)m_boundaries, sizeof(unsigned), 1 , "boundaries") ;
    assert(sizeof(unsigned) == sizeof(unsigned)*1);
}
void GMesh::setBoundariesBuffer(GBuffer* buffer)
{
    m_boundaries_buffer = buffer ; 
    if(!buffer) return ;

    m_boundaries = (unsigned int*)buffer->getPointer();

    unsigned int numBytes = buffer->getNumBytes();
    unsigned int num_boundaries = numBytes/sizeof(unsigned int);

    // assert(m_num_faces == num_boundaries);   // must load indices before boundaries, for m_num_faces
    bool face_boundary_mismatch = m_num_faces != num_boundaries ; 
    LOG_IF(warning, face_boundary_mismatch) << "GMesh::setBoundariesBuffer allowing face_boundary_mismatch " ; 
}



void GMesh::setSensors(unsigned* sensors)
{
    m_sensors = sensors ;
    m_sensors_buffer = new GBuffer( sizeof(unsigned)*m_num_faces, (void*)m_sensors, sizeof(unsigned), 1 , "sensors") ;
    assert(sizeof(unsigned) == sizeof(unsigned)*1);
}
void GMesh::setSensorsBuffer(GBuffer* buffer)
{
    m_sensors_buffer = buffer ; 
    if(!buffer) return ;

    m_sensors = (unsigned*)buffer->getPointer();

    unsigned numBytes = buffer->getNumBytes();
    unsigned num_sensors = numBytes/sizeof(unsigned);
    assert(m_num_faces == num_sensors);   // must load indices before sensors, for m_num_faces
}




void GMesh::setColor(float r, float g, float b)
{
    //assert(m_num_colors == m_num_vertices);
    if(!m_colors)
    {
        setColors(new gfloat3[m_num_vertices]);
    }
    for(unsigned int i=0 ; i<m_num_vertices ; ++i )
    {
        m_colors[i].x  = r ;
        m_colors[i].y  = g ;
        m_colors[i].z  = b ;
    }
}

void GMesh::dumpNormals(const char* msg, unsigned int nmax) const 
{
    LOG(info) << msg  ;
    LOG(info) << " num_vertices " << m_num_vertices 
              ;  

    for(unsigned int i=0 ; i < std::min(nmax,m_num_vertices) ; i++)
    {
        gfloat3& nrm = m_normals[i] ;
        printf(" nrm %5u  %10.3f %10.3f %10.3f \n", i, nrm.x, nrm.y, nrm.z );
    } 
}

void GMesh::dump(const char* msg, unsigned int nmax) const 
{
    LOG(info) << msg  
              << " num_vertices " << m_num_vertices 
              << " num_faces " << m_num_faces
              << " num_volumes " << m_num_volumes
              << " name " << ( m_name ? m_name : "-" )
              ;  

    std::cout << " low  " << (m_low ?  m_low->desc() : "-" ) << std::endl ; 
    std::cout << " high " << (m_high ? m_high->desc() : "-" ) << std::endl ; 
    std::cout << " dim  " << (m_dimensions ? m_dimensions->desc() : "-" ) << std::endl ; 
    std::cout << " cen  " << (m_center ? m_center->desc() : "-" ) << " extent " << m_extent << std::endl ; 
    std::cout << " ce   " << (m_center_extent ? m_center_extent->desc() : "-" )  << std::endl ; 

    if(m_bbox)
    {
        std::cout << " bb.max   " << m_bbox->max.desc()  << std::endl ; 
        std::cout << " bb.min   " << m_bbox->min.desc()  << std::endl ; 
    }


    for(unsigned int i=0 ; i < std::min(nmax,m_num_vertices) ; i++)
    {
        gfloat3& vtx = m_vertices[i] ;
        gfloat3& nrm = m_normals[i] ;
        std::cout << std::setw(5) << i 
                  << " vtx " << vtx.desc() 
                  << " nrm " << nrm.desc() 
                  << std::endl ; 

    } 
    std::cout << std::endl ;   

    if(hasTexcoords())
    { 
        for(unsigned int i=0 ; i < std::min(nmax,m_num_vertices) ; i++)
        {
            gfloat2& tex = m_texcoords[i] ;
            printf(" tex %5u  %10.3f %10.3f  \n", i, tex.u, tex.v );
        } 
    }


    if(m_colors)
    {
        for(unsigned int i=0 ; i < std::min(nmax,m_num_vertices) ; i++)
        {
            gfloat3& col = m_colors[i] ;
            printf(" col %5u  %10.3f %10.3f %10.3f \n", i, col.x, col.y, col.z );
        }
    } 


    LOG(info) << " num_faces " << m_num_faces ;  

    for(unsigned int i=0 ; i < std::min(nmax,m_num_faces) ; i++)
    {
        guint3& fac = m_faces[i] ;
        printf(" fac %5u  %5u %5u %5u \n", i, fac.x, fac.y, fac.z );
    } 

    if(m_nodes && m_boundaries)
    {
        for(unsigned int i=0 ; i < std::min(nmax,m_num_faces) ; i++)
        {
            unsigned int& node = m_nodes[i] ;
            unsigned int& boundary = m_boundaries[i] ;
            printf(" fac %5u  node %5u boundary %5u  \n", i, node, boundary );
        } 
    }

}




std::string GMesh::desc() const  
{
    std::stringstream ss ; 
    unsigned nv = getNumVertices();
    unsigned nf = getNumFaces();
    //unsigned nc = getNumColors();
    ss 
        << " nv " << std::setw(6) << nv 
        << " nf " << std::setw(6) << nf
      //  << " nc " << std::setw(6) << nc
        ;

    return ss.str();
}

void GMesh::Summary(const char* msg) const 
{
   LOG(info) << msg ;  

   printf("%s idx %u vx %u fc %u n %s sn %s \n",
      msg, 
      m_index, 
      m_num_vertices, 
      m_num_faces,
      m_name,
      m_shortname
   );

   if(m_low) printf("%10s %10.3f %10.3f %10.3f\n",
         "low",
         m_low->x,
         m_low->y,
         m_low->z);

   if(m_high) printf("%10s %10.3f %10.3f %10.3f\n", 
          "high",
          m_high->x,
          m_high->y,
          m_high->z);

   if(m_dimensions) printf("%10s %10.3f %10.3f %10.3f extent %10.3f\n", 
          "dimen",
          m_dimensions->x,
          m_dimensions->y,
          m_dimensions->z,
          m_extent);

   if(m_center) printf("%10s %10.3f %10.3f %10.3f\n", 
          "center",
          m_center->x,
          m_center->y,
          m_center->z);

   if(m_center_extent) printf("%10s %10.3f %10.3f %10.3f %10.3f \n", 
          "center_extent",
          m_center_extent->x,
          m_center_extent->y,
          m_center_extent->z,
          m_center_extent->w);

   //m_model_to_world->Summary(msg);
}





nbbox* GMesh::findBBox_(gfloat3* vertices, unsigned int num_vertices) // static
{
    std::vector<glm::vec3> points ; 
    for( unsigned int i = 0; i < num_vertices ;++i )
    {
        gfloat3& v = vertices[i];
        glm::vec3 p(v.x,v.y,v.z);
        points.push_back(p);
    }
    unsigned verbosity = 0 ;  
    nbbox nbb = nbbox::from_points(points, verbosity);
    nbbox* bb = new nbbox(nbb);
    return bb;
} 

gbbox* GMesh::findBBox(gfloat3* vertices, unsigned int num_vertices) // static
{
    if(num_vertices == 0) return NULL ;
    nbbox* nbb = GMesh::findBBox_(vertices, num_vertices); 
    gbbox* bb = new gbbox(*nbb);
    return bb ; 
} 



gfloat4 GMesh::findCenterExtentDeprecated(gfloat3* vertices, unsigned int num_vertices)
{
    gfloat3  low( 1e10f, 1e10f, 1e10f);
    gfloat3 high( -1e10f, -1e10f, -1e10f);

    for( unsigned int i = 0; i < num_vertices ;++i )
    {
        gfloat3& v = vertices[i];

        low.x = std::min( low.x, v.x);
        low.y = std::min( low.y, v.y);
        low.z = std::min( low.z, v.z);

        high.x = std::max( high.x, v.x);
        high.y = std::max( high.y, v.y);
        high.z = std::max( high.z, v.z);
    }

    gfloat3 dimensions(high.x - low.x, high.y - low.y, high.z - low.z );
    float extent = 0.f ;
    extent = std::max( dimensions.x , extent );
    extent = std::max( dimensions.y , extent );
    extent = std::max( dimensions.z , extent );
    extent = extent / 2.0f ;         
 
    gfloat4 center_extent((high.x + low.x)/2.0f, (high.y + low.y)/2.0f , (high.z + low.z)/2.0f, extent );

    return center_extent ; 
}


/**
GMesh::updatebounds
--------------------

Updates the below based on the bounding box of the vertices.
The vertices present depend on the geometry selection 
when the GMergedMesh was created from the GMesh of the GVolume. 

::

    m_center_extent[0] 
    m_bbox[0]
    m_model_to_world


updateBounds is invoked from::

   GMesh::init
   GMesh::loadBuffers
   GGeo::invokeMeshJoin
   GMergedMesh::combine
   GMergedMesh::create
 
TODO-SOMETIME: 

Avoid the need for updating as it is a source of confusion
arising from double use of slot zero for:

1. absolute volume 0 (world volume, an often over large container)  
2. overall selected volumes 


model coordinates definition
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

* all vertices are contained within model coordinates box  (-1:1,-1:1,-1:1) 
* model coordinates origin (0,0,0) corresponds to world coordinates  m_center
* extent is half the maximal dimension 

world -> model
   
* translate by -m_center 
* scale by 1/m_extent

model -> world

* scale by m_extent
* translate by m_center


TO FIX 
~~~~~~~~~~~~

This avoid stomping on position of array of center_extent in case of MergedMesh, 
but it still overwrites volume 0 


**/

void GMesh::updateBounds()
{
    gbbox*  bb = findBBox(m_vertices, m_num_vertices);

    gfloat4 ce(0,0,0,1.f) ;
    if(bb) ce = bb->center_extent() ;

    LOG(debug) << " ce " <<  ce.desc(); 

    m_model_to_world = new GMatrix<float>( ce.x, ce.y, ce.z, ce.w );

    if(bb)
    {
        if(m_bbox == NULL)
        {
            m_bbox = new gbbox(*bb) ;
        }
        else
        {
            m_bbox[0].min = bb->min ;
            m_bbox[0].max = bb->max ;
        }
    }

    if(m_center_extent == NULL)
    {
        m_center_extent = new gfloat4( ce.x, ce.y, ce.z, ce.w );
    }
    else
    {
        LOG(debug)
            << "(to bbox of selected vertices) "
            << " overwrite volume 0 ce "
            <<  m_center_extent[0].description()
            << " with " 
            << ce.description()
            << " num_vertices " << m_num_vertices
            << " num_volumes " << m_num_volumes
            ;

        m_center_extent[0].x = ce.x ;
        m_center_extent[0].y = ce.y ;
        m_center_extent[0].z = ce.z ;
        m_center_extent[0].w = ce.w ;
    }
}



void GMesh::updateBounds(gfloat3& low, gfloat3& high, GMatrixF& transform)
{
    if(m_low && m_high)
    {   
        gfloat3 mlow(*m_low) ; 
        gfloat3 mhigh(*m_high) ; 

        mlow  *= transform ; 
        mhigh *= transform ; 

        low.x = std::min( low.x, mlow.x);
        low.y = std::min( low.y, mlow.y);
        low.z = std::min( low.z, mlow.z);

        high.x = std::max( high.x, mhigh.x);
        high.y = std::max( high.y, mhigh.y);
        high.z = std::max( high.z, mhigh.z);
   }
}



glm::vec4 GMesh::getVertex(unsigned i) const 
{
    assert( i < m_num_vertices ); 
    glm::vec4 vtx(0.,0.,0.,1.); 
    vtx.x = m_vertices[i].x ; 
    vtx.y = m_vertices[i].y ;
    vtx.z = m_vertices[i].z ;
    return vtx ; 
}



void GMesh::getTransformedVertices(std::vector<glm::vec4>& tvertices, const glm::mat4& transform )
{
    for(unsigned i = 0; i < m_num_vertices; i++)
    {  
        glm::vec4 a = getVertex(i); 
        glm::vec4 b = a * transform ;  // TODO: verify order
        tvertices.push_back(b); 
    }   
}

gfloat3* GMesh::getTransformedVertices(GMatrixF& transform ) const 
{
     gfloat3* vertices = new gfloat3[m_num_vertices];
     for(unsigned i = 0; i < m_num_vertices; i++)
     {  
         vertices[i].x = m_vertices[i].x ;   
         vertices[i].y = m_vertices[i].y ;   
         vertices[i].z = m_vertices[i].z ;   

         vertices[i] *= transform ;
     }   
     return vertices ;
}

gfloat3* GMesh::getTransformedNormals(GMatrixF& transform ) const 
{
     gfloat3* normals = new gfloat3[m_num_vertices];
     for(unsigned int i = 0; i < m_num_vertices; i++)
     {  
         gfloat4 nrm(m_normals[i], 0.);   // w=0 as direction, not position 

         nrm *= transform ; 
         // NB should be transpose of inverse, 
         // (so only OK if orthogonal, that means rotations only no non-uniform scaling)

         normals[i].x = nrm.x ; 
         normals[i].y = nrm.y ; 
         normals[i].z = nrm.z ; 

     }   
     return normals ;
}



void GMesh::updateDistinctBoundaries()
{
    for(unsigned int i=0 ; i < getNumFaces() ; i++)
    {
        unsigned int index = m_boundaries[i] ;
        if(std::count(m_distinct_boundaries.begin(), m_distinct_boundaries.end(), index ) == 0) m_distinct_boundaries.push_back(index);
    }  
    std::sort( m_distinct_boundaries.begin(), m_distinct_boundaries.end() );
}
 
std::vector<unsigned int>& GMesh::getDistinctBoundaries()
{
    if(m_distinct_boundaries.size()==0) updateDistinctBoundaries();
    return m_distinct_boundaries ;
}







bool GMesh::isFloatBuffer(const char* name) const 
{

    return ( strcmp( name, vertices_) == 0 || 
             strcmp( name, normals_) == 0  || 
             strcmp( name, center_extent_) == 0  || 
             strcmp( name, bbox_) == 0  || 
             strcmp( name, transforms_) == 0  || 
             strcmp( name, colors_) == 0 );
}

bool GMesh::isIntBuffer(const char* name) const 
{ 
    return ( 
             strcmp( name, indices_) == 0     || 
             strcmp( name, nodes_) == 0       || 
             strcmp( name, sensors_) == 0     || 
             strcmp( name, boundaries_) == 0 
          );
}
bool GMesh::isUIntBuffer(const char* name) const 
{
    return 
           ( 
              strcmp( name, nodeinfo_) == 0  ||
              strcmp( name, identity_) == 0  ||
              strcmp( name, iidentity_) == 0  ||
              strcmp( name, meshes_) == 0  
           );
}


bool GMesh::isNPYBuffer(const char* name)  const 
{
    return 
           ( 
              strcmp( name, iidentity_) == 0  ||
#ifdef WITH_COMPONENT
              strcmp( name, components_) == 0  ||
#endif 
              strcmp( name, itransforms_) == 0  
           );
}


void GMesh::saveBuffer(const char* path, const char* name, GBuffer* buffer) const 
{
    LOG(debug) << "GMesh::saveBuffer "
               << " name " << std::setw(25) << name 
               << " path " << path  
               ;

    if(isNPYBuffer(name))  
    {
         saveNPYBuffer(path, name);
    }
    else if(buffer != NULL)
    {
        if(isFloatBuffer(name))     buffer->save<float>(path);
        else if(isIntBuffer(name))  buffer->save<int>(path);
        else if(isUIntBuffer(name)) buffer->save<unsigned>(path);
        else 
           printf("GMesh::saveBuffer WARNING NOT saving uncharacterized buffer %s into %s \n", name, path );
    }
}

void GMesh::saveNPYBuffer(const char* path, const char* name) const 
{
    NPYBase* buf = getNPYBuffer(name);
    if(buf)
    {
        buf->save(path);
    }
    else
    {
        LOG(debug) << "GMesh::saveNPYBuffer"
                     << " NULL buffer not saving "
                     << " path " << path
                     << " name " << name 
        ;
    } 
}

NPYBase* GMesh::getNPYBuffer(const char* name) const 
{
    NPYBase* buf(NULL);
    if(strcmp(name, iidentity_) == 0)   buf = getInstancedIdentityBuffer();
    else if(strcmp(name, itransforms_) == 0) buf = getITransformsBuffer();
#ifdef WITH_COMPONENT
    else if(strcmp(name, components_) == 0)  buf = getComponentsBuffer();
#endif
    return buf ; 
}


void GMesh::loadNPYBuffer(const char* path, const char* name)
{

    LOG(LEVEL) << " name " << name << " path " << path ; 

    if(strcmp(name, iidentity_) == 0)
    {
        std::string rpath = Opticks::Instance()->formCacheRelativePath(path); 
        LOG(LEVEL) << rpath ; 
        NPY<unsigned>* buf = NPY<unsigned>::load(path) ;
        setInstancedIdentityBuffer(buf);
    }
    else if(strcmp(name, itransforms_) == 0)
    {
        NPY<float>* buf = NPY<float>::load(path) ;
        setITransformsBuffer(buf);
    }
#ifdef WITH_COMPONENT
    else if(strcmp(name, components_) == 0)
    {
        NPY<unsigned>* buf = NPY<unsigned>::load(path) ;
        setComponentsBuffer(buf);
    }
#endif
    else
    {
        LOG(fatal) << " unknown: " << name ; 
        assert(0);
    }
}

void GMesh::loadGBuffer(const char* path, const char* name)
{
    LOG(LEVEL) << " name " << name << " path " << path ; 

    GBuffer* buffer(NULL); 
    if(isFloatBuffer(name))                    buffer = GBuffer::load<float>(path);
    else if(isIntBuffer(name))                 buffer = GBuffer::load<int>(path);
    else if(isUIntBuffer(name))                buffer = GBuffer::load<unsigned int>(path);
    else LOG(fatal) << " unknown buffer " << name << " " << path ; 
    
    LOG_IF(fatal, !buffer) << " failed to load " << name << " " << path ; 
    assert(buffer); 
 
    if(buffer) setBuffer(name, buffer);
}


void GMesh::loadBuffer(const char* path, const char* name)
{
    if(isNPYBuffer(name))  // iidentity, itransforms, components
    {
         loadNPYBuffer(path, name);
    }
    else
    {
         loadGBuffer(path, name);
    }
}



std::vector<std::string>& GMesh::getNames()
{
    return m_names ; 
}

std::string GMesh::getVersionedBufferName(std::string& name) const 
{
    std::string vname = name ;
    if(m_version)
    {
        if(vname.compare("vertices") == 0 || 
           vname.compare("indices") == 0  || 
           vname.compare("colors") == 0  || 
           vname.compare("normals") == 0)
           { 
               vname += m_version ;
               LOG(warning) << "GMesh::loadBuffers version setting changed buffer name to " << vname ; 
           }
    }
    return vname ; 
}



GMesh* GMesh::load(const char* dir, const char* typedir, const char* instancedir)
{

    std::string cachedir = BFile::FormPath(dir, typedir, instancedir);
    bool existsdir = BFile::ExistsDir(dir, typedir, instancedir);

    LOG(debug) << "GMesh::load"
              << " dir " << dir 
              << " typedir " << typedir 
              << " instancedir " << instancedir 
              << " cachedir " << cachedir 
              << " existsdir " << existsdir
              ;
 


    GMesh* mesh(NULL);
    if(!existsdir)
    {
        LOG(error)  << "GMesh::load FAILED : NO DIRECTORY "
                    << " dir " << dir
                    << " typedir " << typedir
                    << " instancedir " << instancedir
                    << " -> cachedir " << cachedir
                    ;
    }
    else
    {
        mesh = new GMesh(0, NULL, 0, NULL, 0, NULL, NULL );
        mesh->loadBuffers(cachedir.c_str());
    }
    return mesh ; 
}


void GMesh::save(const char* dir, const char* typedir, const char* instancedir) const 
{
    LOG(LEVEL) << "[ instancedir " << instancedir  ; 

    std::string cachedir = BFile::CreateDir(dir, typedir, instancedir);

    if(!cachedir.empty())
     {
        const char* dir = cachedir.c_str() ;   
        saveBuffers(dir);
    }
    else 
    {
        LOG(error)  << "GMesh::save FAILED : NO DIRECTORY "
                    << " dir " << dir
                    << " typedir " << typedir
                    << " instancedir " << instancedir
                    << " -> cachedir " << cachedir 
                    ;
    }

    LOG(LEVEL) << "] instancedir " << instancedir  ; 
}


void GMesh::loadBuffers(const char* dir)
{
    LOG(LEVEL) << dir ;  

    for(unsigned int i=0 ; i<m_names.size() ; i++)
    {
        std::string name = m_names[i];
        std::string vname = getVersionedBufferName(name);  
        fs::path bufpath(dir);
        bufpath /= vname + ".npy" ; 

        if(fs::exists(bufpath) && fs::is_regular_file(bufpath))
        { 
            loadBuffer(bufpath.string().c_str(), name.c_str());
        }
        else
        {
            LOG(verbose) << "no such bufpath: " << bufpath ; 
        }
    } 
    updateBounds();
}


void GMesh::saveBuffers(const char* dir) const 
{
    LOG(LEVEL) << "[" ; 
    for(unsigned int i=0 ; i<m_names.size() ; i++)
    {
        std::string name = m_names[i];
        std::string vname = getVersionedBufferName(name);  
        fs::path bufpath(dir);
        bufpath /= vname + ".npy" ; 

        GBuffer* buffer = getBuffer(name.c_str());

        saveBuffer(bufpath.string().c_str(), name.c_str(), buffer);  
    } 
    LOG(LEVEL) << "]" ; 
}


GMesh* GMesh::makeDedupedCopy()
{
    GMeshFixer fixer(this);
    fixer.copyWithoutVertexDuplicates();   
    return fixer.getDst(); 
}


GMesh* GMesh::load_deduped(const char* dir, const char* typedir, const char* instancedir)
{
    LOG(verbose) << "GMesh::load_deduped"
               << " dir " << dir
               << " typedir " << typedir
               << " instancedir " << instancedir
               ;

    GMesh* gm = GMesh::load(dir, typedir, instancedir) ;

    if(!gm)
    {
         LOG(error) << "GMesh::load_deduped FAILED to load mesh"
               << " dir " << dir
               << " typedir " << typedir
               << " instancedir " << instancedir
               ;

         return NULL ; 
    }    

    GMesh* dm = gm->makeDedupedCopy();
    delete gm ; 
    return dm ; 
}

void GMesh::findShortName()
{
   if(!m_name) return ; 
   m_shortname = SStr::TrimPointerSuffix(m_name);   
}



/**
GMesh::getFaceCount
--------------------

Alternative check getting nodeinfo per-volume sum of faces.

**/

unsigned GMesh::getFaceCount() const 
{
    guint4* nodeinfo = getNodeInfo();
    unsigned numVolumes = getNumVolumes();
    unsigned int nftot(0);

    for(unsigned s=0 ; s < numVolumes ; s++)
    {
        unsigned nf = (nodeinfo + s)->x ;
        nftot += nf ;
    }
    return nftot ; 
}






/**
GMesh::getAppropriateRepeatedIdentityBuffer
---------------------------------------------

mmidx > 0 (FORMERLY: numITransforms > 0)
   friib : FaceRepeatedInstancedIdentityBuffer 

frib (FORMERLY: numITransforms == 0)
   frib :  FaceRepeatedIdentityBuffer


Sep 2020: moved to branching on mmidx > 0 as that 
matches the rest of the geometry conversion code.  
In anycase numITransforms is never zero. 
For global mmidx=0 it is always 1 (identity matrix). 
So was previously always returning friib.

**/

GBuffer*  GMesh::getAppropriateRepeatedIdentityBuffer()
{
    GMesh* mm = this ; 
    unsigned numITransforms = mm->getNumITransforms();
    unsigned numFaces = mm->getNumFaces();
    unsigned mmidx = mm->getIndex(); 

    GBuffer* id = NULL ;  

    if(mmidx > 0) 
    {
        id = mm->getFaceRepeatedInstancedIdentityBuffer(); 
        assert(id);
        LOG(LEVEL) << "using FaceRepeatedInstancedIdentityBuffer" << " friid items " << id->getNumItems() << " numITransforms*numFaces " << numITransforms*numFaces ;     
        assert( id->getNumItems() == numITransforms*numFaces );
    }
    else
    {
        id = mm->getFaceRepeatedIdentityBuffer();
        assert(id);
        LOG(LEVEL) << "using FaceRepeatedIdentityBuffer" << " frid items " << id->getNumItems() << " numFaces " << numFaces ;
        assert( id->getNumItems() == numFaces );
    }
    return id ; 
}




GBuffer*  GMesh::getFaceRepeatedInstancedIdentityBuffer()
{
    if(m_facerepeated_iidentity_buffer == NULL)
    {
         m_facerepeated_iidentity_buffer = makeFaceRepeatedInstancedIdentityBuffer() ;  
    }
    return m_facerepeated_iidentity_buffer ;
}

GBuffer*  GMesh::getFaceRepeatedIdentityBuffer()
{
    if(m_facerepeated_identity_buffer == NULL)
    {
         m_facerepeated_identity_buffer = makeFaceRepeatedIdentityBuffer() ;  
    }
    return m_facerepeated_identity_buffer ;
}



            
void GMesh::checks_faceRepeatedInstancedIdentity()
{
    unsigned numITransforms = getNumITransforms() ;
    unsigned mmidx = getIndex(); 
    LOG(info) << " mmidx " << mmidx  << " numITransforms " << numITransforms ; 

    assert( numITransforms > 0 && "zero should never happen anymore" ); 
    if( mmidx == 0 ) assert( numITransforms == 1 && "global mm0 should have only one ITransform, which is the idenitity matrix" ); 
    assert( mmidx > 0 && "GMesh::makeFaceRepeatedInstancedIdentityBuffer only relevant to instanced meshes " ); 

    unsigned numVolumes = getNumVolumes();
    unsigned numVolumesSelected = getNumVolumesSelected();   // NOT persisted
    unsigned numFaces = getNumFaces() ;
    unsigned numRepeatedIdentity = numITransforms*numFaces ;   // total faces for all occurrences of this GMergedMesh instance


    unsigned numInstanceIdentity = m_iidentity_buffer->getShape(0)*m_iidentity_buffer->getShape(1) ;  

    /*
    recent change in ii shape for globals
    */

    LOG(LEVEL)
        << "\n m_index " << m_index
        << "\n numITransforms " << numITransforms
        << "\n numVolumes " << numVolumes
        << "\n numVolumesSelected " << numVolumesSelected
        << "\n numFaces " << numFaces
        << "\n numRepeatedIdentity (numITransforms*numFaces) " << numRepeatedIdentity
        << "\n numInstanceIdentity " << numInstanceIdentity
        << "\n m_iidentity_buffer " << m_iidentity_buffer->getShapeString()
        << "\n m_itransforms_buffer " << m_itransforms_buffer->getShapeString()
        ;

    bool nodeinfo_ok = m_nodeinfo_buffer && m_nodeinfo_buffer->getNumItems() == numVolumes ;
    bool iidentity_ok = m_iidentity_buffer && numInstanceIdentity == numVolumes*numITransforms ;

    LOG_IF(fatal, !nodeinfo_ok) 
        << "\n nodeinfo_ok " << nodeinfo_ok
        << "\n nodeinfo_buffer_items " << ( m_nodeinfo_buffer ? m_nodeinfo_buffer->getNumItems() : -1 )
        << "\n numVolumes " << numVolumes  
        ;

    LOG_IF(fatal, !iidentity_ok) 
        << "\n iidentity_ok " << iidentity_ok
        << "\n iidentity_buffer_items " << ( m_iidentity_buffer ? m_iidentity_buffer->getNumItems() : -1 )
        << "\n numFaces (sum of faces in numVolumes)" << numFaces 
        << "\n numVolumes " << numVolumes
        << "\n numITransforms " << numITransforms
        << "\n numVolumes*numITransforms " << numVolumes*numITransforms 
        << "\n numInstanceIdentity " << numInstanceIdentity << " (expected to equal the above) " 
        << "\n numRepeatedIdentity " << numRepeatedIdentity 
        << "\n m_iidentity_buffer " << m_iidentity_buffer->getShapeString()
        << "\n m_itransforms_buffer " << m_itransforms_buffer->getShapeString()
        ; 

    assert(nodeinfo_ok);
    assert(iidentity_ok); 


    // check nodeinfo per-volume sum of faces matches expected total 
    unsigned nftot = getFaceCount(); 
    LOG(info) << " nftot " << nftot << " numFaces " << numFaces ; 
    assert( numFaces == nftot );
}




/**
GMesh::makeFaceRepeatedInstancedIdentityBuffer
-----------------------------------------------------

Canonically invoked by optixrap-/OGeo::makeTriangulatedGeometry
Constructing a face repeated IIdentity buffer
to be addressed with 0:numInstances*PrimitiveCount::

   instanceIdx*PrimitiveCount + primIdx ;

where the primIdx goes over all the volumes 

**/

GBuffer* GMesh::makeFaceRepeatedInstancedIdentityBuffer()
{ 
    checks_faceRepeatedInstancedIdentity();

    unsigned numITransforms = getNumITransforms() ;
    unsigned numVolumes = getNumVolumes();
    unsigned numFaces = getNumFaces() ;                          // from sum over all GMesh(solids) within the GMergedMesh 
    unsigned numRepeatedIdentity = numITransforms*numFaces ;     // total faces for all occurrences of this GMergedMesh instance

    guint4* nodeinfo = getNodeInfo();
    guint4* riid = new guint4[numRepeatedIdentity] ;
    
    unsigned offset(0);
    for(unsigned i=0 ; i < numITransforms ; i++)
    {
        unsigned instance_index = i ; 
        offset = 0 ; 
        for(unsigned s=0 ; s < numVolumes ; s++)
        {   
            unsigned volume_index = s ;  
            guint4 iid = m_iidentity[numVolumes*instance_index + volume_index]  ;  

            unsigned nf = (nodeinfo + volume_index)->x ;    // faces for the volume 
            for(unsigned f=0 ; f < nf ; ++f) 
            {
                riid[instance_index*numFaces+offset+f] = iid ; 
            }
            offset += nf ; 
        }  
    }   
    
    unsigned int size = sizeof(guint4) ;
    GBuffer* buffer = new GBuffer( size*numRepeatedIdentity, (void*)riid, size, 4 , "friib" ); 
    return buffer ; 
}

/**
GMesh::makeFaceRepeatedIdentityBuffer
-------------------------------------

For non-instanced GMergedMesh mm0, duplicate m_identity for 
each volume of the GMergedMesh out to each face.

**/

GBuffer* GMesh::makeFaceRepeatedIdentityBuffer()
{
    unsigned mmidx = getIndex(); 
    unsigned numITransforms = getNumITransforms() ;
    unsigned numVolumes = getNumVolumes();
    unsigned numFaces = getNumFaces() ;
    unsigned numFacesCheck = getFaceCount(); 

    LOG(info) 
        << " mmidx " << mmidx
        << " numITransforms " << numITransforms
        << " numVolumes " << numVolumes 
        << " numFaces (sum of faces in numVolumes)" << numFaces 
        << " numFacesCheck " << numFacesCheck
        ; 

    assert( mmidx == 0 );  
    assert( numITransforms == 1 && "GMesh::makeFaceRepeatedIdentityBuffer only relevant to the non-instanced mm0 "); 
    assert( m_nodeinfo_buffer->getNumItems() == numVolumes);
    assert( numFaces == numFacesCheck );   // check nodeinfo sum of per-volume faces matches expectation

    guint4* nodeinfo = getNodeInfo();
    guint4* rid = new guint4[numFaces] ;

    unsigned offset(0);
    for(unsigned s=0 ; s < numVolumes ; s++)
    {   
        guint4 sid = m_identity[s]  ;  
        unsigned int nf = (nodeinfo + s)->x ;
        for(unsigned f=0 ; f < nf ; ++f)
        {
            rid[offset+f] = sid ; 
        }
        offset += nf ; 
    }  
    
    unsigned int size = sizeof(guint4) ;
    GBuffer* buffer = new GBuffer( size*numFaces, (void*)rid, size, 4 , "frib" ); 
    return buffer ; 
}






void GMesh::explodeZVertices(float zoffset, float zcut)
{
    unsigned int noffset(0);
    for(unsigned int i=0 ; i < m_num_vertices ; i++)
    {
        gfloat3* v = m_vertices + i  ;
        if( v->z > zcut )
        {
            noffset += 1 ; 
            v->z += zoffset ; 
            printf("GMesh::explodeZVertices %6d : %10.4f %10.4f %10.4f \n", i, v->x, v->y, v->z );
        }
    }
    LOG(info) << "GMesh::explodeZVertices" 
              << " noffset " << noffset 
              << " zoffset " << zoffset 
              << " zcut " << zcut 
              ;
}


/**
GMesh::findContainer
---------------------

Find volumes that contains the point
returning the index of the volume with the smallest extent 
or 0 if none found.

Caution when using mm0 as the old practice of keeping 
all volumes in mm0 is going to change see notes/issues/geometry_model_review.rst

**/

unsigned GMesh::findContainer(gfloat3 p)
{
    unsigned container(0);
    float cext(FLT_MAX) ; 

    for(unsigned index=0 ; index < m_num_volumes ; index++)
    {
         gfloat4 ce = m_center_extent[index] ;
         gfloat3 hi(ce.x + ce.w, ce.y + ce.w, ce.z + ce.w );
         gfloat3 lo(ce.x - ce.w, ce.y - ce.w, ce.z - ce.w );

         if( 
              p.x > lo.x && p.x < hi.x  &&
              p.y > lo.y && p.y < hi.y  &&
              p.z > lo.z && p.z < hi.z 
           )
          {
               //printf("GMesh::findContainer %d   %10.4f %10.4f %10.4f %10.4f  \n", index, ce.x, ce.y, ce.z, ce.w  );
               if(ce.w < cext)
               {
                   cext = ce.w ; 
                   container = index ; 
               }
          }
    }
    return container ; 
}







template <typename T>
void GMesh::setMeta(const char* key, T value)
{
    assert( m_itransforms_buffer ); 
    m_itransforms_buffer->setMeta<T>(key, value);
}
template <typename T>
T GMesh::getMeta(const char* key, const char* fallback) const 
{
    assert( m_itransforms_buffer ); 
    return m_itransforms_buffer->getMeta<T>(key, fallback);
}



template GGEO_API void GMesh::setMeta(const char* key, int value);
template GGEO_API void GMesh::setMeta(const char* key, float value);
template GGEO_API void GMesh::setMeta(const char* key, std::string value);

template GGEO_API int GMesh::getMeta(const char* key, const char* fallback) const ;
template GGEO_API float GMesh::getMeta(const char* key, const char* fallback) const ;
template GGEO_API std::string GMesh::getMeta(const char* key, const char* fallback) const ;





 
