#include "OptiXAssimpGeometry.hh"
#include "OptiXProgram.hh"

#include <string.h>
#include <stdlib.h>
#include <sstream>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/material.h>
#include <assimp/postprocess.h>

#include <optixu/optixu_vector_types.h>



OptiXAssimpGeometry::OptiXAssimpGeometry(const char* path, const char* query )
           : 
           AssimpGeometry(path, query),
           m_context(NULL),
           m_program(NULL)
{
}

OptiXAssimpGeometry::~OptiXAssimpGeometry()
{
}


void OptiXAssimpGeometry::setContext(optix::Context& context)
{
    m_context = context ;   
}

void OptiXAssimpGeometry::setProgram(OptiXProgram* program)
{
    m_program = program ;   
}




void OptiXAssimpGeometry::convert()
{
    for(unsigned int i = 0; i < m_aiscene->mNumMaterials; i++)
    {
        optix::Material material = convertMaterial(m_aiscene->mMaterials[i]);
        m_materials.push_back(material);
    }

    for(unsigned int i = 0; i < m_aiscene->mNumMeshes; i++)
    {
        optix::Geometry geometry = convertGeometry(m_aiscene->mMeshes[i]);
        m_geometries.push_back(geometry);
    }



    aiNode* node = searchNode(m_query); 
    if(!node){
        printf("failed to find node %s \n", m_query );
        node = m_aiscene->mRootNode ;
    } 


    optix::GeometryGroup top = convertNode(node);

    optix::Acceleration acceleration = m_context->createAcceleration("Sbvh", "Bvh");
    acceleration->setProperty( "vertex_buffer_name", "vertexBuffer" );
    acceleration->setProperty( "index_buffer_name", "indexBuffer" );
    top->setAcceleration( acceleration );
    acceleration->markDirty();

    m_context["top_object"]->set(top);
}



optix::Material OptiXAssimpGeometry::convertMaterial(aiMaterial* ai_material)
{
    /*
    TODO:
        get assimp to access wavelength dependant material properties
        and feed them through into the material program 
        * by code gen of tables ? referencing a buffer of structs ?
    */

    optix::Material material = m_context->createMaterial();
    const char* filename = "material1.cu" ; 
    const char* fname = "closest_hit_radiance" ; 
    optix::Program  program = m_program->createProgram(filename, fname);
    material->setClosestHitProgram(0, program);
    material["Kd"]->setFloat( 0.7f, 0.7f, 0.7f);
    return material ; 
}


optix::Geometry OptiXAssimpGeometry::convertGeometry(aiMesh* mesh)
{
    unsigned int numFaces = mesh->mNumFaces;
    unsigned int numVertices = mesh->mNumVertices;

    optix::Geometry geometry = m_context->createGeometry();

    geometry->setPrimitiveCount(numFaces);

    const char* filename = "TriangleMesh.cu" ;   // cached program is returned after first  
    optix::Program intersectionProgram = m_program->createProgram( filename, "mesh_intersect" );
    optix::Program boundingBoxProgram = m_program->createProgram( filename, "mesh_bounds" );

    geometry->setIntersectionProgram(intersectionProgram);
    geometry->setBoundingBoxProgram(boundingBoxProgram);

    // Create vertex, normal and texture buffer

    optix::Buffer vertexBuffer = m_context->createBuffer( RT_BUFFER_INPUT, RT_FORMAT_FLOAT3, numVertices);
    optix::float3* vertexBuffer_Host = static_cast<optix::float3*>( vertexBuffer->map() );

    optix::Buffer normalBuffer = m_context->createBuffer( RT_BUFFER_INPUT, RT_FORMAT_FLOAT3, numVertices);
    optix::float3* normalBuffer_Host = static_cast<optix::float3*>( normalBuffer->map() );

    geometry["vertexBuffer"]->setBuffer(vertexBuffer);
    geometry["normalBuffer"]->setBuffer(normalBuffer);

    // Copy vertex and normal buffers

    memcpy( static_cast<void*>( vertexBuffer_Host ),
        static_cast<void*>( mesh->mVertices ),
        sizeof( optix::float3 )*numVertices); 
    vertexBuffer->unmap();

    memcpy( static_cast<void*>( normalBuffer_Host ),
        static_cast<void*>( mesh->mNormals),
        sizeof( optix::float3 )*numVertices); 
    normalBuffer->unmap();

    // Transfer texture coordinates to buffer
    optix::Buffer texCoordBuffer;
    if(mesh->HasTextureCoords(0))
    {
        texCoordBuffer = m_context->createBuffer( RT_BUFFER_INPUT, RT_FORMAT_FLOAT2, numVertices);
        optix::float2* texCoordBuffer_Host = static_cast<optix::float2*>( texCoordBuffer->map());
        for(unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            aiVector3D texCoord = (mesh->mTextureCoords[0])[i];
            texCoordBuffer_Host[i].x = texCoord.x;
            texCoordBuffer_Host[i].y = texCoord.y;
        }
        texCoordBuffer->unmap();
    }
    else
    {
        texCoordBuffer = m_context->createBuffer( RT_BUFFER_INPUT, RT_FORMAT_FLOAT2, 0);
    }

    geometry["texCoordBuffer"]->setBuffer(texCoordBuffer);

    // Tangents and bi-tangents buffers

    geometry["hasTangentsAndBitangents"]->setUint(mesh->HasTangentsAndBitangents() ? 1 : 0);
    if(mesh->HasTangentsAndBitangents())
    {
        optix::Buffer tangentBuffer = m_context->createBuffer( RT_BUFFER_INPUT, RT_FORMAT_FLOAT3, numVertices);
        optix::float3* tangentBuffer_Host = static_cast<optix::float3*>( tangentBuffer->map() );
        memcpy( static_cast<void*>( tangentBuffer_Host ),
            static_cast<void*>( mesh->mTangents),
            sizeof( optix::float3 )*numVertices); 
        tangentBuffer->unmap();

        optix::Buffer bitangentBuffer = m_context->createBuffer( RT_BUFFER_INPUT, RT_FORMAT_FLOAT3, numVertices);
        optix::float3* bitangentBuffer_Host = static_cast<optix::float3*>( bitangentBuffer->map() );
        memcpy( static_cast<void*>( bitangentBuffer_Host ),
            static_cast<void*>( mesh->mBitangents),
            sizeof( optix::float3 )*numVertices); 
        bitangentBuffer->unmap();

        geometry["tangentBuffer"]->setBuffer(tangentBuffer);
        geometry["bitangentBuffer"]->setBuffer(bitangentBuffer);
    }
    else
    {
        optix::Buffer emptyBuffer = m_context->createBuffer(RT_BUFFER_INPUT_OUTPUT, RT_FORMAT_FLOAT3, 0);
        geometry["tangentBuffer"]->setBuffer(emptyBuffer);
        geometry["bitangentBuffer"]->setBuffer(emptyBuffer);
    }

    // Create index buffer

    optix::Buffer indexBuffer = m_context->createBuffer( RT_BUFFER_INPUT, RT_FORMAT_INT3, numFaces );
    optix::int3* indexBuffer_Host = static_cast<optix::int3*>( indexBuffer->map() );
    geometry["indexBuffer"]->setBuffer(indexBuffer);

    // Copy index buffer from host to device

    for(unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        indexBuffer_Host[i].x = face.mIndices[0];
        indexBuffer_Host[i].y = face.mIndices[1];
        indexBuffer_Host[i].z = face.mIndices[2];
    }

    indexBuffer->unmap();

    return geometry;

}






optix::GeometryGroup OptiXAssimpGeometry::convertNode(aiNode* node)
{
    //
    // aiming for fig2:  single gg containing many gi 
    //
    traverseNode(node);
    optix::GeometryGroup gg = m_context->createGeometryGroup();
    gg->setChildCount(m_gis.size());

    for(unsigned int i=0 ; i <m_gis.size() ; i++)
    {
        gg->setChild(i, m_gis[i]);
    }

    return gg ;
}


void OptiXAssimpGeometry::traverseNode(aiNode* node)
{
    aiString _name = node->mName;
    const char* name = _name.C_Str(); 

    //printf("OptiXAssimpGeometry::traverseNode name %s #meshes %d #children %d \n", name, node->mNumMeshes, node->mNumChildren);


    for(unsigned int i = 0; i < node->mNumMeshes; i++)
    {   
        unsigned int meshIndex = node->mMeshes[i];

        optix::Geometry geometry = m_geometries[meshIndex] ;

        aiMesh* mesh = m_aiscene->mMeshes[meshIndex];

        unsigned int materialIndex = mesh->mMaterialIndex;

        std::vector<optix::Material>::iterator mit = m_materials.begin()+materialIndex ;

        printf("OptiXAssimpGeometry::traverseNode i %d meshIndex %d materialIndex %d \n", i, meshIndex, materialIndex );

        optix::GeometryInstance gi = m_context->createGeometryInstance( geometry, mit, mit+1  );

        m_gis.push_back(gi);
    }   

    for(unsigned int i = 0; i < node->mNumChildren; i++)
    {
        traverseNode(node->mChildren[i]);
    }

}


