skipping_solids_by_name
===========================



Feature
---------

Want to skip the virtual "hat boxes" surrounding PMTs for the GPU geometry.
But do not want to use raw lvidx indices as they go stale too fast with
geometry updates. So need to get from mesh/solid name to idx at OpticksDbg level.
Normally geometry info is not available down there.


Approach
-----------

1. introduce sysrap/SGeo pure virtual base protocol for ggeo/GGeo to inherit from
2. arrange for Opticks to hold onto SGeo instance set with `Opticks::setGeo(const SGeo*)`
   and add `OpticksDbg::postgeometry` to act (translating names into indices) when
   the geometry becomes available.

3. invoke that setter from `GGeo::deferred` after loading or creating geometry::

     594 void GGeo::deferred()
     595 {
     596     m_ok->setGeo((SGeo*)this);   //  for access to limited geometry info from lower levels
     597     deferredCreateGParts();
     598 }

4. provide accessor via Opticks::

     758 bool Opticks::isSkipSolidIdx(unsigned lvIdx) const  // --skipsolidname
     759 {
     760     return m_dbg->isSkipSolidIdx(lvIdx);
     761 }

5. use that from `GParts::Create`::


     220 GParts* GParts::Create(const Opticks* ok, const GPts* pts, const std::vector<const NCSG*>& solids, unsigned& num_mismatch_pt, std::vector<glm::mat4>* mismatch_placements ) // static
     221 {
     222     LOG(LEVEL) << "[  deferred creation from GPts" ;
     223
     224     GParts* com = new GParts() ;
     225
     226     unsigned num_pt = pts->getNumPt();
     227
     228     LOG(LEVEL) << " num_pt " << num_pt ;
     229
     230     unsigned verbosity = 0 ;
     231
     232     std::vector<unsigned> mismatch_pt ;
     233
     234     for(unsigned i=0 ; i < num_pt ; i++)
     235     {
     236         const GPt* pt = pts->getPt(i);
     237         int   lvIdx = pt->lvIdx ;
     238
     239         bool deferredcsgskiplv = ok->isDeferredCSGSkipLV(lvIdx); //  --deferredcsgskiplv
     240         bool skipsolidname     = ok->isSkipSolidIdx(lvIdx);      //   --skipsolidname
     241
     242         if(deferredcsgskiplv || skipsolidname)
     243         {
     244             LOG(info)
     245                 << " SKIPPING SOLID FROM ANALYTIC GEOMETRY VIA COMMANDLINE OPTION "
     246                 << " i " << i
     247                 << " num_pt " << num_pt
     248                 << " lvIdx " << lvIdx
     249                 << " deferredcsgskiplv " << deferredcsgskiplv
     250                 << " skipsolidname " << skipsolidname
     251                 ;
     252             continue ;
     253         }




Testing
----------

::

    epsilon:opticks blyth$ ggeo.sh 1:5/ --names
    python3 /Users/blyth/opticks/ana/ggeo.py 1:5/ --names
    nrpo( 176632     1     0     0 )                        PMT_3inch_log_phys0x4437d00                             PMT_3inch_log0x4436df0  114 PMT_3inch_pmt_solid0x4436210
    nrpo( 176633     1     0     1 )                       PMT_3inch_body_phys0x4437230                        PMT_3inch_body_log0x4436ce0  112 PMT_3inch_body_solid_ell_ell_helper0x44364d0
    nrpo( 176634     1     0     2 )                     PMT_3inch_inner1_phys0x44372b0                      PMT_3inch_inner1_log0x4436f00  110 PMT_3inch_inner1_solid_ell_helper0x4436560
    nrpo( 176635     1     0     3 )                     PMT_3inch_inner2_phys0x4437360                      PMT_3inch_inner2_log0x4437010  111 PMT_3inch_inner2_solid_ell_helper0x4436640
    nrpo( 176636     1     0     4 )                       PMT_3inch_cntr_phys0x4437410                        PMT_3inch_cntr_log0x4437120  113 PMT_3inch_cntr_solid0x44366d0
    nrpo(  70960     2     0     0 )                         pLPMT_NNVT_MCPPMT0x3cbba60                    NNVTMCPPMTlMaskVirtual0x3cb41a0  103 NNVTMCPPMTsMask_virtual0x3cb3b40
    nrpo(  70961     2     0     1 )                           NNVTMCPPMTpMask0x3c9fe00                           NNVTMCPPMTlMask0x3c9fc80   98 NNVTMCPPMTsMask0x3c9fa80
    nrpo(  70962     2     0     2 )            NNVTMCPPMT_PMT_20inch_log_phys0x3c9fe80                 NNVTMCPPMT_PMT_20inch_log0x3caec40  102 NNVTMCPPMT_PMT_20inch_pmt_solid0x3ca9320
    nrpo(  70963     2     0     3 )           NNVTMCPPMT_PMT_20inch_body_phys0x3caefa0            NNVTMCPPMT_PMT_20inch_body_log0x3caeb60  101 NNVTMCPPMT_PMT_20inch_body_solid0x3cad240
    nrpo(  70964     2     0     4 )         NNVTMCPPMT_PMT_20inch_inner1_phys0x3caf030          NNVTMCPPMT_PMT_20inch_inner1_log0x3caed60   99 NNVTMCPPMT_PMT_20inch_inner1_solid_1_Ellipsoid0x3503950
    nrpo(  70965     2     0     5 )         NNVTMCPPMT_PMT_20inch_inner2_phys0x3caf0f0          NNVTMCPPMT_PMT_20inch_inner2_log0x3caee80  100 NNVTMCPPMT_PMT_20inch_inner2_solid0x3cae8f0
    nrpo(  70966     3     0     0 )                    pLPMT_Hamamatsu_R128600x3cbbae0               HamamatsuR12860lMaskVirtual0x3c9a5c0  109 HamamatsuR12860sMask_virtual0x3c99fb0
    nrpo(  70967     3     0     1 )                      HamamatsuR12860pMask0x3c9b320                      HamamatsuR12860lMask0x3c9b1a0  104 HamamatsuR12860sMask0x3c9afa0
    nrpo(  70968     3     0     2 )       HamamatsuR12860_PMT_20inch_log_phys0x3c9b3b0            HamamatsuR12860_PMT_20inch_log0x3c93920  108 HamamatsuR12860_PMT_20inch_pmt_solid_1_90x3cb68e0
    nrpo(  70969     3     0     3 )      HamamatsuR12860_PMT_20inch_body_phys0x345b3c0       HamamatsuR12860_PMT_20inch_body_log0x3c93830  107 HamamatsuR12860_PMT_20inch_body_solid_1_90x3ca7680
    nrpo(  70970     3     0     4 )    HamamatsuR12860_PMT_20inch_inner1_phys0x3c94040     HamamatsuR12860_PMT_20inch_inner1_log0x345b160  105 HamamatsuR12860_PMT_20inch_inner1_solid_I0x3c96fa0
    nrpo(  70971     3     0     5 )    HamamatsuR12860_PMT_20inch_inner2_phys0x3c94100     HamamatsuR12860_PMT_20inch_inner2_log0x345b290  106 HamamatsuR12860_PMT_20inch_inner2_solid_1_90x3c93610
    nrpo( 304636     4     0     0 )     mask_PMT_20inch_vetolMaskVirtual_phys0x4433460          mask_PMT_20inch_vetolMaskVirtual0x3ca10e0  126 mask_PMT_20inch_vetosMask_virtual0x3ca0a80
    nrpo( 304637     4     0     1 )                 mask_PMT_20inch_vetopMask0x3ca1e40                 mask_PMT_20inch_vetolMask0x3ca1cb0  121 mask_PMT_20inch_vetosMask0x3ca1aa0
    nrpo( 304638     4     0     2 )                  PMT_20inch_veto_log_phys0x3ca5fa0                       PMT_20inch_veto_log0x3ca5470  125 PMT_20inch_veto_pmt_solid_1_20x3ca38b0
    nrpo( 304639     4     0     3 )                 PMT_20inch_veto_body_phys0x3ca57a0                  PMT_20inch_veto_body_log0x3ca5360  124 PMT_20inch_veto_body_solid_1_20x3ca4230
    nrpo( 304640     4     0     4 )               PMT_20inch_veto_inner1_phys0x3ca5820                PMT_20inch_veto_inner1_log0x3ca5580  122 PMT_20inch_veto_inner1_solid0x3ca4f10
    nrpo( 304641     4     0     5 )               PMT_20inch_veto_inner2_phys0x3ca58d0                PMT_20inch_veto_inner2_log0x3ca5690  123 PMT_20inch_veto_inner2_solid0x3ca5130
    epsilon:opticks blyth$
    epsilon:opticks blyth$


Collect virtual mask names::

   103 NNVTMCPPMTsMask_virtual0x3cb3b40
   109 HamamatsuR12860sMask_virtual0x3c99fb0
   126 mask_PMT_20inch_vetosMask_virtual0x3ca0a80

Check with GMeshLibTest::

    GMeshLibTest 103 109 126

    epsilon:issues blyth$ GMeshLibTest 103 109 126
    NNVTMCPPMTsMask_virtual0x3cb3b40
    HamamatsuR12860sMask_virtual0x3c99fb0
    mask_PMT_20inch_vetosMask_virtual0x3ca0a80
    epsilon:issues blyth$

    epsilon:issues blyth$ GMeshLibTest NNVTMCPPMTsMask_virtual0x HamamatsuR12860sMask_virtual0x mask_PMT_20inch_vetosMask_virtual0x
    103
    109
    126



So the option to use is::

    --skipsolidname NNVTMCPPMTsMask_virtual0x,HamamatsuR12860sMask_virtual0x,mask_PMT_20inch_vetosMask_virtual0x

Check that with::

    OpticksDbg=INFO GGeoTest --skipsolidname NNVTMCPPMTsMask_virtual0x,HamamatsuR12860sMask_virtual0x,mask_PMT_20inch_vetosMask_virtual0x

    epsilon:ggeo blyth$ OpticksDbg=INFO GGeoTest --skipsolidname NNVTMCPPMTsMask_virtual0x,HamamatsuR12860sMask_virtual0x,mask_PMT_20inch_vetosMask_virtual0x
    PLOG::EnvLevel adjusting loglevel by envvar   key OpticksDbg level INFO fallback DEBUG
    2021-04-23 14:42:36.618 INFO  [29545435] [OpticksDbg::postconfigure@229]  spec ~0 SBit::PosString(bitfield,',',true) ~0p,
    2021-04-23 14:42:38.005 INFO  [29545435] [OpticksDbg::postgeometry@179] [
    2021-04-23 14:42:38.005 INFO  [29545435] [OpticksDbg::postgeometry@197]  midx  103 sn [NNVTMCPPMTsMask_virtual0x]
    2021-04-23 14:42:38.005 INFO  [29545435] [OpticksDbg::postgeometry@197]  midx  109 sn [HamamatsuR12860sMask_virtual0x]
    2021-04-23 14:42:38.005 INFO  [29545435] [OpticksDbg::postgeometry@197]  midx  126 sn [mask_PMT_20inch_vetosMask_virtual0x]
    2021-04-23 14:42:38.005 INFO  [29545435] [OpticksDbg::postgeometry@208]  --skipsolidname NNVTMCPPMTsMask_virtual0x,HamamatsuR12860sMask_virtual0x,mask_PMT_20inch_vetosMask_virtual0x solidname.size 3 soidx.size 3
    2021-04-23 14:42:38.005 INFO  [29545435] [OpticksDbg::postgeometry@215] ]
    2021-04-23 14:42:38.309 INFO  [29545435] [*GParts::Create@244]  SKIPPING SOLID FROM ANALYTIC GEOMETRY VIA COMMANDLINE OPTION  i 0 num_pt 6 lvIdx 103 deferredcsgskiplv 0 skipsolidname 1
    2021-04-23 14:42:38.310 INFO  [29545435] [*GParts::Create@244]  SKIPPING SOLID FROM ANALYTIC GEOMETRY VIA COMMANDLINE OPTION  i 0 num_pt 6 lvIdx 109 deferredcsgskiplv 0 skipsolidname 1
    2021-04-23 14:42:38.311 INFO  [29545435] [*GParts::Create@244]  SKIPPING SOLID FROM ANALYTIC GEOMETRY VIA COMMANDLINE OPTION  i 0 num_pt 6 lvIdx 126 deferredcsgskiplv 0 skipsolidname 1
    2021-04-23 14:42:38.317 INFO  [29545435] [test_getNumMeshes@326]  num_meshes_ggeo 137 num_meshes_sgeo 137
    epsilon:ggeo blyth$




Test on Precision
-------------------

Ahha the geocache in use on O seems not to have references on the names::

    O[blyth@localhost opticks]$ GMeshLibTest 103 109 126
    NNVTMCPPMTsMask_virtual
    HamamatsuR12860sMask_virtual
    mask_PMT_20inch_vetosMask_virtual


::

    O[blyth@localhost bin]$ LIMIT=1 jflight.sh

    /home/blyth/local/opticks/lib/OpFlightPathTest
    /home/blyth/opticks/bin
    === flight-render-jpg : creating output directory outdir: /tmp/blyth/opticks/flight/RoundaboutZX__lLowerChimney_phys__~5,__8__
    OpFlightPathTest --targetpvn lLowerChimney_phys --flightconfig "flight=RoundaboutZX,ext=.jpg,scale0=3,scale1=0.5,framelimit=1,period=8" --flightoutdir "/tmp/blyth/opticks/flight/RoundaboutZX__lLowerChimney_phys__~5,__8__" --nameprefix "RoundaboutZX__lLowerChimney_phys__~5,__8__" -e "~5," --rtx 1 --cvd 1 --skipsolidname NNVTMCPPMTsMask_virtual,HamamatsuR12860sMask_virtual,mask_PMT_20inch_vetosMask_virtual
    PLOG::EnvLevel adjusting loglevel by envvar   key OpticksDbg level INFO fallback DEBUG
    2021-04-23 22:41:40.622 INFO  [354540] [Opticks::postconfigure@2717]  setting CUDA_VISIBLE_DEVICES envvar internally to 1
    2021-04-23 22:41:40.626 INFO  [354540] [OpticksDbg::postconfigure@229]  spec ~5, SBit::PosString(bitfield,',',true) ~0p5,
    2021-04-23 22:41:40.626 INFO  [354540] [OpticksHub::loadGeometry@284] [ /home/blyth/.opticks/geocache/DetSim0Svc_pWorld_g4live/g4ok_gltf/85d8514854333c1a7c3fd50cc91507dc/1
    2021-04-23 22:41:41.339 INFO  [354540] [OpticksDbg::postgeometry@179] [
    2021-04-23 22:41:41.339 INFO  [354540] [OpticksDbg::postgeometry@197]  midx  103 sn [NNVTMCPPMTsMask_virtual]
    2021-04-23 22:41:41.339 INFO  [354540] [OpticksDbg::postgeometry@197]  midx  109 sn [HamamatsuR12860sMask_virtual]
    2021-04-23 22:41:41.339 INFO  [354540] [OpticksDbg::postgeometry@197]  midx  126 sn [mask_PMT_20inch_vetosMask_virtual]
    2021-04-23 22:41:41.339 INFO  [354540] [OpticksDbg::postgeometry@208]  --skipsolidname NNVTMCPPMTsMask_virtual,HamamatsuR12860sMask_virtual,mask_PMT_20inch_vetosMask_virtual solidname.size 3 soidx.size 3
    2021-04-23 22:41:41.339 INFO  [354540] [OpticksDbg::postgeometry@215] ]
    2021-04-23 22:41:41.700 INFO  [354540] [GParts::Create@244]  SKIPPING SOLID FROM ANALYTIC GEOMETRY VIA COMMANDLINE OPTION  i 0 num_pt 6 lvIdx 103 deferredcsgskiplv 0 skipsolidname 1
    2021-04-23 22:41:41.701 INFO  [354540] [GParts::Create@244]  SKIPPING SOLID FROM ANALYTIC GEOMETRY VIA COMMANDLINE OPTION  i 0 num_pt 6 lvIdx 109 deferredcsgskiplv 0 skipsolidname 1
    2021-04-23 22:41:41.702 INFO  [354540] [GParts::Create@244]  SKIPPING SOLID FROM ANALYTIC GEOMETRY VIA COMMANDLINE OPTION  i 0 num_pt 6 lvIdx 126 deferredcsgskiplv 0 skipsolidname 1
    2021-04-23 22:41:41.731 INFO  [354540] [OpticksHub::loadGeometry@316] ]
    2021-04-23 22:41:41.754 INFO  [354540] [OContext::InitRTX@342]  --rtx 1 setting  ON
    2021-04-23 22:41:41.833 INFO  [354540] [OContext::CheckDevices@226]
    Device 0                      TITAN RTX ordinal 0 Compute Support: 7 5 Total Memory: 25396445184
    ...
    2021-04-23 22:41:42.073 INFO  [354540] [OGeo::convert@302] [ nmm 10
    OpFlightPathTest: /home/blyth/opticks/optixrap/OGeo.cc:806: optix::Geometry OGeo::makeAnalyticGeometry(GMergedMesh*): Assertion `idBuf->hasShape(numInstances,numPrim,4)' failed.
    /home/blyth/local/opticks/bin/flight.sh: line 85: 354540 Aborted                 (core dumped) OpFlightPathTest --targetpvn lLowerChimney_phys --flightconfig "flight=RoundaboutZX,ext=.jpg,scale0=3,scale1=0.5,framelimit=1,period=8" --flightoutdir "/tmp/blyth/opticks/flight/RoundaboutZX__lLowerChimney_phys__~5,__8__" --nameprefix "RoundaboutZX__lLowerChimney_phys__~5,__8__" -e "~5," --rtx 1 --cvd 1 --skipsolidname NNVTMCPPMTsMask_virtual,HamamatsuR12860sMask_virtual,mask_PMT_20inch_vetosMask_virtual
    === flight-render-jpg : rc 134
    /tmp/blyth/opticks/flight/RoundaboutZX__lLowerChimney_phys__~5,__8__
    /home/blyth/junotop/ExternalLibs/Opticks/0.0.0-rc1/bashrc: line 4: /home/blyth/junotop/



The skip trips idBuf assert::

    2021-04-23 22:51:04.132 INFO  [370437] [OGeo::makeAnalyticGeometry@767]  skip GParts::close
    2021-04-23 22:51:04.132 INFO  [370437] [OGeo::makeAnalyticGeometry@770] mm 2 verbosity: 0   pts:  GParts  primflag         flagnodetree numParts   29 numPrim    5
    2021-04-23 22:51:04.132 INFO  [370437] [OGeo::makeAnalyticGeometry@797]  mmidx 2 numInstances 12612 numPrim 5 idBuf 12612,6,4
    2021-04-23 22:51:04.132 FATAL [370437] [OGeo::makeAnalyticGeometry@808]  UNEXPECTED  idBuf 12612,6,4 numInstance 12612 numPrim 5 mm.index 2
    OpFlightPathTest: /home/blyth/opticks/optixrap/OGeo.cc:816: optix::Geometry OGeo::makeAnalyticGeometry(GMergedMesh*): Assertion `expect' failed.
    /home/blyth/local/opticks/bin/flight.sh: line 85: 370437 Aborted                 (core dumped) OpFlightPathTest --targetpvn lLowerChimney_phys --flightconfig "flight=RoundaboutZX,ext=.jpg,scale0=3,scale1=0


OGeo inconsistency between GParts and GMergedMesh, idBuf still has 6 prim without the skip::

     785
     786     NPY<float>*     partBuf = pts->getPartBuffer(); assert(partBuf && partBuf->hasShape(-1,4,4));    // node buffer
     787     NPY<float>*     tranBuf = pts->getTranBuffer(); assert(tranBuf && tranBuf->hasShape(-1,3,4,4));  // transform triples (t,v,q)
     788     NPY<float>*     planBuf = pts->getPlanBuffer(); assert(planBuf && planBuf->hasShape(-1,4));      // planes used for convex polyhedra such as trapezoid
     789     NPY<int>*       primBuf = pts->getPrimBuffer(); assert(primBuf && primBuf->hasShape(-1,4));      // prim
     790
     791     // NB these buffers are concatenations of the corresponding buffers for multiple prim
     792     unsigned numPrim = primBuf->getNumItems();
     793
     794     NPY<float>* itransforms = mm->getITransformsBuffer(); assert(itransforms && itransforms->hasShape(-1,4,4) ) ;
     795     unsigned numInstances = itransforms->getNumItems();
     796     NPY<unsigned>*  idBuf = mm->getInstancedIdentityBuffer();   assert(idBuf);
     797     LOG(LEVEL)
     798         << " mmidx " << mm->getIndex()
     799         << " numInstances " << numInstances
     800         << " numPrim " << numPrim
     801         << " idBuf " << idBuf->getShapeString()
     802         ;
     803
     804     if( mm->getIndex() > 0 )  // volume level buffers do not honour selection unless using globalinstance
     805     {
     806         bool expect = idBuf->hasShape(numInstances,numPrim,4) ;
     807         if(!expect)
     808             LOG(fatal)
     809                 << " UNEXPECTED "
     810                 << " idBuf " << idBuf->getShapeString()
     811                 << " numInstance " << numInstances
     812                 << " numPrim " << numPrim
     813                 << " mm.index " << mm->getIndex()
     814                 ;
     815
     816         assert(expect);
     817     }



::

    1186 void GMergedMesh::addInstancedBuffers(const std::vector<const GNode*>& placements)
    1187 {
    1188     LOG(LEVEL) << " placements.size() " << placements.size() ;
    1189
    1190     NPY<float>* itransforms = GTree::makeInstanceTransformsBuffer(placements);
    1191     setITransformsBuffer(itransforms);
    1192
    1193     NPY<unsigned int>* iidentity  = GTree::makeInstanceIdentityBuffer(placements);
    1194     setInstancedIdentityBuffer(iidentity);
    1195 }
    1196


::

    epsilon:2 blyth$ ipython

    In [1]: a = np.load("placement_iidentity.npy")

    In [2]: a
    Out[2]:
    array([[[   70960, 33554432,  6750230,        0],
            [   70961, 33554433,  6422544,        0],
            [   70962, 33554434,  6684695,        0],
            [   70963, 33554435,  6619160,        0],
            [   70964, 33554436,  6488089,        1],
            [   70965, 33554437,  6553626,        0]],

           [[   70972, 33554688,  6750230,        0],



::

    256 glm::uvec4 GVolume::getIdentity() const
    257 {
    258     glm::uvec4 id(getIndex(), getTripletIdentity(), getShapeIdentity(), getSensorIndex()) ;
    259     return id ;
    260 }



    262 /**
    263 GVolumne::getShapeIdentity
    264 ----------------------------
    265
    266 The shape identity packs mesh index and boundary index together.
    267 This info is used GPU side by::
    268
    269    oxrap/cu/material1_propagate.cu:closest_hit_propagate
    270
    271 ::
    272
    273     id = np.load("all_volume_identity.npy")
    274
    275     bidx = ( id[:,2] >>  0)  & 0xffff )
    276     midx = ( id[:,2] >> 16)  & 0xffff )
    277
    278
    279 **/
    280
    281 unsigned GVolume::getShapeIdentity() const
    282 {
    283     return OpticksShape::Encode( getMeshIndex(), getBoundary() );
    284 }


    In [1]: a = np.load("placement_iidentity.npy")

    In [2]: a.shape
    Out[2]: (12612, 6, 4)

    In [3]: a[0]
    Out[3]:
    array([[   70960, 33554432,  6750230,        0],
           [   70961, 33554433,  6422544,        0],
           [   70962, 33554434,  6684695,        0],
           [   70963, 33554435,  6619160,        0],
           [   70964, 33554436,  6488089,        1],
           [   70965, 33554437,  6553626,        0]], dtype=uint32)

    In [4]: a[0,:,2]
    Out[4]: array([6750230, 6422544, 6684695, 6619160, 6488089, 6553626], dtype=uint32)

    In [5]: ( a[0,:,2] >> 16 ) & 0xffff
    Out[5]: array([103,  98, 102, 101,  99, 100], dtype=uint32)

    In [6]:



Hmm doing the skipping postcache would entail changing the idBuf on the fly.
Simpler to make the skips at geocache creation, then have no complicated contortions
or need to commandline options to skip.



That means need to go back to **csgskip**::

     354 void GMergedMesh::traverse_r( const GNode* node, unsigned depth, unsigned pass )
     355 {
     356     const GVolume* volume = dynamic_cast<const GVolume*>(node) ;
     357 
     358     int idx = getIndex() ;
     359     assert(idx > -1 ) ;
     360     //unsigned uidx = m_globalinstance ? 0u : idx ;                 // needed as globalinstance goes into top slot (nmm-1) 
     361 
     362     unsigned uidx = idx ;
     363     unsigned ridx = volume->getRepeatIndex() ;
     364 
     365     bool repeat_selection =  ridx == uidx ;                       // repeatIndex of volume same as index of mm (or 0 for globalinstance)
     366     bool csgskip = volume->isCSGSkip() ;                          // --csgskiplv : for DEBUG usage only 
     367     bool selected_ =  volume->isSelected() && repeat_selection ;  // volume selection defaults to true and appears unused
     368     bool selected = selected_ && !csgskip ;                       // selection honoured by both triangulated and analytic 
     369 
     370 




Check tds on Precision, first without the skips
--------------------------------------------------

1. O: update Opticks
2. P: jre ; jok-all
3. P: tds 

New KEY is reported::

    export OPTICKS_KEY=DetSim0Svc.X4PhysicalVolume.pWorld.7ca7500a89b9fd1f5e354090a7758692 

::

    144 ok_juno_tds_apr23_2021(){ export OPTICKS_KEY=DetSim0Svc.X4PhysicalVolume.pWorld.7ca7500a89b9fd1f5e354090a7758692  ; }
    145 
    146 #ok_dyb
    147 #ok_juno
    148 #ok_juno_tds
    149 ok_juno_tds_apr23_2021
    150 


::    

    O[blyth@localhost opticks]$ ./ana/ggeo.sh 1:9/ --names
    /home/blyth/junotop/ExternalLibs/Opticks/0.0.0-rc1/bashrc: line 4: /home/blyth/junotop/ExternalLibs/Opticks/0.0.0-rc1/bin/opticks-setup.sh: No such file or directory
    mo .bashrc OPTICKS_MODE:dev O : ordinary opticks dev ontop of juno externals CMTEXTRATAGS:
    python3 ./ana/ggeo.py 1:9/ --names
    nrpo( 176632     1     0     0 )                                 PMT_3inch_log_phys                                      PMT_3inch_log  114 PMT_3inch_pmt_solid 
    nrpo( 176633     1     0     1 )                                PMT_3inch_body_phys                                 PMT_3inch_body_log  112 PMT_3inch_body_solid_ell_ell_helper 
    nrpo( 176634     1     0     2 )                              PMT_3inch_inner1_phys                               PMT_3inch_inner1_log  110 PMT_3inch_inner1_solid_ell_helper 
    nrpo( 176635     1     0     3 )                              PMT_3inch_inner2_phys                               PMT_3inch_inner2_log  111 PMT_3inch_inner2_solid_ell_helper 
    nrpo( 176636     1     0     4 )                                PMT_3inch_cntr_phys                                 PMT_3inch_cntr_log  113 PMT_3inch_cntr_solid 
    nrpo(  70960     2     0     0 )                                  pLPMT_NNVT_MCPPMT                             NNVTMCPPMTlMaskVirtual  103 NNVTMCPPMTsMask_virtual 
    nrpo(  70961     2     0     1 )                                    NNVTMCPPMTpMask                                    NNVTMCPPMTlMask   98 NNVTMCPPMTsMask 
    nrpo(  70962     2     0     2 )                     NNVTMCPPMT_PMT_20inch_log_phys                          NNVTMCPPMT_PMT_20inch_log  102 NNVTMCPPMT_PMT_20inch_pmt_solid 
    nrpo(  70963     2     0     3 )                    NNVTMCPPMT_PMT_20inch_body_phys                     NNVTMCPPMT_PMT_20inch_body_log  101 NNVTMCPPMT_PMT_20inch_body_solid 
    nrpo(  70964     2     0     4 )                  NNVTMCPPMT_PMT_20inch_inner1_phys                   NNVTMCPPMT_PMT_20inch_inner1_log   99 NNVTMCPPMT_PMT_20inch_inner1_solid_1_Ellipsoid 
    nrpo(  70965     2     0     5 )                  NNVTMCPPMT_PMT_20inch_inner2_phys                   NNVTMCPPMT_PMT_20inch_inner2_log  100 NNVTMCPPMT_PMT_20inch_inner2_solid 
    nrpo(  70966     3     0     0 )                             pLPMT_Hamamatsu_R12860                        HamamatsuR12860lMaskVirtual  109 HamamatsuR12860sMask_virtual 
    nrpo(  70967     3     0     1 )                               HamamatsuR12860pMask                               HamamatsuR12860lMask  104 HamamatsuR12860sMask 
    nrpo(  70968     3     0     2 )                HamamatsuR12860_PMT_20inch_log_phys                     HamamatsuR12860_PMT_20inch_log  108 HamamatsuR12860_PMT_20inch_pmt_solid_1_9 
    nrpo(  70969     3     0     3 )               HamamatsuR12860_PMT_20inch_body_phys                HamamatsuR12860_PMT_20inch_body_log  107 HamamatsuR12860_PMT_20inch_body_solid_1_9 
    nrpo(  70970     3     0     4 )             HamamatsuR12860_PMT_20inch_inner1_phys              HamamatsuR12860_PMT_20inch_inner1_log  105 HamamatsuR12860_PMT_20inch_inner1_solid_I 
    nrpo(  70971     3     0     5 )             HamamatsuR12860_PMT_20inch_inner2_phys              HamamatsuR12860_PMT_20inch_inner2_log  106 HamamatsuR12860_PMT_20inch_inner2_solid_1_9 
    nrpo( 304636     4     0     0 )              mask_PMT_20inch_vetolMaskVirtual_phys                   mask_PMT_20inch_vetolMaskVirtual  126 mask_PMT_20inch_vetosMask_virtual 
    nrpo( 304637     4     0     1 )                          mask_PMT_20inch_vetopMask                          mask_PMT_20inch_vetolMask  121 mask_PMT_20inch_vetosMask 
    nrpo( 304638     4     0     2 )                           PMT_20inch_veto_log_phys                                PMT_20inch_veto_log  125 PMT_20inch_veto_pmt_solid_1_2 
    nrpo( 304639     4     0     3 )                          PMT_20inch_veto_body_phys                           PMT_20inch_veto_body_log  124 PMT_20inch_veto_body_solid_1_2 
    nrpo( 304640     4     0     4 )                        PMT_20inch_veto_inner1_phys                         PMT_20inch_veto_inner1_log  122 PMT_20inch_veto_inner1_solid 
    nrpo( 304641     4     0     5 )                        PMT_20inch_veto_inner2_phys                         PMT_20inch_veto_inner2_log  123 PMT_20inch_veto_inner2_solid 
    nrpo(  68488     5     0     0 )                                        lSteel_phys                                             lSteel   93 sStrutBallhead 
    nrpo(  69078     6     0     0 )                                    lFasteners_phys                                         lFasteners   94 uni1 
    nrpo(  69668     7     0     0 )                                        lUpper_phys                                             lUpper   95 base_steel 
    nrpo(  70258     8     0     0 )                                     lAddition_phys                                          lAddition   96 uni_acrylic3 
    O[blyth@localhost opticks]$ 
    O[blyth@localhost opticks]$ 
       




Add tds-ectrl::

    epsilon:j blyth$ git diff 
    diff --git a/j.bash b/j.bash
    index af214a6..44bfc6d 100644
    --- a/j.bash
    +++ b/j.bash
    @@ -863,7 +863,8 @@ tds-ectrl(){
        local extra
        #extra="--dbggssave --dumphit --layout 100 --savesensor"
        #extra="--dbggdmlpath $dbggdmlpath" 
    -   extra="--rngmax 100"
    +   #extra="--rngmax 100"
    +   extra="--rngmax 100 --skipsolidname NNVTMCPPMTsMask_virtual,HamamatsuR12860sMask_virtual,mask_PMT_20inch_vetosMask_virtual"
     
        unset OPTICKS_EMBEDDED_COMMANDLINE_EXTRA
        if [ -n "$extra" ]; then 


    P[blyth@localhost cmt]$ OpticksDbg=INFO tds


Hmm still same idBuf inconsistency assert when doing the skips precache ?::

    P[blyth@localhost ~]$ OpticksDbg=INFO tds

    2021-04-24 00:28:47.722 INFO  [69012] [OGeo::makeOGeometry@683] ugeocode [A]
    2021-04-24 00:28:47.722 INFO  [69012] [OGeo::makeAnalyticGeometry@767]  skip GParts::close 
    2021-04-24 00:28:47.722 INFO  [69012] [OGeo::makeAnalyticGeometry@770] mm 2 verbosity: 0   pts:  GParts  primflag         flagnodetree numParts   29 numPrim    5
    2021-04-24 00:28:47.722 INFO  [69012] [OGeo::makeAnalyticGeometry@797]  mmidx 2 numInstances 12612 numPrim 5 idBuf 12612,6,4
    2021-04-24 00:28:47.722 FATAL [69012] [OGeo::makeAnalyticGeometry@808]  UNEXPECTED  idBuf 12612,6,4 numInstance 12612 numPrim 5 mm.index 2
    python: /home/blyth/opticks/optixrap/OGeo.cc:816: optix::Geometry OGeo::makeAnalyticGeometry(GMergedMesh*): Assertion `expect' failed.

Need to reproduce locally with geocache-create for faster dev cycle::

    scp P:/home/blyth/.opticks/geocache/DetSim0Svc_pWorld_g4live/g4ok_gltf/7ca7500a89b9fd1f5e354090a7758692/1/origin_CGDMLKludge.gdml apr23_origin_CGDMLKludge.gdml

    epsilon:opticks blyth$ t geocache-apr23
    geocache-apr23 () 
    { 
        local msg="=== $FUNCNAME :";
        local path=$(geocache-apr23-gdmlpath);
        local skipsolidname="NNVTMCPPMTsMask_virtual,HamamatsuR12860sMask_virtual,mask_PMT_20inch_vetosMask_virtual";
        local csgskiplv="103,109,126";
        GTree=INFO OpticksDbg=INFO GInstancer=INFO geocache-create- --gdmlpath $path -D --noviz --csgskiplv $csgskiplv $*
    }



Adapt GTree::makeInstanceIdentityBuffer to honour the skips
----------------------------------------------------------------

::

    In [1]: 12612*5
    Out[1]: 63060

    In [2]: 12612*6
    Out[2]: 75672

    In [1]: 12612*4
    Out[1]: 50448



    2021-04-23 19:31:22.377 INFO  [30107217] [*GTree::makeInstanceIdentityBuffer@146]  progeny0.size 5 count0 5 skips0 1 numVolumes 5 numPlacements 12612 numVolumes*numPlacements (num) 63060
    2021-04-23 19:31:22.409 FATAL [30107217] [*GTree::makeInstanceIdentityBuffer@214]  MISMATCH  buf_num 50448 num 63060
    Assertion failed: (buf_num == num), function makeInstanceIdentityBuffer, file /Users/blyth/opticks/ggeo/GTree.cc, line 220.


ana/GTree.py::

    import os, numpy as np

    if __name__ == '__main__':
        a = np.load(os.path.expandvars("$TMP/GTree/iid.npy"))
        b = np.load(os.path.expandvars("$TMP/GTree/iid2.npy"))



::

    In [4]: a.shape
    Out[4]: (12612, 5, 4)

    In [5]: b.shape
    Out[5]: (12612, 5, 4)



    In [6]: a[0]
    Out[6]:
    array([[   70961, 33554433,  6422544,        0],
           [   70962, 33554434,  6684695,        0],
           [   70963, 33554435,  6619160,        0],
           [   70964, 33554436,  6488089,        1],
           [   70965, 33554437,  6553626,        0]], dtype=uint32)

    In [7]: b[0]
    Out[7]:
    array([[       0,        0,        0,        0],
           [   70961, 33554433,  6422544,        0],
           [   70962, 33554434,  6684695,        0],
           [   70963, 33554435,  6619160,        0],
           [   70964, 33554436,  6488089,        1]], dtype=uint32)

    In [8]: a[1]
    Out[8]:
    array([[   70973, 33554689,  6422544,        0],
           [   70974, 33554690,  6684695,        0],
           [   70975, 33554691,  6619160,        0],
           [   70976, 33554692,  6488089,        3],
           [   70977, 33554693,  6553626,        0]], dtype=uint32)

    In [9]: b[1]
    Out[9]:
    array([[   70965, 33554437,  6553626,        0],
           [   70973, 33554689,  6422544,        0],
           [   70974, 33554690,  6684695,        0],
           [   70975, 33554691,  6619160,        0],
           [   70976, 33554692,  6488089,        3]], dtype=uint32)

    In [10]:




With skips done precache
--------------------------

::

    epsilon:opticks blyth$ ggeo.sh 1:9/ --names
    python3 /Users/blyth/opticks/ana/ggeo.py 1:9/ --names
    [2021-04-23 20:59:13,530] p75142 {/Users/blyth/opticks/ana/ggeo.py:438} INFO - oidx mismatch oidx2 1 oidx 0 
    [2021-04-23 20:59:13,530] p75142 {/Users/blyth/opticks/ana/ggeo.py:438} INFO - oidx mismatch oidx2 2 oidx 1 
    [2021-04-23 20:59:13,530] p75142 {/Users/blyth/opticks/ana/ggeo.py:438} INFO - oidx mismatch oidx2 3 oidx 2 
    [2021-04-23 20:59:13,530] p75142 {/Users/blyth/opticks/ana/ggeo.py:438} INFO - oidx mismatch oidx2 4 oidx 3 
    [2021-04-23 20:59:13,530] p75142 {/Users/blyth/opticks/ana/ggeo.py:438} INFO - oidx mismatch oidx2 5 oidx 4 
    [2021-04-23 20:59:13,531] p75142 {/Users/blyth/opticks/ana/ggeo.py:438} INFO - oidx mismatch oidx2 1 oidx 0 
    [2021-04-23 20:59:13,531] p75142 {/Users/blyth/opticks/ana/ggeo.py:438} INFO - oidx mismatch oidx2 2 oidx 1 
    [2021-04-23 20:59:13,531] p75142 {/Users/blyth/opticks/ana/ggeo.py:438} INFO - oidx mismatch oidx2 3 oidx 2 
    [2021-04-23 20:59:13,531] p75142 {/Users/blyth/opticks/ana/ggeo.py:438} INFO - oidx mismatch oidx2 4 oidx 3 
    [2021-04-23 20:59:13,532] p75142 {/Users/blyth/opticks/ana/ggeo.py:438} INFO - oidx mismatch oidx2 5 oidx 4 
    [2021-04-23 20:59:13,532] p75142 {/Users/blyth/opticks/ana/ggeo.py:438} INFO - oidx mismatch oidx2 1 oidx 0 
    [2021-04-23 20:59:13,532] p75142 {/Users/blyth/opticks/ana/ggeo.py:438} INFO - oidx mismatch oidx2 2 oidx 1 
    [2021-04-23 20:59:13,532] p75142 {/Users/blyth/opticks/ana/ggeo.py:438} INFO - oidx mismatch oidx2 3 oidx 2 
    [2021-04-23 20:59:13,532] p75142 {/Users/blyth/opticks/ana/ggeo.py:438} INFO - oidx mismatch oidx2 4 oidx 3 
    [2021-04-23 20:59:13,533] p75142 {/Users/blyth/opticks/ana/ggeo.py:438} INFO - oidx mismatch oidx2 5 oidx 4 
    nrpo( 176632     1     0     0 )                        PMT_3inch_log_phys0x43c2530                             PMT_3inch_log0x43c1620  114 PMT_3inch_pmt_solid0x43c0a40 
    nrpo( 176633     1     0     1 )                       PMT_3inch_body_phys0x43c1a60                        PMT_3inch_body_log0x43c1510  112 PMT_3inch_body_solid_ell_ell_helper0x43c0d00 
    nrpo( 176634     1     0     2 )                     PMT_3inch_inner1_phys0x43c1ae0                      PMT_3inch_inner1_log0x43c1730  110 PMT_3inch_inner1_solid_ell_helper0x43c0d90 
    nrpo( 176635     1     0     3 )                     PMT_3inch_inner2_phys0x43c1b90                      PMT_3inch_inner2_log0x43c1840  111 PMT_3inch_inner2_solid_ell_helper0x43c0e70 
    nrpo( 176636     1     0     4 )                       PMT_3inch_cntr_phys0x43c1c40                        PMT_3inch_cntr_log0x43c1950  113 PMT_3inch_cntr_solid0x43c0f00 
    nrpo(  70961     2     0     1 )                           NNVTMCPPMTpMask0x3c2cad0                           NNVTMCPPMTlMask0x3c2c950   98 NNVTMCPPMTsMask0x3c2c750 
    nrpo(  70962     2     0     2 )            NNVTMCPPMT_PMT_20inch_log_phys0x3c2cb50                 NNVTMCPPMT_PMT_20inch_log0x3c2a6b0  102 NNVTMCPPMT_PMT_20inch_pmt_solid0x3c21980 
    nrpo(  70963     2     0     3 )           NNVTMCPPMT_PMT_20inch_body_phys0x3c2aa10            NNVTMCPPMT_PMT_20inch_body_log0x3c2a5d0  101 NNVTMCPPMT_PMT_20inch_body_solid0x3c258a0 
    nrpo(  70964     2     0     4 )         NNVTMCPPMT_PMT_20inch_inner1_phys0x3c2aaa0          NNVTMCPPMT_PMT_20inch_inner1_log0x3c2a7d0   99 NNVTMCPPMT_PMT_20inch_inner1_solid_1_Ellipsoid0x3497520 
    nrpo(  70965     2     0     5 )         NNVTMCPPMT_PMT_20inch_inner2_phys0x3c2ab60          NNVTMCPPMT_PMT_20inch_inner2_log0x3c2a8f0  100 NNVTMCPPMT_PMT_20inch_inner2_solid0x3c2a360 
    nrpo(  70967     3     0     1 )                      HamamatsuR12860pMask0x3c394b0                      HamamatsuR12860lMask0x3c39330  104 HamamatsuR12860sMask0x3c39130 
    nrpo(  70968     3     0     2 )       HamamatsuR12860_PMT_20inch_log_phys0x3c39540            HamamatsuR12860_PMT_20inch_log0x3c36c90  108 HamamatsuR12860_PMT_20inch_pmt_solid_1_90x3c4a970 
    nrpo(  70969     3     0     3 )      HamamatsuR12860_PMT_20inch_body_phys0x33eeec0       HamamatsuR12860_PMT_20inch_body_log0x3c36ba0  107 HamamatsuR12860_PMT_20inch_body_solid_1_90x3c28080 
    nrpo(  70970     3     0     4 )    HamamatsuR12860_PMT_20inch_inner1_phys0x3c373b0     HamamatsuR12860_PMT_20inch_inner1_log0x33eec60  105 HamamatsuR12860_PMT_20inch_inner1_solid_I0x3c32bc0 
    nrpo(  70971     3     0     5 )    HamamatsuR12860_PMT_20inch_inner2_phys0x3c37470     HamamatsuR12860_PMT_20inch_inner2_log0x33eed90  106 HamamatsuR12860_PMT_20inch_inner2_solid_1_90x3c36980 
    nrpo( 304637     4     0     1 )                 mask_PMT_20inch_vetopMask0x3c2eb60                 mask_PMT_20inch_vetolMask0x3c2e9d0  121 mask_PMT_20inch_vetosMask0x3c2e7c0 
    nrpo( 304638     4     0     2 )                  PMT_20inch_veto_log_phys0x3c3e950                       PMT_20inch_veto_log0x3c3de20  125 PMT_20inch_veto_pmt_solid_1_20x3c305d0 
    nrpo( 304639     4     0     3 )                 PMT_20inch_veto_body_phys0x3c3e150                  PMT_20inch_veto_body_log0x3c3dd10  124 PMT_20inch_veto_body_solid_1_20x3c3cc50 
    nrpo( 304640     4     0     4 )               PMT_20inch_veto_inner1_phys0x3c3e1d0                PMT_20inch_veto_inner1_log0x3c3df30  122 PMT_20inch_veto_inner1_solid0x3c3d8c0 
    nrpo( 304641     4     0     5 )               PMT_20inch_veto_inner2_phys0x3c3e280                PMT_20inch_veto_inner2_log0x3c3e040  123 PMT_20inch_veto_inner2_solid0x3c3dae0 
    nrpo(  68488     5     0     0 )                               lSteel_phys0x34c07b0                                    lSteel0x34c0680   93 sStrutBallhead0x34be280 
    nrpo(  69078     6     0     0 )                           lFasteners_phys0x3461f60                                lFasteners0x3461e20   94 uni10x3461bd0 
    nrpo(  69668     7     0     0 )                               lUpper_phys0x35499e0                                    lUpper0x3549920   95 base_steel0x35a1810 
    nrpo(  70258     8     0     0 )                            lAddition_phys0x3593690                                 lAddition0x3593510   96 uni_acrylic30x35932f0 
    epsilon:opticks blyth$ 


::

    416     def get_node_index(self, ridx, pidx, oidx):
    417         """
    418         :param ridx: repeat index, 0 for remainder
    419         :param pidx: placement index of the instance, 0 for remainder
    420         :param oidx: offset index, within the instance or among the remainder
    421         :return nidx: all_volume node index 
    422 
    423         The node index obtained from the placement_identity is used
    424         to do a reverse conversion check using nrpo, looking up 
    425         the triplet identity from the node index. These indices
    426         are consistency checked with the inputs.
    427         """
    428         placement_iidentity = self.get_array(ridx, "placement_iidentity")  # eg shape  (672, 5, 4)
    429         iid = placement_iidentity[pidx, oidx]
    430         nidx = iid[0]
    431 
    432         nidx2,ridx2,pidx2,oidx2 = self.nrpo[nidx]
    433         assert nidx2 == nidx
    434         assert ridx2 == ridx
    435         assert pidx2 == pidx
    436 
    437         if oidx2 != oidx:
    438             log.info("oidx mismatch oidx2 %d oidx %d " % (oidx2, oidx))
    439         pass
    440         #assert oidx2 == oidx 
    441         return nidx




