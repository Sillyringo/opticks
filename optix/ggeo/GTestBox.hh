#pragma once
#include <cstring>
#include <glm/glm.hpp>

/*

How to hookup a GTestBox, and do it postcache ?
===================================================

  * GMergedMesh asis needs a tree to traverse
    collecting mesh vertices etc.. into its flat list

  * have pre-merged meshes of the target geometry already

  * although are mainly interested to put Analytic PMTs inside 
    the test box need the triangulated geometry 
    for the OpenGL visualisation

  * does the order matter? probably not just triangle soup 
    so tacking the test box geometry onto the end of the GMergedMesh 
    PMT subtree should work ?

    * if order matters would have to diddle with the face indices, 
      in order to slide the additional geometry in front of the rest 

  * essentially need to create a new GMergedMesh combining the 
    pre-cooked one with a vector of GSolids 

    This will entail:

    * determine additional allocation needed (vertex, face counts) for the GSolids
    * allocate space for the combination
    * memcopy from the pre-cooked into the new 
    * copy mesh data from the solids into the new similarly to GMergedMesh from tree
      creation

*/

struct gbbox ; 
class GGeo ; 
class GMesh ; 

class GTestBox {
   public:
       enum { NUM_VERTICES = 24, 
              NUM_FACES = 6*2 } ;

       typedef enum { FRAME, 
                      BOUNDARY, 
                      UNRECOGNIZED } Param_t ;

       static const char* DEFAULT_CONFIG ; 
   public:
       static const char* FRAME_ ; 
       static const char* BOUNDARY_ ; 
   public:
       GTestBox(GGeo* ggeo, const char* config=NULL);
       void make();
   private:
       void configure(const char* config);
       Param_t getParam(const char* k);
       void set(Param_t p, const char* s);
   public:
       void setFrame(const char* s);
       void setBoundary(const char* s);
   public:
       GMesh* makeMesh(unsigned int index, gbbox& bb); 
   public:
       void dump(const char* msg="GTestBox::dump");
   private:
       GGeo*        m_ggeo ;  
       GMesh*       m_mesh ; 
       const char*  m_config ; 
       glm::ivec4   m_frame ;
       unsigned int m_boundary ; 

};


inline GTestBox::GTestBox(GGeo* ggeo, const char* config)
    :
    m_ggeo(ggeo),
    m_mesh(NULL)
{
    configure(config);
}

