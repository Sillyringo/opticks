
#include <cstdio>
#include <cassert>
#include <cmath>
#include <sstream>
#include <iomanip>

#include "NGLM.hpp"
#include "NGLMExt.hpp"

#include "NNode.hpp"
#include "NPart.hpp"
#include "NQuad.hpp"
#include "NSphere.hpp"
#include "NBox.hpp"
#include "NBBox.hpp"

#include "PLOG.hh"


double nnode::operator()(double,double,double) 
{
    return 0.f ; 
} 

std::string nnode::desc()
{
    std::stringstream ss ; 
    ss  << " nnode "
        << std::setw(3) << type 
        << std::setw(15) << CSGName(type) 
        ;     
    return ss.str();
}


void nnode::dump(const char* msg)
{
    printf("(%s)%s\n",csgname(), msg);
    if(left && right)
    {
        left->dump("left");
        right->dump("right");
    }

    if(transform)
    {
        //std::cout << "transform: " << glm::to_string( *transform ) << std::endl ; 
        std::cout << "transform: " << *transform  << std::endl ; 
    } 

}

void nnode::Init( nnode& n , OpticksCSG_t type, nnode* left, nnode* right )
{
    n.idx = 0 ; 
    n.type = type ; 

    n.left = left ; 
    n.right = right ; 
    n.parent = NULL ; 
    n.label = NULL ; 

    n.transform = NULL ; 
    n.gtransform = NULL ; 

    n.param = {0.f, 0.f, 0.f, 0.f };
}

const char* nnode::csgname()
{
   return CSGName(type);
}
unsigned nnode::maxdepth()
{
    return _maxdepth(0);
}
unsigned nnode::_maxdepth(unsigned depth)  // recursive 
{
    return left && right ? nmaxu( left->_maxdepth(depth+1), right->_maxdepth(depth+1)) : depth ;  
}



nmat4pair* nnode::global_transform()
{
    return global_transform(this);
}

nmat4pair* nnode::global_transform(nnode* n)
{
    std::vector<nmat4pair*> tt ; 
    while(n)
    {
        if(n->transform) tt.push_back(n->transform);
        n = n->parent ; 
    }
    return tt.size() == 0 ? NULL : nmat4pair::product(tt) ; 
}



void nnode::update_gtransforms()
{
    update_gtransforms_r(this);
}
void nnode::update_gtransforms_r(nnode* node)
{
    // NB this traversal doesnt need parent links, but global_transforms does...
    node->gtransform = node->global_transform();

    if(node->left && node->right)
    {
        update_gtransforms_r(node->left);
        update_gtransforms_r(node->right);
    }
}






npart nnode::part()
{
    // this is invoked by NCSG::export_r to totally re-write the nodes buffer 

    nbbox bb = bbox();

    npart pt ; 
    pt.zero();
    pt.setParam( param );
    pt.setTypeCode( type );

    // hmm need to setGTransform() using 
    // an index into a buffer of distinct compound transforms for the tree

    pt.setBBox( bb );

    return pt ; 
}


nbbox nnode::bbox()
{
   // needs to be overridden for primitives
    nbbox bb = make_nbbox() ; 
    if(left && right)
    {
        bb.include( left->bbox() );
        bb.include( right->bbox() );
    }
    return bb ; 
}



/**
To translate or rotate a surface modeled as an SDF, you can apply the inverse
transformation to the point before evaluating the SDF.

**/


double nunion::operator()(double px, double py, double pz) 
{
    assert( left && right );
    double l = (*left)(px, py, pz) ;
    double r = (*right)(px, py, pz) ;
    return fmin(l, r);
}
double nintersection::operator()(double px, double py, double pz) 
{
    assert( left && right );
    double l = (*left)(px, py, pz) ;
    double r = (*right)(px, py, pz) ;
    return fmax( l, r);
}
double ndifference::operator()(double px, double py, double pz) 
{
    assert( left && right );
    double l = (*left)(px, py, pz) ;
    double r = (*right)(px, py, pz) ;
    return fmax( l, -r);    // difference is intersection with complement, complement negates signed distance function
}


void nnode::Tests(std::vector<nnode*>& nodes )
{
    nsphere* a = make_nsphere_ptr(0.f,0.f,-50.f,100.f);
    nsphere* b = make_nsphere_ptr(0.f,0.f, 50.f,100.f);
    nbox*    c = make_nbox_ptr(0.f,0.f,0.f,200.f);

    nunion* u = make_nunion_ptr( a, b );
    nintersection* i = make_nintersection_ptr( a, b ); 
    ndifference* d1 = make_ndifference_ptr( a, b ); 
    ndifference* d2 = make_ndifference_ptr( b, a ); 
    nunion* u2 = make_nunion_ptr( d1, d2 );

    nodes.push_back( (nnode*)a );
    nodes.push_back( (nnode*)b );
    nodes.push_back( (nnode*)u );
    nodes.push_back( (nnode*)i );
    nodes.push_back( (nnode*)d1 );
    nodes.push_back( (nnode*)d2 );
    nodes.push_back( (nnode*)u2 );

    nodes.push_back( (nnode*)c );


    float radius = 200.f ; 
    float inscribe = 1.3f*radius/sqrt(3.f) ; 

    nsphere* sp = make_nsphere_ptr(0.f,0.f,0.f,radius);
    nbox*    bx = make_nbox_ptr(0.f,0.f,0.f, inscribe );
    nunion*  u_sp_bx = make_nunion_ptr( sp, bx );
    nintersection*  i_sp_bx = make_nintersection_ptr( sp, bx );
    ndifference*    d_sp_bx = make_ndifference_ptr( sp, bx );
    ndifference*    d_bx_sp = make_ndifference_ptr( bx, sp );

    nodes.push_back( (nnode*)u_sp_bx );
    nodes.push_back( (nnode*)i_sp_bx );
    nodes.push_back( (nnode*)d_sp_bx );
    nodes.push_back( (nnode*)d_bx_sp );


}




std::function<float(float,float,float)> nnode::sdf()
{
    nnode* node = this ; 
    std::function<float(float,float,float)> f ; 
    switch(node->type)
    {
        case CSG_UNION:
            {
                nunion* n = (nunion*)node ; 
                f = *n ;
            }
            break ;
        case CSG_INTERSECTION:
            {
                nintersection* n = (nintersection*)node ; 
                f = *n ;
            }
            break ;
        case CSG_DIFFERENCE:
            {
                ndifference* n = (ndifference*)node ; 
                f = *n ;
            }
            break ;
        case CSG_SPHERE:
            {
                nsphere* n = (nsphere*)node ; 
                f = *n ;
            }
            break ;
        case CSG_BOX:
            {
                nbox* n = (nbox*)node ;  
                f = *n ;
            }
            break ;
        default:
            LOG(fatal) << "Need to add upcasting for type: " << node->type << " name " << CSGName(node->type) ;  
            assert(0);
    }
    return f ;
}



void nnode::collect_prim_centers(std::vector<glm::vec3>& centers, std::vector<glm::vec3>& dirs )
{
    std::vector<nnode*> prim ; 
    collect_prim(prim); 

    unsigned npr = prim.size();
    for(unsigned i=0 ; i < npr ; i++)
    {
        nnode* p = prim[i] ; 
        switch(p->type)
        {
            case CSG_SPHERE: 
               {  
                   nsphere* n = (nsphere*)p ;
                   centers.push_back(n->gcenter()); 
                   glm::vec4 dir(1,1,1,0); 
                   if(n->gtransform) dir = n->gtransform->tr * dir ; 
                   dirs.push_back( glm::vec3(dir));
               }
               break ;  
          
            case CSG_BOX: 
               {  
                   nbox* n = (nbox*)p ;
                   centers.push_back(n->gcenter()); 
                   
                   glm::vec4 dir(1,1,1,0); 
                   if(n->gtransform) dir = n->gtransform->tr * dir ; 
                   dirs.push_back( glm::vec3(dir));
               }
               break ;  
            default:
               {
                   LOG(fatal) << "nnode::collect_prim_centers unhanded shape type " << p->type << " name " << CSGName(p->type) ;
                   assert(0) ;
               }
        }
    }
}


void nnode::collect_prim(std::vector<nnode*>& prim)
{
    collect_prim_r(prim, this);   
}

void nnode::collect_prim_r(std::vector<nnode*>& prim, nnode* node)
{
    bool internal = node->left && node->right ; 
    if(!internal)
    {
        prim.push_back(node);
    }
    else
    {
        collect_prim_r(prim, node->left);
        collect_prim_r(prim, node->right);
    }
}


void nnode::dump_prim( const char* msg, int verbosity )
{
    std::vector<nnode*> prim ;
    collect_prim(prim);   
    unsigned nprim = prim.size();
    LOG(info) << msg << " nprim " << nprim ; 
    for(unsigned i=0 ; i < nprim ; i++)
    {
        nnode* p = prim[i] ; 
        switch(p->type)
        {
            case CSG_SPHERE: ((nsphere*)p)->pdump("sp",verbosity) ; break ; 
            case CSG_BOX   :    ((nbox*)p)->pdump("bx",verbosity) ; break ; 
            default:
            {
                   LOG(fatal) << "nnode::dump_prim unhanded shape type " << p->type << " name " << CSGName(p->type) ;
                   assert(0) ;
            }
        }
    }
}




