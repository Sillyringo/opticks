AltXJfixtureConstruction
===========================

Looks like Z-shift transforms present in G4VSolid are not getting thru the GeoChain:: 

     geom  ## check that only one entry is uncommented in ~/.opticks/GEOM.txt eg AltXJfixtureConstruction_YZ

 
All the below scripts access the GEOM.txt file to set the geometry to create, visualize or shoot single rays at:: 
  
     gc ; ./run.sh 

     x4 ; ./X4MeshTest.sh    ## CPU : Geant4 polygons visualized with pyvista

     x4 ; ./xxs.sh           ## CPU : 2D Geant4 intersects visualized with matplotlib and/or pyvista

     c ; ./sdf_geochain.sh   ## CPU : 3D Opticks distance field visualised with pyvista iso-surface finding 

     c ; ./csg_geochain.sh   ## CPU : 2D(or 3D) pyvista visualization of Opticks intersects (CPU test run of CUDA comparible intersect code)

     cx ; ./cxr_geochain.sh  ## GPU : 3D OptiX/Opticks render of geometry      



     c ; ./CSGQueryTest.sh   ## CPU : test mostly used for shooting single rays at geometry, useful after compiling with DEBUG flag enabled   




* Added CrossHairs to X4MeshTest.sh and sdf_geochain.sh : clearly zero level in different place



XJFixtureConstruction assert : not expecting more than one level of translation
----------------------------------------------------------------------------------

::

    2022-02-19 15:40:20.119 FATAL [5281713] [GeoChain::convertSolid@65] [
    2022-02-19 15:40:20.119 INFO  [5281713] [GeoChain::convertSolid@67] meta.empty
    X4SolidTree::BooleanClone expect_tla ERROR (not expecting more than one level of translation) 
    X4SolidTree::BooleanClone tla( 0 0 -25) 
    Assertion failed: (expect_tla), function BooleanClone, file /Users/blyth/opticks/extg4/X4SolidTree.cc, line 1943.
    Process 52331 stopped
    * thread #1, queue = 'com.apple.main-thread', stop reason = signal SIGABRT
        frame #0: 0x00007fff72958b66 libsystem_kernel.dylib`__pthread_kill + 10
    libsystem_kernel.dylib`__pthread_kill:
    ->  0x7fff72958b66 <+10>: jae    0x7fff72958b70            ; <+20>
        0x7fff72958b68 <+12>: movq   %rax, %rdi
        0x7fff72958b6b <+15>: jmp    0x7fff7294fae9            ; cerror_nocancel
        0x7fff72958b70 <+20>: retq   
    Target 0: (GeoChainSolidTest) stopped.

    Process 52331 launched: '/usr/local/opticks/lib/GeoChainSolidTest' (x86_64)
    (lldb) bt
        frame #3: 0x00007fff7287c1ac libsystem_c.dylib`__assert_rtn + 320
        frame #4: 0x00000001001b19ed libExtG4.dylib`X4SolidTree::BooleanClone(solid=0x000000010852efe0, depth=1, rot=0x00007ffeefbfcb50, tla=0x00007ffeefbfcb20) at X4SolidTree.cc:1943
        frame #5: 0x00000001001b14ab libExtG4.dylib`X4SolidTree::DeepClone_r(node_=0x000000010852f290, depth=1, rot=0x00007ffeefbfcb50, tla=0x00007ffeefbfcb20) at X4SolidTree.cc:1889
        frame #6: 0x00000001001b1be0 libExtG4.dylib`X4SolidTree::BooleanClone(solid=0x000000010852f1c0, depth=0, rot=0x0000000000000000, tla=0x0000000000000000) at X4SolidTree.cc:1952
        frame #7: 0x00000001001b14ab libExtG4.dylib`X4SolidTree::DeepClone_r(node_=0x000000010852f1c0, depth=0, rot=0x0000000000000000, tla=0x0000000000000000) at X4SolidTree.cc:1889
        frame #8: 0x000000010019fa37 libExtG4.dylib`X4SolidTree::DeepClone(solid=0x000000010852f1c0) at X4SolidTree.cc:1845
        frame #9: 0x000000010019ee2d libExtG4.dylib`X4SolidTree::X4SolidTree(this=0x000000010852fa20, original_=0x000000010852f1c0) at X4SolidTree.cc:59
        frame #10: 0x000000010019dc5d libExtG4.dylib`X4SolidTree::X4SolidTree(this=0x000000010852fa20, original_=0x000000010852f1c0) at X4SolidTree.cc:88
        frame #11: 0x000000010019de6f libExtG4.dylib`X4SolidTree::Draw(original=0x000000010852f1c0, msg="GeoChain::convertSolid original G4VSolid tree") at X4SolidTree.cc:50
        frame #12: 0x00000001000dc54f libGeoChain.dylib`GeoChain::convertSolid(this=0x00007ffeefbfe1a0, solid=0x000000010852f1c0, meta="") at GeoChain.cc:70
        frame #13: 0x000000010000dc22 GeoChainSolidTest`main(argc=1, argv=0x00007ffeefbfe790) at GeoChainSolidTest.cc:83
        frame #14: 0x00007fff72808015 libdyld.dylib`start + 1
    (lldb) 



First : Check with a simpler solid : AnnulusTwoBoxUnionContiguous
-------------------------------------------------------------------


::

     627 const G4VSolid* X4SolidMaker::AnnulusTwoBoxUnion(const char* name)
     628 {
     629     bool contiguous = strstr(name, "Contiguous");
     630     const char* rootname = SStr::Concat("uni133", contiguous ? "_CSG_CONTIGUOUS" : "", "" );
     631     double innerRadius = contiguous ? 0.*mm : 25.*mm ;
     632     // for hinting as CSG_CONTIGUOUS to work requires setting the inner to zero to avoid the CSG_DIFFERENCE
     633     
     634     G4VSolid* down1  = new G4Tubs("down1", innerRadius, 45.*mm, 13./2*mm, 0.*deg, 360.*deg);
     635     G4VSolid* down3 = new G4Box("down3", 15.*mm, 15.*mm, 13/2.*mm);
     636     G4VSolid* uni13 = new G4UnionSolid(  "uni13", down1, down3, 0, G4ThreeVector(0.*mm, 50.*mm, 0.*mm));  // +Y
     637     
     638     LOG(LEVEL) << " name " << name << " contiguous " << contiguous << " rootname " << rootname ;
     639     G4VSolid* uni133 = new G4UnionSolid(rootname, uni13, down3, 0, G4ThreeVector(0.*mm, -50.*mm, 0.*mm)); // -Y 
     640     return uni133 ;  
     641 }   


Find that only one of the boxes is  Y-shifted when use "AnnulusTwoBoxUnionContiguous" 

* maybe its the transform on the node which is converted to a list node that is being lost. 
* actually the box was missing due to lack of setSubOffset for standalone list nodes


Probably loss of transform in::

    079 nmultiunion* nmultiunion::CreateFromTree( OpticksCSG_t type, const nnode* src )  // static 
     80 {   
     81     LOG(LEVEL) << "[" ; 
     82     nnode* subtree = src->deepclone(); 
     83     subtree->prepareTree() ;  // sets parent links and gtransforms by multiplying the transforms 
     84     
     85     unsigned mask = subtree->get_oper_mask(); 
     86     OpticksCSG_t subtree_type = CSG_MonoOperator(mask);
     87     
     88     if(subtree_type != CSG_UNION)
     89     {    
     90          LOG(fatal) << "Can only create nmultiunion from a subtree that is purely composed of CSG_UNION operator nodes" ;
     91          std::raise(SIGINT);
     92     }
     93     
     94     std::vector<nnode*> prim ; 
     95     subtree->collect_prim_for_edit(prim);
     96     
     97     unsigned num_prim = prim.size();  
     98     for(unsigned i=0 ; i < num_prim ; i++)
     99     {   
    100         nnode* p = prim[i];
    101         if( p->gtransform )
    102         {   
    103             p->transform = p->gtransform ;
    104         }
    105     }
    106     
    107     nmultiunion* n = CreateFromList(type, prim) ;
    108     
    109     LOG(LEVEL) << "]" ;
    110     return n ;
    111 }
    112 


After making the boxes bigger, it seems that one of the boxes is missed, or they are both on top of each other. 
After making  the bny box larger in z than the bpy one can confirm that the bny box is missed. 

::

     623 /**
     624 X4SolidMaker::AnnulusTwoBoxUnion
     625 
     626 
     627                tub_bpy_bny
     628 
     629      tub_bpy                  bny 
     630 
     631   tub       bpy 
     632 
     633 
     634 
     635 For hinting as CSG_CONTIGUOUS or CSG_DISCONTIGUOUS to work 
     636 requires setting the inner to zero to avoid the CSG_DIFFERENCE. 
     637 
     638 **/
     639 
     640 
     641 const G4VSolid* X4SolidMaker::AnnulusTwoBoxUnion(const char* name)
     642 {
     643     const char* suffix = nullptr ;
     644     if(     strstr(name, "Contiguous"))    suffix = "_CSG_CONTIGUOUS" ;
     645     else if(strstr(name, "Discontiguous")) suffix = "_CSG_DISCONTIGUOUS" ;
     646     const char* rootname = SStr::Concat("tub_bpy_bny", suffix, "" );     
     647     
     648     double innerRadius = suffix ? 0.*mm : 25.*mm ;
     649     double bpy_scale_z = suffix ? 2. : 1. ; 
     650     double bny_scale_z = suffix ? 4. : 1. ; 
     651     
     652     
     653     LOG(LEVEL)
     654         << " name " << name
     655         << " suffix " << suffix
     656         << " rootname " << rootname
     657         << " innerRadius " << innerRadius
     658         << " bpy_scale_z " << bpy_scale_z
     659         << " bny_scale_z " << bny_scale_z
     660         ;
     661 
     662     G4VSolid* tub  = new G4Tubs("tub", innerRadius, 45.*mm, 13./2*mm, 0.*deg, 360.*deg);
     663     G4VSolid* bpy = new G4Box("bpy", 15.*mm, 15.*mm, bpy_scale_z*13/2.*mm);
     664     G4VSolid* tub_bpy = new G4UnionSolid(  "tub_bpy", tub, bpy, 0, G4ThreeVector(0.*mm, 50.*mm, 0.*mm));  // +Y
     665 
     666     G4VSolid* bny = new G4Box("bny", 15.*mm, 15.*mm, bny_scale_z*13/2.*mm);
     667     G4VSolid* tub_bpy_bny = new G4UnionSolid(rootname, tub_bpy, bny, 0, G4ThreeVector(0.*mm, -50.*mm, 0.*mm)); // -Y 
     668 
     669     return tub_bpy_bny ;
     670 }


Hmm seems offsetSub is still at 0, it should be 1::

    epsilon:CSG blyth$ ./CSGQueryTest.sh 
    === ./CSGQueryTest.sh catgeom AnnulusTwoBoxUnionDiscontiguous_XYZ
    === ./CSGQueryTest.sh catgeom AnnulusTwoBoxUnionDiscontiguous_XYZ geom AnnulusTwoBoxUnionDiscontiguous GEOM AnnulusTwoBoxUnionDiscontiguous
    //distance_node_list num_sub 3 offset_sub 0 isub 0 sub_node.typecode 12 sub_node.typecode.name discontiguous
    Assertion failed: (sub_node->typecode() > CSG_LEAF), function distance_node_list, file /Users/blyth/opticks/CSG/csg_intersect_node.h, line 64.
    ./CSGQueryTest.sh: line 27:  2428 Abort trap: 6           CSGQueryTest $mode
    epsilon:CSG blyth$ 


Fixing that recovers the missing box::

    1322 /**
    1323 NCSG::export_list_
    1324 ----------------------
    1325 
    1326 This is for a standalone list node (NOT for list nodes that are contained within trees). 
    1327 As the list node is standalone the subOffset is set to 1 in order to find the subs 
    1328 which immediately follow the header. 
    1329 
    1330 **/
    1331 
    1332 void NCSG::export_list_()
    1333 {
    1334     unsigned idx = 0 ;
    1335     m_root->setSubOffset(1);
    1336 
    1337     export_node( m_root,  idx) ;
    1338 
    1339     check_subs();
    1340     unsigned sub_num = m_root->subNum();
    1341 
    1342     for(unsigned isub=0 ; isub < sub_num ; isub++)
    1343     {
    1344         idx = 1 + isub ;
    1345         nnode* sub = m_root->subs[isub];
    1346         // sub cannot be const, as the export writes things like indices into the node
    1347         export_node( sub,  idx) ;
    1348     }
    1349 }
    1350 

Seems that now standalone list nodes work with correct transforms, but lists within trees loose all 
their transforms.

For AnnulusTwoBoxUnionContiguous with node list within tree the trIdx for the subs are all zero::

    AnnulusTwoBoxUnionContiguous
    2022-02-19 20:06:03.136 INFO  [5605863] [CSGGeometry::init_fd@110]  booting from provided CSGFoundry pointer 
    2022-02-19 20:06:03.136 INFO  [5605863] [CSGQuery::dumpPrim@370] CSGGeometry::init select_prim_numNode 6 select_nodeOffset 0
    2022-02-19 20:06:03.136 INFO  [5605863] [CSGQuery::dumpPrim@379] CSGNode     0  in aabb:    -0.0    -0.0    -0.0     0.0     0.0     0.0  trIdx:     0 subNum:   3 subOffset::   0
    2022-02-19 20:06:03.136 INFO  [5605863] [CSGQuery::dumpPrim@379] CSGNode     1  co aabb:   -45.0   -65.0   -26.0    45.0    65.0    26.0  trIdx:     1 subNum:   3 subOffset::   3
    2022-02-19 20:06:03.136 INFO  [5605863] [CSGQuery::dumpPrim@379] CSGNode     2 !cy aabb:   -25.0   -25.0    -7.5    25.0    25.0     7.5  trIdx:     2 subNum:  -1 subOffset::  -1
    2022-02-19 20:06:03.136 INFO  [5605863] [CSGQuery::dumpPrim@379] CSGNode     3  cy aabb:   -45.0   -45.0    -6.5    45.0    45.0     6.5  trIdx:     0 subNum:  -1 subOffset::  -1
    2022-02-19 20:06:03.136 INFO  [5605863] [CSGQuery::dumpPrim@379] CSGNode     4  bo aabb:   -15.0   -15.0   -13.0    15.0    15.0    13.0  trIdx:     0 subNum:  -1 subOffset::  -1
    2022-02-19 20:06:03.137 INFO  [5605863] [CSGQuery::dumpPrim@379] CSGNode     5  bo aabb:   -15.0   -15.0   -26.0    15.0    15.0    26.0  trIdx:     0 subNum:  -1 subOffset::  -1
    2022-02-19 20:06:03.137 INFO  [5605863] [CSGGeometry::init_selection@150]  no SXYZ or SXYZW selection 
    2022-02-19 20:06:03.137 INFO  [5605863] [CSGDraw::draw@27] GeoChain::convertSolid converted CSGNode tree axis Y

With standalone List AnnulusTwoBoxUnionContiguousList the trIdx are set::

    2022-02-19 20:32:19.735 INFO  [5635205] [CSGGeometry::init_selection@150]  no SXYZ or SXYZW selection 
    2022-02-19 20:32:19.735 INFO  [5635205] [CSGGeometry::saveSignedDistanceField@159]  name AnnulusTwoBoxUnionContiguousList RESOLUTION 25
    2022-02-19 20:32:19.735 INFO  [5635205] [CSGQuery::dumpPrim@370] CSGQuery::dumpPrim select_prim_numNode 4 select_nodeOffset 0
    2022-02-19 20:32:19.735 INFO  [5635205] [CSGQuery::dumpPrim@379] CSGNode     0  co aabb:   -45.0   -65.0   -26.0    45.0    65.0    26.0  trIdx:     1 subNum:   3 subOffset::   1
    2022-02-19 20:32:19.735 INFO  [5635205] [CSGQuery::dumpPrim@379] CSGNode     1  cy aabb:   -45.0   -45.0    -6.5    45.0    45.0     6.5  trIdx:     2 subNum:  -1 subOffset::  -1
    2022-02-19 20:32:19.735 INFO  [5635205] [CSGQuery::dumpPrim@379] CSGNode     2  bo aabb:   -15.0    35.0   -13.0    15.0    65.0    13.0  trIdx:     3 subNum:  -1 subOffset::  -1
    2022-02-19 20:32:19.735 INFO  [5635205] [CSGQuery::dumpPrim@379] CSGNode     3  bo aabb:   -15.0   -65.0   -26.0    15.0   -35.0    26.0  trIdx:     4 subNum:  -1 subOffset::  -1
    2022-02-19 20:32:19.735 INFO  [5635205] [*CSGQuery::scanPrim@358]  ce ( 0.000, 0.000, 0.000,65.000)  resolution 25




Adding handling for list nodes within trees to NCSG::collect_global_transforms_r succeeds to include all transforms, 
but seeing spurious intersects onto the subtracted inner tubs.
Spurs look to be mostly from gensteps with direct sightlines to the subtracted tubs. 


With GEOM AnnulusTwoBoxUnionContiguous_YZ for listnode to mop up everything other than the subtracted inner
-----------------------------------------------------------------------------------------------------------

::

    SPUR=1 IXYZ=0,0,2 ./csg_geochain.sh ana
    SPUR=1 IXYZ=0,1,2 ./csg_geochain.sh ana
    SPUR=1 IXYZ=0,2,2 ./csg_geochain.sh ana
         seems to be 2nd crossings showing up as spurious
         with inwards normal showing up (from the complemented)

    SPUR=1 IXYZ=0,0,0 ./csg_geochain.sh ana
         only from rays that are also in line with the end boxes 

    IXYZ=0,1,2  ./csg_geochain.sh ana 
         ~1/20 spurs, DONE: color spur rays different
  
    IXYZ=0,-2,1 ./csg_geochain.sh ana 


::

    SXYZW=0,-2,1,93 ./csg_geochain.sh run


Pick a spurious intersect to delve into:: 

    IXYZ=0,8,8  ./csg_geochain.sh ana 

    SXYZW=0,8,8,61 ./csg_geochain.sh run 


HMM: get CSGRecord to work again to dive into the single intersect CSG decision without 
needing such laborious manual debugging




With GEOM AnnulusTwoBoxUnionContiguousList_YZ to just show the standalone listnode
------------------------------------------------------------------------------------

No spurs in either Opticks or G4 


WIth GEOM AnnulusTwoBoxUnionDiscontiguous_YZ using simple first ENTER/EXIT intersect on listnode
--------------------------------------------------------------------------------------------------

No spurs other than the expected internals. 


With GEOM AnnulusTwoBoxUnion_YZ for ordinary tree
----------------------------------------------------

G4 spurs on line between end boxes and tubs::

    ./xxs.sh 

No spurs with Opticks::

    c ; ./csg_geochain.sh 




  

Getting an inkling of the cause of problems
-----------------------------------------------

Comparing intersects from the tree with nodelist and standalone list for a genstep with spurs in the tree starts to provide an inkling.::

   AnnulusTwoBoxUnionContiguous_YZ
   AnnulusTwoBoxUnionContiguousList_YZ



::

   IXYZ=0,-3,-1 ./csg_geochain.sh ana 


The nodelist is always giving the furthest exit. Which misses intermediate exits breaking the CSG tree intersect alg.

Suspect by that working first with "first-exits" will allow disjointed exits to be disqualified. 


NEW IMPLEMENTATION THAT DISQUALIFIES DISJOINT BY PUSHING THE ENVELOPE
-------------------------------------------------------------------------


1. first pass : find farthest first exits and mark subs that are Miss or Exit as done 
2. second pass : redo the Enter intersects and loop them to get Exits 


::

    353     else   // FROM INSIDE
    354     {
    355         // *first pass* : find furthest first exit and update done vector for state Exit and Miss 
    356         for(unsigned isub=0 ; isub < num_sub ; isub++)
    357         {
    358             const CSGNode* sub_node = root+offset_sub+isub ;
    359             if(intersect_leaf( sub_isect, sub_node, plan, itra, t_min, ray_origin, ray_direction ))
    360             {
    361                 sub_state = CSG_CLASSIFY( sub_isect, ray_direction, t_min );
    362 
    363                 if( sub_state == State_Exit)
    364                 {
    365                     exit_count += 1 ;
    366                     if( sub_isect.w > farthest_exit.w ) farthest_exit = sub_isect ;
    367                 }
    368 
    369                 if( sub_state == State_Exit || sub_state == State_Miss ) done |= ( 0x1 << isub ) ;
    370                 // only subs with State_Enter are left undone 
    371             }
    372         }
    373 
    374         // *second pass* : redo the undone that are all State_Enter 
    375         for(unsigned isub=0 ; isub < num_sub ; isub++)
    376         {
    377             if((done & ( 0x1 << isub )) == 0)   // sub us undone : only state Enter need intersecting again
    378             {
    379                 const CSGNode* sub_node = root+offset_sub+isub ;
    380 
    381                 if(intersect_leaf( sub_isect, sub_node, plan, itra, t_min, ray_origin, ray_direction ))
    382                 {
    387                     // only State_Enter within envelope are permissable,  otherwise isub is disqualified as disjoint  
    388                     // HMM: suspect there is dependency on sub order here ... so need another pass to avoid ?
    389                     //  think about a long line of subs which keeps pushing the envelope 
    390                     // TODO: test with a line of boxes where deliberatately shuffle the sub order
    391 
    392                     if( sub_isect.w < farthest_exit.w )
    393                     {
    394                         float tminAdvanced = sub_isect.w + propagate_epsilon ;
    395                         if(intersect_leaf( sub_isect, sub_node, plan, itra, tminAdvanced , ray_origin, ray_direction ))
    396                         {
    397                             sub_state = CSG_CLASSIFY( sub_isect, ray_direction, tminAdvanced );
    398                             if( sub_state == State_Exit )
    399                             {
    400                                 exit_count += 1 ;
    401                                 if( sub_isect.w > farthest_exit.w ) farthest_exit = sub_isect ;
    402                             }
    ...
    409                         }
    410                    }      // within the envelope 
    411                }          // redoing the Enter intersect    
    412            }              // sub is undone 
    413        }                  // over subs 
    414     }                     // end INSIDE branch 
    415 
    418     bool valid_intersect = false ;
    419     if( inside_or_surface )
    420     {
    421         valid_intersect = exit_count > 0 && farthest_exit.w > t_min ;
    422         if(valid_intersect) isect = farthest_exit ;
    423     }
    424     else
    425     {
    426         valid_intersect = enter_count > 0 && nearest_enter.w > t_min ;
    427         if(valid_intersect) isect = nearest_enter ;
    428     }
    429 
    430 #ifdef DEBUG
    431      printf("//intersect_node_contiguous valid_intersect %d  (%10.4f %10.4f %10.4f %10.4f) \n", valid_intersect, isect.x, isect.y, isect.z, isect.w );
    432 #endif
    433 
    434     return valid_intersect ;




Hmm need to sort indices of enter distances like np.argsort

* https://github.com/numpy/numpy/tree/main/numpy/core/src/npysort


Spurs from gensteps at extremities in XY projection::

    GEOM=AltXJfixtureConstruction_XY IXYZ=-4,0,0 ./csg_geochain.sh 







