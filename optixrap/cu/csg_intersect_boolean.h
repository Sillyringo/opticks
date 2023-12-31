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

#pragma once

/**0
csg_intersect_boolean.h : General CSG intersection using evaluative_csg approach
===================================================================================

* https://bitbucket.org/simoncblyth/opticks/src/master/optixrap/cu/csg_intersect_boolean.h


Intersecting rays with general CSG shapes requires the appropriate primitive intersect to be  
selected depending on the origin and direction of the ray and the current t_min. 

Traditional implementations of CSG intersection first calculate 
ray intervals with each primitive and then combine these intervals using the boolean operators to determine intersects.  
Efficient use of GPUs requires many thousands of simultaneously operational threads which disfavors the traditional approach due to  
the requirement to store intervals for all constituent primitives.  A quite different approach
described by Andrew Kensler avoids interval storage by instead selecting between 
the two candidate intersects at each level of the binary tree, which allows a recursive algorithm 
to be developed. The two candidate intersects at each level are classified as "Enter", "Exit" or "Miss" 
using the angle between the ray direction and surface normal. 
Six decision tables corresponding to which side is closer and 
to the three boolean operators are used to determine an action from the classifications such as returning an intersect 
or advancing t_min and intersecting again. 

Recursive function calls are a natural way to process self similar structures such as CSG trees,  
however recursion is a memory expensive technique which makes it inappropriate for GPU usage.
Although NVIDIA OptiX supports recursive ray tracing in does not support recursion within intersect programs. 
The Opticks "evaluative" CSG implementation was inspired by the realization that CSG node tree intersection 
directly parallels binary expression tree evaluation and that techniques to simplify expression 
tree evaluation such as using postorder traversals could be applied. 
Binary expression trees are used to represent and evaluate mathematical expressions. 
Factoring out the postorder sequence allowed an iterative solution to be developed 
for a recursive problem.  

The CSG implementation relies on selection of the closer of two intersects at each level 
of the node tree. When the faces of constituent shapes coincide the ambiguity regarding which is closer
can cause spurious intersects. Modifying some constituents to prevent coincident faces avoids 
the issue without changing the intended geometry. As such coincidences are rather common Opticks includes 
detection and automated fixing for some common situations.    


* postorder traversal means that have always 
  visited left and right subtrees before visiting a node

* slavish python translations of the csg intersect implementationa
  are in dev/csg/slavish.py 

* postorder_sequence for four tree heights were prepared by  
  opticks/dev/csg/node.py:Node.postOrderSequence
  
* the sequence contains 1-based levelorder indices(nodeIdx) in right to left postorder

* left,right child of nodeIdx are at (nodeIdx*2, nodeIdx*2+1)

* for non-perfect trees, the height means the maximum 


Below indices are postorder slice flavor, not levelorder

::

   Height 3 tree, numNodes = 15, halfNodes = 7, root at i = 14 

      upTree       i      : numNodes    14        : 15      =  14:15    
      leftTree     i - 2h : i - h       14 - 2*7  : 14 - 7  =   0:7       
      rightTree    i -  h : i           14 -  7   : 14      =   7:14

  NB all nodes including root needs an upTree tranche to evaluate left and right 



CSG shape complete binary tree serialization 
------------------------------------------------

A complete binary tree serialization with array indices matching level order tree indices
and zeros at missing nodes is used for the serialization of the CSG trees. This simple 
serialization allows tree navigation directly from bitwise manipulations of the serialized array index.
 
Advantages:

* simple, with no tree overheads for child/parent etc.. 
* no need to deserialize as can postorder tree traverse direct from the serialized buffer 
  just by bit twiddling, as shown below

Disadvantages:

* very inefficient for complex CSG shapes with many constituent nodes  
* must implement tree balancing to handle CSG shapes of "medium" complexity, 
  this is done in geometry preprocessing 


::

    .
                                                      depth     elevation

                         1                               0           3    

              10                   11                    1           2    

         100       101        110        111             2           1    
     
     1000 1001  1010 1011  1100 1101  1110  1111         3           0    


    postorder_next(i,elevation) = i & 1 ? i >> 1 : (i << elevation) + (1 << elevation) ;   // from pattern of bits 


A postorder tree traverse visits all nodes, starting from leftmost, such that children 
are visited prior to their parents.



CSG Development code including python implementations of the CUDA
-------------------------------------------------------------------

:doc:`../../dev/csg/index`


1-based binary tree indices in binary
--------------------------------------

Repeat above enumeration in nodeIdx lingo

::

      nodeIdx{dec}
       [postorderIdx]

              height 3 tree

                                   1{1}                                                 elev:3    depth:0
                                   [14]       

                10{2}                                  11{3}                            elev:2    depth:1
                 [6]                                   [13]
  
          100{4}           101{5}                110{6}              111{7}             elev:1    depth:2
           [2]              [5]                  [9]                 [12]
 
      1000{8} 1001{9}   1010{a}  1011{b}      1100{c} 1101{d}      1110{e}  1111{f}     elev:0    depth:3 
      [0]     [1]       [3]      [4]          [7]     [8]          [10]     [11]
      


      upTree         1    :    0    
                    root     1 >> 1 ("parent" of root)  

      leftTree     
                     10{2} << 2      11{3} << 2   (leftmost of the rhs is one-past the leftTree)
                   1000{8}         1100{c}      
    
      rightTree     
                     11{3} << 2        1      <-- one-past the end of the rightTree is root   
                   1100{c}                  
             
                         
      Algorithm to find leftTree of nodeIdx, is find leftIdx
         
          leftIdx  = (nodeIdx << 1) 
          rightIdx = (nodeIdx << 1) + 1 
   
    leftTree      (leftIdx << (elevation-1))    :  (rightIdx << (elevation-1))   
    rightTree      (rightIdx << (elevation-1))  :   nodeIdx      




POSTORDER_RANGE debug ideas
-------------------------------

* parallelR variables
* use quad-decker begin,end,beginR,endR inside the tranche slice so can 
  see exactly where things diverge


* maybe running into elev 0 and doing << -1 



Recursive within intersect, nope
-----------------------------------

Hmm seems cannot do recursive within an intersect program anyhow ??

::

    libc++abi.dylib: terminating with uncaught exception of type optix::Exception: 
    Parse error (Details: Function "RTresult _rtProgramCreateFromPTXFile(RTcontext, const char *, const char *, RTprogram *)" 
    caught exception: /usr/local/opticks/installcache/PTX/OptiXRap_generated_hemi-pmt.cu.ptx: 
    error: Recursion detected in function 
    _Z15recursive_csg_rjjjf( file /usr/local/opticks/installcache/PTX/OptiXRap_generated_hemi-pmt.cu.ptx ), 
    cannot inline. [5308892], [5308892])



Stacks
--------

Stack curr:

* -1 : empty
* 0 : one item
* 1 : two items
* SIZE - 1 : SIZE items, full stack


0**/




__device__ __forceinline__
unsigned long getBitField(unsigned long val, int pos, int len) {
  unsigned long ret;
  asm("bfe.u64 %0, %1, %2, %3;" : "=l"(ret) : "l"(val), "r"(pos), "r"(len));
  return ret;
}


/*
intersect_boolean_only_first
-----------------------------

Used during development only, gives intersects only with the first part.

*/

static __device__
void intersect_boolean_only_first( const Prim& prim, const uint4& identity )
{
    unsigned a_partIdx = prim.partOffset() + 1u ;  
    //unsigned tranOffset = prim.tranOffset(); 

    float tA_min = propagate_epsilon ;  
    float4 tt = make_float4(0.f,0.f,1.f, tA_min);

    //IntersectionState_t a_state = intersect_part( a_partIdx , tA_min, tt ) ;
    csg_intersect_part( prim, a_partIdx , tA_min, tt ) ;

    IntersectionState_t a_state = tt.w > tA_min ? 
                        ( (tt.x * ray.direction.x + tt.y * ray.direction.y + tt.z * ray.direction.z) < 0.f ? State_Enter : State_Exit ) 
                                  :
                              State_Miss
                              ; 

    if(a_state != State_Miss)
    {
        if(rtPotentialIntersection(tt.w))
        {
            shading_normal.x = geometric_normal.x = tt.x ;
            shading_normal.y = geometric_normal.y = tt.y ;
            shading_normal.z = geometric_normal.z = tt.z ;

            instanceIdentity = identity ;
            //instanceIdentity.x = dot(a_normal, ray.direction) < 0.f ? 1 : 2 ;

            rtReportIntersection(0);
        }
    }
}


#define CSG_STACK_SIZE 15
#define TRANCHE_STACK_SIZE 4

#define POSTORDER_NODE(postorder, i) (((postorder) & (0xFull << (i)*4 )) >> (i)*4 )
#define POSTORDER_NODE_BFE(postorder, i) (getBitField((postorder), (i)*4, 4))



#define CSG_PUSH(_stack, stack, ERR, val) \
             { \
                 if((stack) >= CSG_STACK_SIZE - 1 ) \
                 {  \
                     ierr |= (ERR) ; \
                     break ;  \
                 }  \
                 (stack)++ ;   \
                 (_stack)[(stack)] = val ; \
             } \


#define CSG_PUSH_(ERR, val, idx) \
             { \
                 if(csg.curr >= CSG_STACK_SIZE - 1 ) \
                 {  \
                     ierr |= (ERR) ; \
                     break ;  \
                 }  \
                 csg.curr++ ;   \
                 csg.data[csg.curr] = (val) ; \
                 csg.idx[csg.curr] = (idx) ; \
             } \



#define CSG_POP(_stack, stack, ERR, ret ) \
             { \
                if((stack) < 0) \
                {  \
                    ierr |= (ERR)  ;\
                    break ; \
                }  \
                (ret) = (_stack)[(stack)] ;  \
                (stack)-- ;    \
             } \

#define CSG_POP_(ERR, ret, idx ) \
             { \
                if(csg.curr < 0) \
                {  \
                    ierr |= (ERR)  ;\
                    break ; \
                }  \
                (ret) = csg.data[csg.curr] ;  \
                (idx) = csg.idx[csg.curr] ;  \
                csg.curr-- ;    \
             } \


// pop without returning data
#define CSG_POP0(_stack, stack, ERR ) \
             { \
                if((stack) < 0) \
                {  \
                    ierr |= (ERR)  ;\
                    break ; \
                }  \
                (stack)-- ;    \
             } \
 

#define CSG_POP0_(ERR ) \
             { \
                if(csg.curr < 0) \
                {  \
                    ierr |= (ERR)  ;\
                    break ; \
                }  \
                csg.curr-- ;    \
             } \
 

#define CSG_CLASSIFY( ise, dir, tmin )   (fabsf((ise).w) > (tmin) ?  ( (ise).x*(dir).x + (ise).y*(dir).y + (ise).z*(dir).z < 0.f ? State_Enter : State_Exit ) : State_Miss )


#define TRANCHE_PUSH0( _stacku, _stackf, stack, ERR, valu, valf ) \
           { \
                if((stack) >= TRANCHE_STACK_SIZE - 1 ) \
                {  \
                    ierr |= ERR ; \
                } \
                else  \
                {  \
                    (stack)++ ; \
                    setByIndex( (_stacku), (stack), (valu) ) ; \
                    setByIndex( (_stackf), (stack), (valf) ) ; \
                } \
           }   


#define TRANCHE_PUSH( _stacku, _stackf, stack, ERR, valu, valf ) \
            { \
                if((stack) >= TRANCHE_STACK_SIZE - 1) \
                {   \
                     ierr |= (ERR) ; \
                     break ; \
                } \
                (stack)++ ; \
                setByIndex( (_stacku), (stack), (valu) ) ; \
                setByIndex( (_stackf), (stack), (valf) ) ; \
           }  


#define TRANCHE_POP0( _stacku, _stackf, stack, valu, valf ) \
         (valf) = getByIndex((_stackf), (stack));  \
         (valu) = getByIndex((_stacku), (stack) ); \
         (stack)-- ; 




#define POSTORDER_SLICE(begin, end) (  (((end) & 0xff) << 8) | ((begin) & 0xff)  )
#define POSTORDER_BEGIN( slice )  ( (slice) & 0xff )
#define POSTORDER_END( slice )  ( (slice) >> 8 )


#define POSTORDER_RANGE(beginIdx, endIdx) (  (((endIdx) & 0xff) << 8) | ((beginIdx) & 0xff)  )
#define POSTORDER_RANGE_BEGIN( range )    (  (range) & 0xff )
#define POSTORDER_RANGE_END(   range )    (  (range) >> 8 )


#include "pack.h"


/**1
csg_intersect_boolean.h : struct Tranche
-------------------------------------------

Postorder Tranch storing a stack of slices into the postorder sequence.

slice
   32 bit unsigned holding a pair of begin and end indices 
   specifying a range over the postorder traversal sequence

tranche_push 
    push (slice, tmin) onto the small stack 

tranche_pop
    pops off (slice, tmin) 

tranche_repr
    representation of the stack of slices packed into a 64 bit unsigned long long  

1**/

struct Tranche
{
    float tmin[TRANCHE_STACK_SIZE] ;     // TRANCHE_STACK_SIZE is 4 
    unsigned  slice[TRANCHE_STACK_SIZE] ; 
    int curr ;
};


__device__ int tranche_push(Tranche& tr, const unsigned slice, const float tmin)
{
    if(tr.curr >= TRANCHE_STACK_SIZE - 1) return ERROR_TRANCHE_OVERFLOW ; 
    tr.curr++ ; 
    tr.slice[tr.curr] = slice  ; 
    tr.tmin[tr.curr] = tmin ; 
    return 0 ; 
}

__device__ int tranche_pop(Tranche& tr, unsigned& slice, float& tmin)
{
    if(tr.curr < 0) return ERROR_POP_EMPTY  ; 
    slice = tr.slice[tr.curr] ;
    tmin = tr.tmin[tr.curr] ;  
    tr.curr-- ; 
    return 0 ; 
}

__device__ unsigned long long tranche_repr(Tranche& tr)
{
    unsigned long long val = 0ull ; 
    if(tr.curr == -1) return val ; 

    unsigned long long c = tr.curr ;
    val |= ((c+1ull)&0xfull)  ;     // count at lsb, contents from msb 
 
    do { 
        unsigned long long x = tr.slice[c] & 0xffff ;
        val |=  x << ((4ull-c-1ull)*16ull) ; 
    } 
    while(c--) ; 

    return val ; 
} 


struct History
{
    enum {
           NUM = 2,     
           SIZE = 64,    // of each carrier     
           NITEM = 16,   // items within each 64bit
           NBITS = 4,     // bits per item    
           MASK  = 0xf 
         } ;  // 
    unsigned long long idx[NUM] ; 
    unsigned long long ctrl[NUM] ; 
    int curr ; 
};


__device__ int history_append( History& h, unsigned idx, unsigned ctrl)
{
    if((h.curr+1) > h.NUM*h.NITEM ) return ERROR_OVERFLOW ; 
    h.curr++ ; 

    int nb = h.curr/h.NITEM  ;                    // target carrier int 
    unsigned long long  ii = h.curr*h.NBITS - h.SIZE*nb ; // bit offset within target 64bit 
    unsigned long long hidx = h.MASK & idx ;
    unsigned long long hctrl = h.MASK & ctrl ;

    h.idx[nb]  |=  hidx << ii   ; 
    h.ctrl[nb] |=  hctrl << ii  ; 

    return 0 ; 
}





/**2
csg_intersect_boolean.h : struct CSG
-------------------------------------------

Small stack of float4 isect (surface normals and t:distance_to_intersect).

csg_push 
    push float4 isect and nodeIdx into stack

csg_pop
    pop float4 isect and nodeIdx off the stack   

2**/

struct CSG 
{
   float4 data[CSG_STACK_SIZE] ; 
   unsigned idx[CSG_STACK_SIZE] ; 
   int curr ;
};

__device__ int csg_push(CSG& csg, const float4& isect, unsigned nodeIdx)
{
    if(csg.curr >= CSG_STACK_SIZE - 1) return ERROR_OVERFLOW ; 
    csg.curr++ ; 
    csg.data[csg.curr] = isect ; 
    csg.idx[csg.curr] = nodeIdx ; 
    return 0 ; 
}
__device__ int csg_pop(CSG& csg, float4& isect, unsigned& nodeIdx)
{
    if(csg.curr < 0) return ERROR_POP_EMPTY ;     
    isect = csg.data[csg.curr] ;
    nodeIdx = csg.idx[csg.curr] ;
    csg.idx[csg.curr] = 0u ;   // scrub the idx for debug
    csg.curr-- ; 
    return 0 ; 
}

__device__ int csg_pop0(CSG& csg)
{
    if(csg.curr < 0) return ERROR_POP_EMPTY ;     
    csg.idx[csg.curr] = 0u ;   // scrub the idx for debug
    csg.curr-- ; 
    return 0 ; 
}

__device__ unsigned long long csg_repr(CSG& csg)
{
    unsigned long long val = 0 ; 
    if(csg.curr == -1) return val ; 

    unsigned long long c = csg.curr ; 
    val |= (c+1ull) ;  // count at lsb, contents from msb 
 
    do { 
        unsigned long long x = csg.idx[c] & 0xf ;
        val |=  x << ((16ull-c-1ull)*4ull) ; 
    } 
    while(c--) ; 

    return val ; 
} 



/*
unsupported_recursive_csg_r
----------------------------

Demonstration unsupported recursive implemetation. 
Does not run as recursion is not allowed in OptiX intersect functions.

*/

__device__
float4 unsupported_recursive_csg_r( const Prim& prim, unsigned partOffset, unsigned numInternalNodes, unsigned nodeIdx, float tmin )
{
    unsigned leftIdx = nodeIdx*2 ; 
    unsigned rightIdx = leftIdx + 1 ; 

    bool bileaf = leftIdx > numInternalNodes ; 
    enum { LEFT=0, RIGHT=1 };

    float4 isect[2] ;  
    isect[LEFT] = make_float4(0.f, 0.f, 0.f, 0.f) ;  
    isect[RIGHT] = make_float4(0.f, 0.f, 0.f, 0.f) ;  
    if(bileaf)
    {
        csg_intersect_part( prim, partOffset+leftIdx-1, tmin, isect[LEFT] );
        csg_intersect_part( prim, partOffset+rightIdx-1, tmin, isect[RIGHT] );
    }  
    else
    {
        isect[LEFT]  = unsupported_recursive_csg_r( prim, partOffset, numInternalNodes, leftIdx, tmin);
        isect[RIGHT] = unsupported_recursive_csg_r( prim, partOffset, numInternalNodes, rightIdx, tmin);
    } 

    Part pt = partBuffer[partOffset+nodeIdx-1] ; 
    OpticksCSG_t operation = (OpticksCSG_t)pt.typecode() ;


    IntersectionState_t x_state[2] ; 
    x_state[LEFT]  = CSG_CLASSIFY( isect[LEFT], ray.direction, tmin );
    x_state[RIGHT] = CSG_CLASSIFY( isect[RIGHT], ray.direction, tmin );

    float x_tmin[2] ;
    x_tmin[LEFT] = tmin ; 
    x_tmin[RIGHT] = tmin ; 

    int ctrl = boolean_ctrl_packed_lookup( operation, x_state[LEFT], x_state[RIGHT], isect[LEFT].w <= isect[RIGHT].w ) ;

    if(ctrl >= CTRL_LOOP_A)  // not ready to return yet 
    {
        int side = ctrl - CTRL_LOOP_A ;
        int loop = -1 ; 
        while(side > -1 && loop < 10)
        {
            loop += 1 ; 
            x_tmin[side] = isect[side].w + propagate_epsilon ; 
            if(bileaf)
            {
                csg_intersect_part( prim, partOffset+leftIdx+side-1, x_tmin[side], isect[LEFT+side] );
            }
            else
            {
                isect[LEFT+side] = unsupported_recursive_csg_r( prim, partOffset, numInternalNodes, leftIdx+side, x_tmin[side]);
            }
            x_state[LEFT+side] = CSG_CLASSIFY( isect[LEFT+side], ray.direction, x_tmin[side] );
            ctrl = boolean_ctrl_packed_lookup( operation, x_state[LEFT], x_state[RIGHT], isect[LEFT].w <= isect[RIGHT].w ) ;
            side = ctrl - CTRL_LOOP_A  ; 
        }
    }
    float4 result = ctrl == CTRL_RETURN_MISS ?  make_float4(0.f, 0.f, 0.f, 0.f ) : ( ctrl == CTRL_RETURN_A ? isect[LEFT] : isect[RIGHT] ) ;
    if(ctrl == CTRL_RETURN_FLIP_B)
    {
        result.x = -result.x ;     
        result.y = -result.y ;     
        result.z = -result.z ;     
    }
    return result ;
}
 


static __device__
void UNSUPPORTED_recursive_csg( const Prim& prim, const uint4& identity )
{
    unsigned partOffset = prim.partOffset() ; 
    unsigned numParts   = prim.numParts() ;
    //unsigned tranOffset = prim.tranOffset() ; 

    unsigned fullHeight = TREE_HEIGHT(numParts) ; // 1->0, 3->1, 7->2, 15->3, 31->4 
    unsigned numInternalNodes = TREE_NODES(fullHeight-1) ;
    unsigned rootIdx = 1 ; 

    float4 ret = unsupported_recursive_csg_r( prim, partOffset, numInternalNodes, rootIdx, ray.tmin ); 
    if(rtPotentialIntersection( fabsf(ret.w) ))
    {
        shading_normal = geometric_normal = make_float3(ret.x, ret.y, ret.z) ;
        instanceIdentity = identity ;
        rtReportIntersection(0);
    } 
}

 
#define USE_TWIDDLE_POSTORDER 1

/**3
csg_intersect_boolean.h : evaluative_csg
-------------------------------------------

Recursive CSG intersection algorithm implemented iteratively 
using a stack of slices into the postorder traversal sequence 
of the complete binary tree, effectively emulating recursion.

The whole algoritm depends on the postorder traversal feature 
that the left and right children of a node are always visited 
before the node itself. 

USE_TWIDDLE_POSTORDER 
   macro that is now standardly defined

   bit-twiddle postorder is limited to trees of height 7, 
   ie a maximum of 0xff (255) nodes
   (using 2-bytes with PACK2 would bump that to 0xffff (65535) nodes)
   In any case 0xff nodes are far more than this is expected to be used with


3**/

static __device__
void evaluative_csg( const Prim& prim, const int primIdx )   // primIdx just used for identity access
{
    unsigned partOffset = prim.partOffset() ; 
    unsigned numParts   = prim.numParts() ;
    unsigned tranOffset = prim.tranOffset() ; 

    Part pt0 = partBuffer[partOffset + 0] ; 
    unsigned boundary = pt0.boundary() ; 


    unsigned height = TREE_HEIGHT(numParts) ; // 1->0, 3->1, 7->2, 15->3, 31->4 

#ifdef USE_TWIDDLE_POSTORDER
    if(height > 7)
    {
        rtPrintf("// evaluative_csg repeat_index %d primIdx %d tranOffset %u numParts %u perfect tree height %u exceeds current limit\n", repeat_index, primIdx, tranOffset, numParts, height ) ;
        return ; 
    } 
#else
    // pre-baked postorder limited to height 3 tree,  ie maximum of 0xf nodes
    // by needing to stuff the postorder sequence 0x137fe6dc25ba498ull into 64 bits 
    if(height > 3)
    {
        rtPrintf("// evaluative_csg repeat_index %d tranOffset %u numParts %u perfect tree height %u exceeds current limit\n", repeat_index, tranOffset, numParts, height ) ;
        return ; 
    } 
    const unsigned long long postorder_sequence[4] = { 0x1ull, 0x132ull, 0x1376254ull, 0x137fe6dc25ba498ull } ;
    unsigned long long postorder = postorder_sequence[height] ; 
#endif

    int ierr = 0 ;  

    History hist ; 
    hist.curr = -1 ; 
    hist.ctrl[0] = 0 ;
    hist.ctrl[1] = 0 ;
    hist.idx[0] = 0 ;
    hist.idx[1] = 0 ;

    Tranche tr ; 
    tr.curr = -1 ;


#ifdef USE_TWIDDLE_POSTORDER
    unsigned fullTree = PACK4(0,0, 1 << height, 0 ) ;  // leftmost: 1<<height,  root:1>>1 = 0 ("parent" of root)  
#else
    unsigned numNodes = TREE_NODES(height) ;      
    unsigned fullTree = PACK4(0, numNodes, 1 << height, 0 ) ; 
#endif

    tranche_push( tr, fullTree, ray.tmin );

    CSG csg ;  
    csg.curr = -1 ;


    int tloop = -1 ; 

    while (tr.curr > -1)
    {
        tloop++ ; 
        unsigned slice ; 
        float tmin ; 
        ierr = tranche_pop(tr, slice, tmin );
        if(ierr) break ; 



#ifdef USE_TWIDDLE_POSTORDER
        // beginIdx, endIdx are 1-based level order tree indices, root:1 leftmost:1<<height 
        unsigned beginIdx = UNPACK4_2(slice);
        unsigned endIdx   = UNPACK4_3(slice);
#else
        // begin, end are 0-based indices into the postorder
        unsigned begin     = UNPACK4_0(slice);
        unsigned end       = UNPACK4_1(slice);
#endif


#ifdef USE_TWIDDLE_POSTORDER
        unsigned nodeIdx = beginIdx ; 
        while( nodeIdx != endIdx )
        {
#else
        for(unsigned i=begin ; i < end ; i++)
        {
            unsigned nodeIdx = POSTORDER_NODE(postorder, i) ; // lookup 1-based nodeIdx using 0-based postorder index i
#endif
            unsigned depth = TREE_DEPTH(nodeIdx) ;
            unsigned elevation = height - depth ; 

            Part pt = partBuffer[partOffset+nodeIdx-1]; 
            OpticksCSG_t typecode = (OpticksCSG_t)pt.typecode() ;

            if(typecode == CSG_ZERO) 
            {
#ifdef USE_TWIDDLE_POSTORDER
                // bit-twiddling postorder needs action in tail, even when skipping empties
                nodeIdx = POSTORDER_NEXT( nodeIdx, elevation ) ;
#endif
                continue ; 
            }

            bool primitive = typecode >= CSG_SPHERE ; 

/*
            rtPrintf("[%5d](visi) nodeIdx %2d csg.curr %2d csg_repr %16llx tr_repr %16llx tloop %2d  operation %d primitive %d halfNodes %2d depth %u \n", 
                           launch_index.x, 
                           nodeIdx,
                           csg.curr,
                           csg_repr(csg), 
                           tranche_repr(tr),
                           tloop,  
                           typecode,
                           primitive,
                           halfNodes,
                           depth
                              );
*/

            if(primitive)
            {
                float4 isect = make_float4(0.f, 0.f, 0.f, 0.f) ;  

                csg_intersect_part( prim, partOffset+nodeIdx-1, tmin, isect );

                isect.w = copysignf( isect.w, nodeIdx % 2 == 0 ? -1.f : 1.f );  // hijack t signbit, to record the side, LHS -ve

                ierr = csg_push(csg, isect, nodeIdx ); 
                if(ierr) break ; 
            /*
                rtPrintf("[%5d](prim) nodeIdx %2d csg.curr %2d csg_repr %16llx tr_repr %16llx  (%5.2f,%5.2f,%5.2f,%7.3f)  \n",
                     launch_index.x, 
                     nodeIdx, 
                     csg.curr,
                     csg_repr(csg),
                     tranche_repr(tr),
                     isect.x, isect.y, isect.z, isect.w 
                     );
             */
            }
            else
            {
                if(csg.curr < 1)  // curr 1 : 2 items 
                {
                    rtPrintf("[%5d]evaluative_csg ERROR_POP_EMPTY nodeIdx %4d typecode %d csg.curr %d \n", launch_index.x, nodeIdx, typecode, csg.curr );
                    ierr |= ERROR_POP_EMPTY ; 
                    break ; 
                }
                bool firstLeft = signbit(csg.data[csg.curr].w) ;
                bool secondLeft = signbit(csg.data[csg.curr-1].w) ;


                if(!(firstLeft ^ secondLeft))
                {
#ifdef WITH_PRINT
                    rtPrintf("[%5d]evaluative_csg ERROR_XOR_SIDE nodeIdx %4d typecode %d tl %10.3f tr %10.3f sl %d sr %d \n", launch_index.x, nodeIdx, typecode, csg.data[csg.curr].w, csg.data[csg.curr-1].w, firstLeft, secondLeft );
#endif
                    ierr |= ERROR_XOR_SIDE ; 
                    break ; 
                }
                int left  = firstLeft ? csg.curr   : csg.curr-1 ;
                int right = firstLeft ? csg.curr-1 : csg.curr   ; 

                IntersectionState_t l_state = CSG_CLASSIFY( csg.data[left], ray.direction, tmin );
                IntersectionState_t r_state = CSG_CLASSIFY( csg.data[right], ray.direction, tmin );


                float t_left  = fabsf( csg.data[left].w );
                float t_right = fabsf( csg.data[right].w );

                bool leftIsCloser = t_left <= t_right ;

#define WITH_COMPLEMENT 1
#ifdef WITH_COMPLEMENT
                // complements (signalled by -0.f) cannot Miss, only Exit, see opticks/notes/issues/csg_complement.rst 

                // these are only valid (and only needed) for misses 
                bool l_complement = signbit(csg.data[left].x) ;
                bool r_complement = signbit(csg.data[right].x) ;

                bool l_complement_miss = l_state == State_Miss && l_complement ;
                bool r_complement_miss = r_state == State_Miss && r_complement ;

                if(r_complement_miss)
                {
                    r_state = State_Exit ; 
                    leftIsCloser = true ; 
                } 

                if(l_complement_miss)
                {
                    l_state = State_Exit ; 
                    leftIsCloser = false ; 
                } 

#endif
                int ctrl = boolean_ctrl_packed_lookup( typecode , l_state, r_state, leftIsCloser ) ;
                history_append( hist, nodeIdx, ctrl ); 

                enum { UNDEFINED=0, CONTINUE=1, BREAK=2 } ;

                int act = UNDEFINED ; 

                if(ctrl < CTRL_LOOP_A) // "returning" with a push 
                {
                    float4 result = ctrl == CTRL_RETURN_MISS ?  make_float4(0.f, 0.f, 0.f, 0.f ) : csg.data[ctrl == CTRL_RETURN_A ? left : right] ;
                    if(ctrl == CTRL_RETURN_FLIP_B)
                    {
                        result.x = -result.x ;     
                        result.y = -result.y ;     
                        result.z = -result.z ;     
                    }
                    result.w = copysignf( result.w , nodeIdx % 2 == 0 ? -1.f : 1.f );

                    ierr = csg_pop0(csg); if(ierr) break ;
                    ierr = csg_pop0(csg); if(ierr) break ;
                    ierr = csg_push(csg, result, nodeIdx );  if(ierr) break ;

                    act = CONTINUE ;  
                }
                else
                {                 
                    int loopside  = ctrl == CTRL_LOOP_A ? left : right ;    
                    int otherside = ctrl == CTRL_LOOP_A ? right : left ;  

                    unsigned leftIdx = 2*nodeIdx ; 
                    unsigned rightIdx = leftIdx + 1; 
                    unsigned otherIdx = ctrl == CTRL_LOOP_A ? rightIdx : leftIdx ; 

                    float tminAdvanced = fabsf(csg.data[loopside].w) + propagate_epsilon ;
                    float4 other = csg.data[otherside] ;  // need tmp as pop about to invalidate indices

                    ierr = csg_pop0(csg);                   if(ierr) break ;
                    ierr = csg_pop0(csg);                   if(ierr) break ;
                    ierr = csg_push(csg, other, otherIdx ); if(ierr) break ;

                    // looping is effectively backtracking, pop both and put otherside back

#ifdef USE_TWIDDLE_POSTORDER
                    unsigned endTree   = PACK4(  0,  0,  nodeIdx,  endIdx  );
                    unsigned leftTree  = PACK4(  0,  0,  leftIdx << (elevation-1), rightIdx << (elevation-1)) ;
                    unsigned rightTree = PACK4(  0,  0,  rightIdx << (elevation-1), nodeIdx );
#else
                    unsigned subNodes = TREE_NODES(elevation) ;
                    unsigned halfNodes = (subNodes - 1)/2 ; 
                    unsigned endTree = PACK4( i, end,   nodeIdx, endIdx  );   // BUG FIXED 2017/3/12 changing numNodes->end
                    unsigned leftTree = PACK4( i-2*halfNodes, i-halfNodes,  leftIdx << (elevation-1), rightIdx << (elevation-1)) ;
                    unsigned rightTree = PACK4( i-halfNodes, i ,  rightIdx << (elevation-1), nodeIdx );
#endif
                    unsigned loopTree  = ctrl == CTRL_LOOP_A ? leftTree : rightTree  ;

                    //if(nodeIdx > 2)
                   /*
                    rtPrintf("nodeIdx %2d height %2d depth %2d elevation %2d subNodes %2d halfNodes %2d endTree %8x leftTree %8x rightTree %8x \n",
                              nodeIdx,
                              height,
                              depth,
                              elevation,
                              subNodes,
                              halfNodes,
                              endTree, 
                              leftTree,
                              rightTree);
                    */


                    ierr = tranche_push( tr, endTree, tmin );          if(ierr) break ;
                    ierr = tranche_push( tr, loopTree, tminAdvanced ); if(ierr) break ; 

                    act = BREAK  ;  
                }             // "return" or "recursive call" 
/* 
                rtPrintf("[%5d](ctrl) nodeIdx %2d csg.curr %2d csg_repr %16llx tr_repr %16llx ctrl %d     tloop %2d (%2d->%2d) typecode %d tlr (%10.3f,%10.3f) \n", 
                           launch_index.x, 
                           nodeIdx,
                           csg.curr,
                           csg_repr(csg), 
                           tranche_repr(tr),
                           ctrl,
                           tloop,  
                           POSTORDER_NODE(postorder, begin),
                           POSTORDER_NODE(postorder, end-1),
                           typecode, 
                           t_left, 
                           t_right
                              );
*/


                if(act == BREAK) break ; 
            }                 // "primitive" or "operation"
#ifdef USE_TWIDDLE_POSTORDER
            nodeIdx = POSTORDER_NEXT( nodeIdx, elevation ) ;
#endif
        }                     // node traversal 
        if(ierr) break ; 
     }                       // subtree tranches



    ierr |= (( csg.curr !=  0)  ? ERROR_END_EMPTY : 0)  ; 

    //if(ierr == 0)   // ideally, but for now incude error returns, to see where the problems are
    if(csg.curr == 0)  
    {
        const float4& ret = csg.data[0] ;   
/*
        rtPrintf("[%5d]evaluative_csg ierr %4x ray.origin (%10.3f,%10.3f,%10.3f) ray.direction (%10.3f,%10.3f,%10.3f) ret (%5.2f,%5.2f,%5.3f,%7.3f) \n",
               launch_index.x, ierr, 
               ray.origin.x, ray.origin.y, ray.origin.z,
               ray.direction.x, ray.direction.y, ray.direction.z,
               ret.x, ret.y, ret.z, ret.w );
*/  
 
        if(rtPotentialIntersection( fabsf(ret.w) ))
        {
            shading_normal = geometric_normal = make_float3(ret.x, ret.y, ret.z) ;
            instanceIdentity = identityBuffer[instance_index*primitive_count+primIdx] ;

#ifdef BOOLEAN_DEBUG
            instanceIdentity.x = ierr > 0 ? 1 : 0 ;   // used for visualization coloring  
            instanceIdentity.y = ierr ; 
            // instanceIdentity.z is used for boundary passing, hijacking prevents photon visualization
            instanceIdentity.w = tloop ; 
#endif

//#define WITH_PRINT_IDENTITY_INTERSECT_TAIL 1 
#ifdef WITH_PRINT_IDENTITY_INTERSECT_TAIL
            rtPrintf("// csg_intersect_boolean.h:evaluative_csg WITH_PRINT_IDENTITY_INTERSECT_TAIL repeat_index %d instance_index %d primitive_count %3d primIdx %3d instanceIdentity ( %7d %7d %7d %7d )   \n", 
            repeat_index, instance_index, primitive_count, primIdx, instanceIdentity.x, instanceIdentity.y, instanceIdentity.z, instanceIdentity.w  );  
#endif

            rtReportIntersection(0);
        }
    } 

#ifdef BOOLEAN_DEBUG
        /*
        rtPrintf("[%5d]evaluative_csg ERROR ierr %4x prevNode %2d nodeIdx %2d csg.curr %d tranche %d  ray.direction (%10.3f,%10.3f,%10.3f) ray.origin (%10.3f,%10.3f,%10.3f)   \n",
              launch_index.x, ierr, prevNode, nodeIdx, csg.curr, tranche,
              ray.direction.x, ray.direction.y, ray.direction.z,
              ray.origin.x, ray.origin.y, ray.origin.z
              );
        */

        if(ierr !=0)
        rtPrintf("[%5d](DONE) csg.curr %2d csg_repr %16llx tr_repr %16llx IERR %6x hcur %2d hi %16llx:%16llx hc %16llx:%16llx \n",
                           launch_index.x, 
                           csg.curr,
                           csg_repr(csg), 
                           tranche_repr(tr),
                           ierr, 
                           hist.curr,
                           hist.idx[1], 
                           hist.idx[0], 
                           hist.ctrl[1],
                           hist.ctrl[0]
                             );





#endif
}

/*
intersect_csg  : an early attempt that is not in use ?
--------------------------------------------------------

*/


static __device__
void intersect_csg( const Prim& prim, const uint4& identity )
{

    const unsigned long long postorder_sequence[4] = { 0x1ull, 0x132ull, 0x1376254ull, 0x137fe6dc25ba498ull } ;
    //const unsigned long long pseq4[4] = { 0x103070f1f1e0eull,0x1d1c060d1b1a0c19ull,0x1802050b17160a15ull,0x1404091312081110ull } ;


    int ierr = 0 ;  
    int loop = -1 ; 

    unsigned partOffset = prim.partOffset() ; 
    unsigned numParts   = prim.numParts() ;
    //unsigned tranOffset = prim.tranOffset() ; 

    unsigned fullHeight = __ffs(numParts + 1) - 2 ;   // assumes perfect binary tree node count       2^(h+1) - 1 
    unsigned height = fullHeight - 1;                 // exclude leaves, triplet has height 0

    unsigned long long postorder = postorder_sequence[height] ; 

    unsigned numInternalNodes = (0x1 << (1+height)) - 1 ;

    float4 _tmin ;  // TRANCHE_STACK_SIZE is 4 
    uint4  _tranche ; 
    int tranche = -1 ;

#ifdef BOOLEAN_DEBUG
    int tloop = -1 ; 
#endif

    enum { LHS, RHS };
    CSG csg[2] ; 
    CSG& lhs = csg[LHS] ; 
    CSG& rhs = csg[RHS] ; 
    lhs.curr = -1 ;  
    rhs.curr = -1 ; 

    enum { MISS, LEFT, RIGHT, RFLIP  } ;  // this order is tied to CTRL_ enum, needs rejig of lookup to change 
    float4 isect[4] ;
    isect[MISS]       =  make_float4(0.f, 0.f, 0.f, 0.f);
    isect[LEFT]       =  make_float4(0.f, 0.f, 0.f, 0.f);
    isect[RIGHT]      =  make_float4(0.f, 0.f, 0.f, 0.f);
    isect[RFLIP]      =  make_float4(0.f, 0.f, 0.f, 0.f);

    const float4& miss  = isect[MISS];
    float4& left  = isect[LEFT];
    float4& right = isect[RIGHT];
    float4& rflip = isect[RFLIP];

    TRANCHE_PUSH0( _tranche, _tmin, tranche, ERROR_TRANCHE_OVERFLOW, POSTORDER_SLICE(0, numInternalNodes), ray.tmin );

    while (tranche >= 0)
    {
#ifdef BOOLEAN_DEBUG
         tloop += 1 ;
#endif 
         float   tmin ;
         unsigned tmp ;
         TRANCHE_POP0( _tranche, _tmin, tranche,  tmp, tmin );
         unsigned begin = POSTORDER_BEGIN( tmp );
         unsigned end   = POSTORDER_END( tmp );

         for(unsigned i=begin ; i < end ; i++)
         {
             // XXidx are 1-based levelorder perfect tree indices
             unsigned nodeIdx = POSTORDER_NODE(postorder, i) ;   
             unsigned leftIdx = nodeIdx*2 ; 
             unsigned rightIdx = nodeIdx*2 + 1; 
             int depth = 32 - __clz(nodeIdx)-1 ;  
             unsigned subNodes = (0x1 << (1+height-depth)) - 1 ; // subtree nodes  
             unsigned halfNodes = (subNodes - 1)/2 ;             // nodes to left or right of subtree
             bool bileaf = leftIdx > numInternalNodes ; 

             //quad q2 ; 
             //q2.f = partBuffer[NPART_Q2(partOffset+nodeIdx-1)];      // (nodeIdx-1) as 1-based

             Part pt = partBuffer[partOffset+nodeIdx-1];
             OpticksCSG_t operation = (OpticksCSG_t)pt.typecode() ;

             float tX_min[2] ; 
             tX_min[LHS] = tmin ;
             tX_min[RHS] = tmin ;

             IntersectionState_t x_state[2] ; 

             if(bileaf) // op-left-right leaves
             {
                 // reusing the same storage so clear ahead

                 left.x = 0.f ; 
                 left.y = 0.f ; 
                 left.z = 0.f ; 
                 left.w = 0.f ;  

                 right.x = 0.f ; 
                 right.y = 0.f ; 
                 right.z = 0.f ; 
                 right.w = 0.f ; 

                 csg_intersect_part( prim, partOffset+leftIdx-1 , tX_min[LHS], left  ) ;
                 csg_intersect_part( prim, partOffset+rightIdx-1 , tX_min[RHS], right  ) ;
             }
             else       //  op-op-op
             {
                 CSG_POP( lhs.data, lhs.curr, ERROR_LHS_POP_EMPTY, left );
                 CSG_POP( rhs.data, rhs.curr, ERROR_RHS_POP_EMPTY, right );
             }
 
             x_state[LHS] = CSG_CLASSIFY( left , ray.direction, tX_min[LHS] ) ;
             x_state[RHS] = CSG_CLASSIFY( right, ray.direction, tX_min[RHS] ) ;

             int ctrl = boolean_ctrl_packed_lookup( operation, x_state[LHS], x_state[RHS], left.w <= right.w );
             int side = ctrl - CTRL_LOOP_A ;   // CTRL_LOOP_A,CTRL_LOOP_B -> LHS, RHS   looper side

             bool reiterate = false ; 
             loop = -1  ;  
             while( side > -1 && loop < 10 )
             {
                 loop++ ; 
                 float4& _side = isect[side+LEFT] ; 

                 tX_min[side] = _side.w + propagate_epsilon ;  // classification as well as intersect needs the advance
            
                 if(bileaf)
                 {
                      csg_intersect_part( prim, partOffset+leftIdx+side-1 , tX_min[side], _side  ) ; // tmin advance
                 }
                 else
                 {
                      CSG_POP( csg[side].data, csg[side].curr, ERROR_POP_EMPTY, _side ); // faux recursive call
                 }

                 // reclassification and boolean decision following advancement of one side 
                 x_state[side] = CSG_CLASSIFY( _side, ray.direction, tX_min[side] );
                 ctrl = boolean_ctrl_packed_lookup( operation, x_state[LHS], x_state[RHS], left.w <= right.w );
                 side = ctrl - CTRL_LOOP_A ; 

                 if(side > -1 && !bileaf)
                 {
                     int other = 1 - side ; 
                     tX_min[side] = isect[side+LEFT].w + propagate_epsilon ; 
                     CSG_PUSH( csg[other].data, csg[other].curr, ERROR_OVERFLOW, isect[other+LEFT] );

                     unsigned subtree = side == LHS ? POSTORDER_SLICE(i-2*halfNodes, i-halfNodes) : POSTORDER_SLICE(i-halfNodes, i) ;
                     TRANCHE_PUSH( _tranche, _tmin, tranche, ERROR_TRANCHE_OVERFLOW, POSTORDER_SLICE(i, end), tmin );  // FIX: numInternalNodes->end
                     TRANCHE_PUSH( _tranche, _tmin, tranche, ERROR_TRANCHE_OVERFLOW, subtree , tX_min[side] );
                     reiterate = true ; 
                     break ; 
                 } 
             }  // side loop


             if(reiterate || ierr ) break ;  
             // reiteration needs to get back to tranche loop for subtree traversal 
             // without "return"ing anything

             rflip.x = -right.x ;
             rflip.y = -right.y ;
             rflip.z = -right.z ; 
             rflip.w =  right.w ;

             const float4& result = ctrl < CTRL_LOOP_A ? isect[ctrl] : miss ;   // CTRL_RETURN_*

             int nside = nodeIdx % 2 == 0 ? LHS : RHS ; 
             CSG_PUSH( csg[nside].data, csg[nside].curr, ERROR_RESULT_OVERFLOW, result );

         }  // end for : node traversal within tranche
         if(ierr) break ;
    }       // end while : tranche


    ierr |= (( lhs.curr != -1 ) ? ERROR_LHS_END_NONEMPTY : 0 ) ;  
    ierr |= (( rhs.curr !=  0)  ? ERROR_RHS_END_EMPTY : 0)  ; 

    if(rhs.curr == 0 || lhs.curr == 0)
    {
         const float4& ret = rhs.curr == 0 ? rhs.data[0] : lhs.data[0] ;   // <-- should always be rhs, accept lhs for debug
         if(rtPotentialIntersection( ret.w ))
         {
              shading_normal = geometric_normal = make_float3(ret.x, ret.y, ret.z) ;
              instanceIdentity = identity ;
#ifdef BOOLEAN_DEBUG
              instanceIdentity.x = ierr != 0  ? 1 : instanceIdentity.x ; 
              instanceIdentity.y = tloop == 1 ? 1 : instanceIdentity.y ; 
              instanceIdentity.z = tloop > 1  ? 1 : instanceIdentity.z ; 
#endif
              rtReportIntersection(0);
         }
    } 

    //rtPrintf("intersect_csg partOffset %u numParts %u numInternalNodes %u primIdx_ %u height %u postorder %llx ierr %x \n", partOffset, numParts, numInternalNodes, primIdx_, height, postorder, ierr );
    if(ierr != 0)
    {

#ifdef BOOLEAN_DEBUG
        rtPrintf("intersect_csg u ierr %4x tloop %3d launch_index (%5d,%5d) li.x(26) %2d  \n",
              ierr, tloop, launch_index.x, launch_index.y,  launch_index.x % 26
          );
#endif
#ifdef WITH_PRINT
        rtPrintf("intersect_csg u ierr %4x tloop %3d launch_index (%5d,%5d) li.x(26) %2d ray.direction (%10.3f,%10.3f,%10.3f) ray.origin (%10.3f,%10.3f,%10.3f)   \n",
              ierr, tloop, launch_index.x, launch_index.y,  launch_index.x % 26,
              ray.direction.x, ray.direction.y, ray.direction.z,
              ray.origin.x, ray.origin.y, ray.origin.z
          );
#endif

    } 


}   // intersect_csg





// same interface as csg_intersect_part : intending to blur line
// between primitives and composites to for example facilitate use
// of a composite solid cy-cy bounding check for the torus
//

/*
static __device__
void csg_intersect_bileaf(const Prim& prim, const unsigned partIdx, const float& tt_min, float4& tt  )
{
    unsigned partOffset = prim.partOffset() ; 
    unsigned nodeIdx = 1 ;    
    unsigned leftIdx = nodeIdx*2 ;      
    unsigned rightIdx = nodeIdx*2 + 1 ;  

    Part pt = partBuffer[partOffset+nodeIdx-1] ;
    OpticksCSG_t operation = (OpticksCSG_t)pt.typecode() ;

}

*/



/*
intersect_boolean_triplet
--------------------------

Used during development and for illustrative purposes only.
It demonstrates the algorithm for the special case of a CSG tree of 
three nodes, ie one CSG operation (union/intersection)
and two constituent primitives.

*/
static __device__
void intersect_boolean_triplet( const Prim& prim, const uint4& identity )
{
    unsigned partOffset = prim.partOffset() ; 
    //unsigned tranOffset = prim.tranOffset() ; 

    unsigned nodeIdx = 1 ;    
    unsigned leftIdx = nodeIdx*2 ;      
    unsigned rightIdx = nodeIdx*2 + 1 ;  

    Part pt = partBuffer[partOffset+nodeIdx-1] ;
    OpticksCSG_t operation = (OpticksCSG_t)pt.typecode() ;


    enum { LHS, RHS };
    enum { MISS, LEFT, RIGHT, RFLIP } ;
    float4 isect[4] ;
    isect[MISS]       =  make_float4(0.f, 0.f, 1.f, 0.f);
    isect[LEFT]       =  make_float4(0.f, 0.f, 1.f, 0.f);
    isect[RIGHT]      =  make_float4(0.f, 0.f, 1.f, 0.f);
    isect[RFLIP]      =  make_float4(0.f, 0.f, 1.f, 0.f);

    float4& left  = isect[LEFT];
    float4& right = isect[RIGHT];
    float4& rflip = isect[RFLIP];

    float tX_min[2] ; 
    tX_min[LHS] = ray.tmin ; 
    tX_min[RHS] = ray.tmin ; 

    left.w = 0.f ;   // reusing the same storage so clear ahead
    right.w = 0.f ; 
    csg_intersect_part( prim, partOffset+leftIdx-1 , tX_min[LHS], left  ) ;
    csg_intersect_part( prim, partOffset+rightIdx-1 , tX_min[RHS], right  ) ;

    IntersectionState_t x_state[2] ;

    x_state[LHS] = CSG_CLASSIFY( left, ray.direction, tX_min[LHS] );
    x_state[RHS] = CSG_CLASSIFY( right, ray.direction, tX_min[RHS] );

    int ctrl = boolean_ctrl_packed_lookup( operation, x_state[LHS], x_state[RHS], left.w <= right.w );
    int side = ctrl - CTRL_LOOP_A ; 

    int loop(-1);
    while( side > -1 && loop < 10)
    {
        loop++ ;
        float4& _side = isect[side+LEFT] ;
        tX_min[side] = _side.w + propagate_epsilon ;
        csg_intersect_part( prim, partOffset+leftIdx+side-1 , tX_min[side], _side  ) ; // tmin advanced intersect

        x_state[side] = CSG_CLASSIFY( _side, ray.direction, tX_min[side] );
        ctrl = boolean_ctrl_packed_lookup( operation, x_state[LHS], x_state[RHS], left.w <= right.w );
        side = ctrl - CTRL_LOOP_A ;
    }

    rflip.x = -right.x ;
    rflip.y = -right.y ;
    rflip.z = -right.z ; 
    rflip.w =  right.w ;

    if(ctrl < 4)
    {
       const float4& ret = isect[ctrl] ; 

        if(rtPotentialIntersection(ret.w))
        {
            shading_normal = geometric_normal = make_float3(ret.x, ret.y, ret.z) ;
            instanceIdentity = identity ;

#ifdef BOOLEAN_DEBUG
            instanceIdentity.x = loop  ; 
#endif
            rtReportIntersection(0);
        }
    }
}


