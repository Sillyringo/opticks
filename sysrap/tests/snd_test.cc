// ./snd_test.sh

#include <iostream>

#include "ssys.h"
#include "stra.h"
#include "stree.h"
#include "snd.hh"
#include "scsg.hh"
#include "NPFold.h"
#include "OpticksCSG.h"

const char* TEST = getenv("TEST"); 
const char* FOLD = getenv("FOLD"); 
const int TREE = ssys::getenvint("TREE", 0); 


void do_Add()
{
    std::cout << "test_Add" << std::endl ;     
    int a = snd::Zero( 1., 2., 3., 4., 5., 6. ); 
    int b = snd::Sphere(100.) ; 
    int c = snd::ZSphere(100., -10.,  10. ) ; 

    std::vector<int> prims = {a,b,c} ; 
    int d = snd::Compound(CSG_CONTIGUOUS, prims ) ; 

    std::cout << "test_Add :  Desc dumping " << std::endl; 

    std::cout << " a " << snd::Desc(a) << std::endl ; 
    std::cout << " b " << snd::Desc(b) << std::endl ; 
    std::cout << " c " << snd::Desc(c) << std::endl ; 
    std::cout << " d " << snd::Desc(d) << std::endl ; 
}

void test_save()
{
    do_Add(); 

    std::cout << snd::Desc() ; 
    NPFold* fold = snd::Serialize() ; 
    std::cout << " save snd to FOLD " << FOLD << std::endl ;  
    fold->save(FOLD); 
}

void test_load()
{
    NPFold* fold = NPFold::Load(FOLD) ; 
    std::cout << " load snd from FOLD " << FOLD << std::endl ;  
    snd::Import(fold);  
    std::cout << snd::Desc() ; 
}


void test_max_depth_(int n, int xdepth)
{
    const snd* nd = snd::Get(n);
    assert( nd );  
    assert( nd->max_depth() == xdepth );  
    assert( snd::GetMaxDepth(n) == xdepth );  

    std::cout << "test_max_depth_ n " << n << " xdepth " << xdepth << std::endl ;  
}

void test_max_depth()
{
    std::cout << "test_max_depth" << std::endl ;     

    int a = snd::Sphere(100.) ; 
    int b = snd::Sphere(100.) ; 
    int c = snd::Boolean(CSG_UNION, a, b ) ; 
    int d = snd::Box3(100.) ; 
    int e = snd::Boolean(CSG_UNION, c, d ) ; 
    int f = snd::Box3(100.) ; 
    int g = snd::Boolean(CSG_UNION, f, e ) ; 

    // NB setLVID never called here : so no snd has been "declared" as root 

    test_max_depth_( a, 0 ); 
    test_max_depth_( b, 0 ); 
    test_max_depth_( c, 1 ); 
    test_max_depth_( d, 0 );
    test_max_depth_( e, 2 );
    test_max_depth_( f, 0 );
    test_max_depth_( g, 3 );

} 



void test_max_binary_depth_(int n, int xdepth)
{
    const snd* nd = snd::Get(n);
    assert( nd );  
    assert( nd->max_binary_depth() == xdepth );  

    std::cout << "test_max_binary_depth_ n " << n << " xdepth " << xdepth << std::endl ;  
}


void test_max_binary_depth()
{
    std::cout << "test_max_binary_depth" << std::endl ;     

    int a = snd::Sphere(100.) ; 
    int b = snd::Sphere(100.) ; 
    int c = snd::Boolean(CSG_UNION, a, b ) ; 
    int d = snd::Box3(100.) ; 
    int e = snd::Boolean(CSG_UNION, c, d ) ; 
    int f = snd::Box3(100.) ; 
    int g = snd::Boolean(CSG_UNION, f, e ) ; 

    test_max_binary_depth_( a, 0 ); 
    test_max_binary_depth_( b, 0 ); 
    test_max_binary_depth_( c, 1 ); 
    test_max_binary_depth_( d, 0 );
    test_max_binary_depth_( e, 2 );
    test_max_binary_depth_( f, 0 );
    test_max_binary_depth_( g, 3 );
}



void test_num_node_(int n, int xnn)
{
    const snd* nd = snd::Get(n);

    int nnn = nd->num_node() ; 
    int gnn = snd::GetNumNode(n) ; 

    std::cout 
        << "test_num_node_ n " << n 
        << " xnn " << xnn 
        << " nnn " << nnn 
        << " gnn " << gnn 
        << std::endl
        ;  

    assert( nd );  
    assert( nnn == xnn );  
    assert( gnn == xnn );  

}


void test_num_node()
{
    std::cout << "test_num_node" << std::endl ;     

    int a = snd::Sphere(100.) ; 
    int b = snd::Sphere(100.) ; 
    int c = snd::Boolean(CSG_UNION, a, b ) ; 
    int d = snd::Box3(100.) ; 
    int e = snd::Boolean(CSG_UNION, c, d ) ; 
    int f = snd::Box3(100.) ; 
    int g = snd::Boolean(CSG_UNION, f, e ) ; 

    // NB setLVID never called here : so no snd has been "declared" as root 

    test_num_node_( a, 1 ); 
    test_num_node_( b, 1 ); 
    test_num_node_( c, 3 ); 
    test_num_node_( d, 1 );
    test_num_node_( e, 5 );
    test_num_node_( f, 1 );
    test_num_node_( g, 7 );
} 



/**
              g 
          f              e
                     c        d
                  a     b


snd::render_r  ix:    6 dp:    0 sx:   -1 pt:   -1     nc:    2 fc:    5 ns:   -1 lv:    0     tc:    1 pa:   -1 bb:   -1 xf:   -1    un   g ordinal 1
snd::render_r  ix:    5 dp:    1 sx:    0 pt:    6     nc:    0 fc:   -1 ns:    4 lv:    0     tc:  110 pa:    3 bb:    3 xf:   -1    bo   f ordinal 0
snd::render_r  ix:    4 dp:    1 sx:    1 pt:    6     nc:    2 fc:    2 ns:   -1 lv:    0     tc:    1 pa:   -1 bb:   -1 xf:   -1    un   e ordinal 5
snd::render_r  ix:    2 dp:    2 sx:    0 pt:    4     nc:    2 fc:    0 ns:    3 lv:    0     tc:    1 pa:   -1 bb:   -1 xf:   -1    un   c ordinal 3
snd::render_r  ix:    0 dp:    3 sx:    0 pt:    2     nc:    0 fc:   -1 ns:    1 lv:    0     tc:  101 pa:    0 bb:    0 xf:   -1    sp   a ordinal 2
snd::render_r  ix:    1 dp:    3 sx:    1 pt:    2     nc:    0 fc:   -1 ns:   -1 lv:    0     tc:  101 pa:    1 bb:    1 xf:   -1    sp   b ordinal 4
snd::render_r  ix:    3 dp:    2 sx:    1 pt:    4     nc:    0 fc:   -1 ns:   -1 lv:    0     tc:  110 pa:    2 bb:    2 xf:   -1    bo   d ordinal 6
snd::render
    g                           
                                
f                   e           
                                
            c           d       
                                
        a       b               

**/

int make_csgtree_0()
{
    int a = snd::Sphere(100.) ; 
    int b = snd::Sphere(100.) ; 
    int c = snd::Boolean(CSG_UNION, a, b ) ; 
    int d = snd::Box3(100.) ; 
    int e = snd::Boolean(CSG_UNION, c, d ) ; 
    int f = snd::Box3(100.) ; 
    int g = snd::Boolean(CSG_UNION, f, e ) ; 

    snd::SetLabel(a, "a"); 
    snd::SetLabel(b, "b"); 
    snd::SetLabel(c, "c"); 
    snd::SetLabel(d, "d"); 
    snd::SetLabel(e, "e"); 
    snd::SetLabel(f, "f"); 
    snd::SetLabel(g, "g"); 

    snd::SetLVID(g, 0);  // declare root in order to set the depths

    return g ; 
}

int make_csgtree_1()
{
    int a = snd::Sphere(100.) ; 
    int b = snd::Sphere(100.) ; 
    int c = snd::Boolean(CSG_UNION, a, b ) ; 

    snd::SetLabel(a, "a"); 
    snd::SetLabel(b, "b"); 
    snd::SetLabel(c, "c"); 
  
    snd::SetLVID(c, 0);  // declare root in order to set the depths
    return c ; 
}

int make_csgtree_2()
{
    int a = snd::Sphere(100.) ; 
    int b = snd::Sphere(100.) ; 
    int c = snd::Sphere(100.) ; 
    std::vector<int> prims = {a,b,c} ; 

    int d = snd::Compound(CSG_CONTIGUOUS, prims ) ; 

    snd::SetLabel(a, "a"); 
    snd::SetLabel(b, "b"); 
    snd::SetLabel(c, "c"); 
    snd::SetLabel(d, "d"); 
  
    snd::SetLVID(d, 0);  // declare root in order to set the depths
    return d ; 
}

int make_csgtree_3()
{
    int a = snd::Sphere(100.) ; 
    int b = snd::Sphere(100.) ; 
    int c = snd::Sphere(100.) ; 
    std::vector<int> abc = {a,b,c} ; 
    int d = snd::Compound(CSG_CONTIGUOUS, abc ) ; 

    snd::SetLabel(a, "a"); 
    snd::SetLabel(b, "b"); 
    snd::SetLabel(c, "c"); 
    snd::SetLabel(d, "d"); 

    int e = snd::Sphere(100.) ; 
    int f = snd::Sphere(100.) ; 
    int g = snd::Sphere(100.) ; 
    std::vector<int> efg = {e,f,g} ; 
    int h = snd::Compound(CSG_CONTIGUOUS, efg ) ; 

    snd::SetLabel(e, "e"); 
    snd::SetLabel(f, "f"); 
    snd::SetLabel(g, "g"); 
    snd::SetLabel(h, "h"); 

    int i = snd::Boolean(CSG_UNION, d, h ); 
    snd::SetLabel(i, "i"); 

    snd::SetLVID(i, 0);  // declare root in order to set the depths
    return i ; 
}


int make_csgtree_4()
{
    int a = snd::Sphere(100.) ; 
    int b = snd::Sphere(100.) ; 
    int c = snd::Sphere(100.) ; 
    int d = snd::Sphere(100.) ; 
    int e = snd::Sphere(100.) ; 
    int f = snd::Sphere(100.) ; 
    int g = snd::Sphere(100.) ; 

    snd::SetLabel(a, "a"); 
    snd::SetLabel(b, "b"); 
    snd::SetLabel(c, "c"); 
    snd::SetLabel(d, "d"); 
    snd::SetLabel(e, "e"); 
    snd::SetLabel(f, "f"); 
    snd::SetLabel(g, "g"); 

    std::vector<int> prims = {a,b,c,d,e,f,g} ; 
    int z = snd::Collection(prims) ;    // either Contiguous OR UnionTree depending on snd::VERSION 
    snd::SetLabel(z, "z"); 

    //snd::SetLVID(z, 0);  // declare root in order to set the depths
    return z ; 
}



void AddDummyTransformsVisit(int idx)
{
    const snd* nd = snd::Get(idx); 
    bool rhs = nd->is_sibdex(1) ; 

    if( rhs )
    {
        glm::tmat4x4<double> tr = stra<double>::Translate(double(idx), 0., 100.*double(idx), 1., false );  
        snd::SetNodeXForm(idx, tr );
        std::cout << "AddDummyTransformsVisit idx " << idx << std::endl ; 
    }
}
void AddDummyTransforms(int root)
{
    std::cout << "AddDummyTransforms root " << root << std::endl ; 
    snd::PreorderTraverse(root,  std::bind( &AddDummyTransformsVisit , std::placeholders::_1 )  ); 
}


int make_csgtree()
{
    int t = -1 ; 
    switch(TREE)
    {
        case 0: t = make_csgtree_0() ; break ; 
        case 1: t = make_csgtree_1() ; break ; 
        case 2: t = make_csgtree_2() ; break ; 
        case 3: t = make_csgtree_3() ; break ; 
        case 4: t = make_csgtree_4() ; break ; 
    }
    assert( t > -1 ); 
    AddDummyTransforms(t); 
    return t ;    
}






void test_inorder()
{
    std::cout << "test_inorder" << std::endl ;     

    int g = make_csgtree(); 
    const snd* nd = snd::Get(g);

    std::vector<int> order ; 
    nd->inorder(order); 

    std::cout << " order.size " << order.size() << std::endl ; 

    for(int z=0 ; z < int(order.size()) ; z++)
    {
         int idx = order[z] ; 
         const snd* n = snd::Get(idx); 
         std::cout << " idx " << idx ; 
         std::cout << " n.brief " << ( n ? n->brief() : "null" ) << std::endl ;  

         if(n == nullptr) continue ; 
         std::cout << n->label << std::endl ; 
    }

}

void test_dump()
{
    std::cout << "test_dump" << std::endl ;     
    int g = make_csgtree(); 
    const snd* nd = snd::Get(g);

    std::cout << nd->dump() << std::endl ; 
    std::cout << nd->dump2() << std::endl ; 
}

void test_render()
{
    int mode = ssys::getenvint("MODE", -1); 
    std::cout << "test_render mode " << mode  << std::endl ;     

    int g = make_csgtree(); 
    const snd* nd = snd::Get(g);

    std::cout << nd->render(mode) << std::endl ;  
    std::cout << nd->rbrief() << std::endl ;  
}

void test_rbrief()
{
    std::cout << "test_rbrief" << std::endl ;     

    int g = make_csgtree(); 
    const snd* nd = snd::Get(g);
    std::cout << nd->rbrief() << std::endl ;  

}


void test_typenodes(int tc)
{
    std::cout << "test_typenodes tc " << tc << " " << CSG::Tag(tc) <<  std::endl ;     

    int g = make_csgtree(); 
    const snd* nd = snd::Get(g);

    std::vector<int> nodes ; 
    nd->typenodes(nodes, tc ); 

    std::cout << snd::Brief(nodes) ; 
}



void test_typenodes()
{
    test_typenodes(CSG_UNION); 
    test_typenodes(CSG_SPHERE); 
    test_typenodes(CSG_CONTIGUOUS); 
}

void test_typenodes_()
{
    std::cout << "test_typenodes_ " <<  std::endl ;     

    int g = make_csgtree(); 
    const snd* nd = snd::Get(g);

    std::vector<int> nodes ; 

    nd->typenodes_(nodes, CSG_UNION, CSG_SPHERE, CSG_CONTIGUOUS ); 
    std::cout << "snd::DescType " << snd::DescType(CSG_UNION, CSG_SPHERE, CSG_CONTIGUOUS) << std::endl ; 
    std::cout << snd::Brief(nodes) ; 
    nodes.clear();

    nd->typenodes_(nodes, CSG_UNION, CSG_SPHERE ); 
    std::cout << "snd::DescType " << snd::DescType(CSG_UNION, CSG_SPHERE, CSG_CONTIGUOUS) << std::endl ; 
    std::cout << snd::Brief(nodes) ; 
    nodes.clear();

    nd->typenodes_(nodes, CSG_UNION ); 
    std::cout << "snd::DescType " << snd::DescType(CSG_UNION) << std::endl ; 
    std::cout << snd::Brief(nodes) ; 
    nodes.clear();

    nd->typenodes_(nodes, CSG_SPHERE ); 
    std::cout << "snd::DescType " << snd::DescType(CSG_SPHERE) << std::endl ; 
    std::cout << snd::Brief(nodes) ; 
    nodes.clear();

}

void test_leafnodes()
{
    std::cout << "test_leafnodes " <<  std::endl ;     

    int g = make_csgtree(); 
    const snd* nd = snd::Get(g);

    std::vector<int> nodes ; 
    nd->leafnodes(nodes); 

    std::cout << " leafnodes " << nodes.size() << std::endl ; 
    std::cout << snd::Brief(nodes) ; 

    std::cout << nd->render(0) ; 
    std::cout << nd->render(3) ; 
}


void test_find()
{
    std::cout << "test_find" <<  std::endl ;     

    int z = make_csgtree(); 
    const snd* nd = snd::Get(z);

    int a = nd->find('a') ; 
    std::cout << snd::Desc(a) << std::endl ;  

    int f = nd->find('f') ; 
    std::cout << snd::Desc(f)  << std::endl ;  


    std::cout << snd::Render(z, 0) ; 
    std::cout << snd::Render(z, 3) ; 
}


 


void test_ancestors()
{
    std::cout << "test_ancestors " <<  std::endl ;     

    int iz = make_csgtree(); 
    const snd* z = snd::Get(iz);

    std::vector<int> za ; 
    z->ancestors(za); 
    assert( za.size() == 0 ); 

    std::cout << z->render(0) ; 
    std::cout << z->render(3) ; 

    int a = z->find('a') ; 
     
    std::vector<int> aa ; 
    snd::Ancestors(a, aa); 
    assert( aa.size() > 0 ); 

    std::cout << "aa:" << snd::Brief(aa) ; 

    std::cout << "   a:" << snd::Brief(a) << std::endl  ; 

}



void LocalVisit(int idx)
{
    const snd* nd = snd::Get(idx); 
    bool rhs = nd->is_sibdex(1) ; 

    std::cout 
        << "LocalVisit" 
        << " rhs " << ( rhs ? "Y" : "N" ) 
        << " " 
        << nd->brief() 
        << std::endl
        ;  
}

void test_traverse()
{
    std::cout << "test_traverse " <<  std::endl ;     
    int z = make_csgtree(); 


    std::cout << "snd::PreorderTraverse.LocalVisit" << std::endl ; 
    snd::PreorderTraverse(z,  std::bind( &LocalVisit, std::placeholders::_1 )  ); 

    std::cout << snd::Render(z, 1) ; 

    std::cout << "snd::PostorderTraverse.LocalVisit" << std::endl ; 
    snd::PostorderTraverse(z, std::bind( &LocalVisit, std::placeholders::_1 ) ); 
}


void test_NodeTransformProduct()
{
    std::cout << "test_NodeTransformProduct " <<  std::endl ;     

    int z = make_csgtree(); 
    char q = 'f' ; 
    int n = snd::Find(z, q) ;

    std::cout 
        << " z " << z 
        << " q " << q 
        << " n " << n 
        << std::endl  
        << snd::Render(z, 0) 
        << snd::Render(z, 1)
        ;

    glm::tmat4x4<double> t(1.) ; 
    glm::tmat4x4<double> v(1.) ; 
    snd::NodeTransformProduct(n, t, v, false ); 
    std::cout << "t: " << glm::to_string(t) << std::endl ; 
    std::cout << "v: " << glm::to_string(v) << std::endl ; 

    std::cout << snd::DescNodeTransformProduct(n, t, v, false ) << std::endl ;
    std::cout << "t: " << glm::to_string(t) << std::endl ; 
    std::cout << "v: " << glm::to_string(v) << std::endl ; 


}

void _test_GetXF(int idx)
{
    const sxf* xf = snd::GetXF(idx); 
    std::cout 
        << "_test_GetXF"
        << " idx " << std::setw(2) << idx 
        << " xf " << ( xf ? "Y" : "N" )
        << std::endl
        ;    

    if(xf) std::cout << xf->desc() << std::endl ; 
}

void test_GetXF()
{
    int z = make_csgtree(); 

    std::cout 
        << snd::Render(z,0) 
        << snd::Render(z,1)
        << snd::Render(z,2)
        << snd::Render(z,3)
        ; 


    snd::PreorderTraverse(z,  std::bind( &_test_GetXF, std::placeholders::_1 )  ); 

    snd::PreorderTraverse(z, [](int idx){ std::cout << snd::Brief(idx) << std::endl ; }  ); 
    // empty capture clause : https://smartbear.com/blog/c11-tutorial-lambda-expressions-the-nuts-and-bolts/

    NPFold* fold = snd::Serialize() ; 
    std::cout << " save snd to FOLD " << FOLD << std::endl ;  
    fold->save(FOLD, "GetXF"); 
}


int main(int argc, char** argv)
{
    stree st ; 

    if(     strcmp(TEST, "save")==0)      test_save(); 
    else if(strcmp(TEST, "load")==0)      test_load(); 
    else if(strcmp(TEST, "max_depth")==0) test_max_depth() ; 
    else if(strcmp(TEST, "max_binary_depth")==0) test_max_binary_depth() ; 
    else if(strcmp(TEST, "num_node")==0)  test_num_node() ; 
    else if(strcmp(TEST, "inorder")==0)   test_inorder() ; 
    else if(strcmp(TEST, "dump")==0)      test_dump() ; 
    else if(strcmp(TEST, "render")==0)    test_render() ; 
    else if(strcmp(TEST, "rbrief")==0)    test_rbrief() ; 
    else if(strcmp(TEST, "typenodes")==0) test_typenodes() ; 
    else if(strcmp(TEST, "typenodes_")==0) test_typenodes_() ; 
    else if(strcmp(TEST, "leafnodes")==0)  test_leafnodes() ; 
    else if(strcmp(TEST, "find")==0)  test_find() ; 
    else if(strcmp(TEST, "ancestors")==0)  test_ancestors() ; 
    else if(strcmp(TEST, "traverse")==0)  test_traverse() ; 
    else if(strcmp(TEST, "NodeTransformProduct")==0)  test_NodeTransformProduct() ; 
    else if(strcmp(TEST, "GetXF")==0)  test_GetXF() ; 
    else std::cerr << " TEST not matched " << TEST << std::endl ; 
        
    return 0 ; 
}

