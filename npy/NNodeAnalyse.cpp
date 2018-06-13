
#include <sstream>
#include <algorithm>
#include "NNodeAnalyse.hpp"
#include "NNodeCollector.hpp"
#include "NGrid.hpp"

template <typename T>
NNodeAnalyse<T>::NNodeAnalyse(T* root_)
    :
    root(root_),
    height(depth_(true)),
    nodes(new NNodeCollector<T>(root)),
    count(nodes->inorder.size()),
    grid(new NGrid<T>(height+1, count))
{
    init(); 
}


template <typename T>
NNodeAnalyse<T>::~NNodeAnalyse()
{
    delete nodes ; 
    delete grid ; 
}


template <typename T>
void NNodeAnalyse<T>::init()
{
    initGrid();
}

template <typename T>
void NNodeAnalyse<T>::initGrid()
{
    for(unsigned i=0 ; i < count ; i++)
    {
        T* node = nodes->inorder[i] ;  
        grid->set(node->depth, i, node) ; 
    }
}


template <typename T>
unsigned NNodeAnalyse<T>::depth_(bool label)
{
    return depth_r(root, 0, label);
}

template <typename T>
unsigned NNodeAnalyse<T>::depth_r(T* node, unsigned depth, bool label)
{
     if(node == NULL) return depth ; 
     if(label) node->depth = depth ; 
     if(node->left == NULL && node->right == NULL) return depth ; 

     unsigned ldepth = depth_r(node->left,  depth+1, label ); 
     unsigned rdepth = depth_r(node->right, depth+1, label ); 
     return std::max(ldepth, rdepth) ;
}


template <typename T>
std::string NNodeAnalyse<T>::desc() const 
{
    std::stringstream ss ; 
    ss 
       << "NNodeAnalyse"
       << " height " << height 
       << " count " << count 
       << std::endl 
       << grid->desc()
       ;

    return ss.str(); 
}



#include "no.hpp"
#include "NNode.hpp"

template struct NNodeAnalyse<no> ; 
template struct NNodeAnalyse<nnode> ; 



