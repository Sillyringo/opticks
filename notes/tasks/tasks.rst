Tasks
=======

.. contents:: Table of Contents : https://bitbucket.org/simoncblyth/opticks/src/master/notes/tasks/tasks.rst
   :depth: 3


**A** : Needed ASAP for JUNO 
---------------------------------

**A1** : Expand GPU processing to detector collection efficiency hit culling
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Typically candidate photon hits are culled stocastically using 
a combined detection efficiency computed from the characteristics
of the sensor.

Migrating such detector specific culling to the GPU can potentially 
greatly reduce CPU memory required for hits by arranging that all 
hits copied from GPU to CPU are collected.

JUNO simulation currently culls candidate hits according to a PMT angle dependent collection 
efficiency parameterised from PMT angle measurements with a cubic spline interpolation
implemented with G4DataInterpolation::CubicSplineInterpolation.

Rather than directly porting G4DataInterpolation to CUDA it is expected that 
a GPU texture based approach would provide a more performant solution with 
less development effort.  Directly using G4DataInterpolation on the CPU 
to fill more "sample" points into a texture array used to construct the GPU texture
will allow the results of the interpolation to be available on the GPU using 
dedicated texture lookup hardware that does linear interpolation. The resolution of the
GPU texture can be increased as much as needed to closely reproduce the result 
of the cubic spline interpolation.

**2020 Aug/Sep/Oct**

* GSensorLib/OSensorLib angular GPU texture handling added, tested with 
  2d theta-phi textures for generality even through JUNO only needs theta dependent 
  efficiencies

* local/global frame positions for hits without doubling photon size  

* local/global implementation revealed problem with Opticks geometry model 
  that made it impossible to address the remainder volumes with Opticks native 
  triplet addressing (mm0 special treatment problem). Fixing this required widespread changes
  but has strong benefits of simplifying the geometry model and avoiding a longstanding 
  cause of confusion and bugs.

**What remains to be done ?**

* optional "way point" recording (needed to complete JUNO hits) 

  * currently doing in separate way buffer just doesnt fly 
  * need to integrate way points into the photon buffer in optional way 
    with the normal 4x4 photon optionally becoming 4x5


**A2** : Machinery for Random Aligned "Bi-Simulation" Validation 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

* **Adaption of existing machinery**

Validation machinery needs to be adapted to work in direct (G4Opticks style)
integrated running. It was excluded from the G4Opticks interface 
because it was too complex to comfortably include in an introductory 
example.  A way needs to be found to have the functionality without 
introducing too much complication.

Opticks includes instrumentation that records all photon parameters at every step 
of both the GPU and CPU simulations. This recording machinery together with the use of  
common input photons and streams of random numbers allows the CPU and GPU simulations 
to be performed in an aligned manner with every reflection, refraction, scatter and 
absorption expected to occur with the same parameters such as positions, times, 
polarizations and wavelengths. 
Aligned running means that direct comparisons of parameter arrays can be performed to  
provide an extremely sensitive way to find discrepancies unclouded by statistics.
Interpretation of the causes of problems is assisted by GPU based photon history 
indexing that facilitates comparisons within categories of photon histories.


**A3** : Production Running Optimization
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

* **New Area : lots of learning + experiments needed**

There are many open questions regarding how best to perform large Opticks
accelerated Monte Carlo production runs on GPU clusters, where for example 
each cluster node houses eight NVIDIA Tesla GV100 GPUs. Opticks measurements with
NVIDIA OptiX 6 indicate that performance scales linearly up to four GPUs only.
Although dedicating four GPUs to a single CPU node may give the fastest single
node performance it is more important to optimize for the overall throughput of
the cluster running hundreds of production jobs rather than the performance of
a single node. Indeed the extreme GPU performance will likely mean that commonly, 
even with a single GPU per CPU node, the GPU will mostly be idle waiting for CPU processing
to complete.
Use of distributed computing techniques may be able to avoid this GPU starvation
situation by allowing the many more CPU nodes available to effectively share the relatively
fewer GPUs.


**A4** : Opticks Server + Client 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Early versions of Opticks incorporated a message queue based asynchronous
server (using ZeroMQ and Boost Asio) which allowed the Opticks GPU simulation to be provided 
as a service to network clients. Reviving this Opticks Server functionality would drastically increase
the flexibility of Opticks usage by bringing it to any network connected device including
learning environments such as IPython and Jupyter Notebooks.

Experimentation is needed to see if these Opticks Server capabilities could allow
limited GPU resources to be effectively shared from a larger number of CPU only nodes.

**Nov 2020** 

* reviewed old NumpyServer, conclude simpler to start over 
* prototyped array transport and server-client in **np** repository https://github.com/simoncblyth/np/

  * conclude dependency on Boost-Asio alone is feasible and advantageous

* "OpticksClient.hh" aims to be header only and trivial to incorporate into Geant4 based simulation code


**A5** : migrate OptixRap geometry to OptiX 7.1+
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

* **Adoption of New API : significant learning/experimentation needed** 


1. OptiX 7 learning via developing standalone examples (see examples/UseOptiX7*)
   in response to techniques gleaned from how others are using optix7 see optix7-vi 

2. improve compartmentalization of the Opticks dependency on OptiX : 
   so can cleanly switch backwards and forwards between 6.5 and 7.1
   and other versions
    
3. explore ways of bringing the geometry to OptiX 7

**Jan 2021**

* LZ/LBL/NERSC/NVIDIA people keen to get involved with this task
* Created orientation documentation as until planned bi-weekly meetings start 
* https://simoncblyth.bitbucket.io/opticks/docs/orientation.html



**A6** : geometry model BVH "tuning" with OptiX 6.5, 7.1,...
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

* **A critical task for performance**


The NVIDIA BVH acceleration structure dictates intersection 
performance both in itself and to what extent the dedicated ray trace hardware RT Cores
are utilized.  The only way of influencing the "black box" BVH is via the choices of 
OptiX geometry modelling (heirarchy, instances, different OptiX geometry classes).

Performance measurements with a variety of GPUs and geometries while exploring 
different ways of setting up the OptiX geometry are needed to arrive at the 
optimal performance and gain some intuition of what works best.


**A7** : Event Splitting/Joining
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

For handling events with more optical photons than can be accommodated in the available VRAM,
what matters is the GPU VRAM that the photons require, not the number of gensteps.  
When collecting gensteps a calculation of the GPU VRAM needed can be made
based on the cumulative photon count. When the VRAM exceeds a configurable limit then the 
propagation needs to be done and hits collected.  
This could happen during the collection rather than waiting for the end of event.

Actually some users of Opticks will have the problem of having too few 
photons per event causing overheads of GPU launching to impact performance. 
To avoid this the gensteps of multiple events need to be combined up to a configurable 
maximum number of events that can be combined before GPU launches are done. Followed
by bookkeeping to place the hits into the hit collections of the appropriate events.  

Clearly the implementation of both these features are similar enough that they
should be done in a unified way.



**B** : Useful for JUNO but not urgently required 
-----------------------------------------------------

**B1** : prototype Geant4 optical genstep API and try to get Geant4 to incorporate it
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Another aspect of improving integration that while possibly quite 
simple requires changes on the Geant4 side. 

Currently it is necessary to  
customize G4Scintillation and G4Cerenkov (and in future G4OpWLS) 
in order to collect "genstep" parameters just before the photon 
generation loop. Addition of a "genstep" API and a way to inhibit the 
photon generation loop to all Geant4 optical photon generating 
processes would be very helpful for integrating external optical photon simulations
like Opticks.  


**B2** : Visualization Refactoring to explore new graphics developments
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Visualizations of detector geometries and event displays provide
the fastest and most effective way to communicate the principals of
detector operation to students and the general public.

Opticks provides NVIDIA OptiX ray traced geometry rendering using precisely the same 
geometry as the optical photon simulation as well as rasterized rendering using OpenGL 4.1 shaders.
The ray traced and rasterized renders are composited by calculation of depth for every ray traced pixel. 
The rasterized render of geometry and optical photon propagations enables photons to be selected 
based upon their histories. OpenGL geometry shaders are used to interpolate between recorded step 
points of the photons allowing the simulation time to be an input uniform to the render.  
This enables time scrubbing the visualizations of optical photon propagations of millions of photons, 
with interactive selection of photon categories based on their histories. 

The graphics community is currently in transition between the OpenGL API
and the low overhead Vulkan API. The platform specific APIs : Direct3D and Metal 
are also widely used. For long term sustainability of Opticks visualization it is necessary to 
refactor to make it possible to work with multiple renderer backends such as OpenGL, Vulkan, DirectX and Metal.

Several open source projects (eg bgfx, LLGL, DiligentEngine : see `env-;llgl-;bgfx-;dileng-`) 
provide abstraction layers above the graphics libraries which may provide a 
quickstart route to Opticks visualization being able to work on a much 
wider variety of devices.  Although abstraction layers might restrict techniques available
they provide a good way to break into new areas.      

Gaining experience with Vulkan is particularly important as it provides a
cross-vendor, cross-platform standard for visualization plus compute shaders and has recently
introduced provisional ray tracing extensions which take advantage of vendor
specific ray tracing cores where available. This opens the possibility of Opticks
visualization and simulation being able to operate across GPUs from all vendors : NVIDIA, AMD and Intel.

This refactor will also open up many opportunities to benefit from recent
developments in the graphics community such as the use of the OpenVR API
providing virtual reality visualization of detector geometries and optical photon propagations.
Also this will provide a route for the Opticks visualization to eventually run on
many more devices.



**B3** : Multi-GPU scaling in OptiX 7+
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

* **New Area : Lots of Learning/Exerimentation Required**

*This task has some cross-talk with production running optimization. 
As I suspect the CPU/GPU work balance will usually mean that GPUs are 
starved of work and waiting around for the CPU : so the priority assigned 
to development of this (which could be very difficult) needs to bear in mind this 
bigger picture.*


OptiX 6 features transparent linear performance scaling out to 4 GPUs.
The transition to OptiX 7 drops this feature, with the 
task of handling multiple GPUs being left to the application.

1. investigations to see how others are doing this
   (praying for some demo code from NVIDIA or some open source OptiX renderers that have done this)

   * expect will entail pinned memory on the host 

2. create "standalone" example code to explore techniques, 
3. performance measurements 
4. investigate how this can be integrated with Opticks 


**B4** : Try to benefit from multi-threaded support in OptiX 7+
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

* **New Area : Lots of Learning/Exerimentation Required**









**C** : Tasks not Needed for JUNO, but useful for wider applicability 
-----------------------------------------------------------------------

**C1** : implement Geant4 extended example
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Fermilab Geant4 group are working on this 


**C2** : add support for G4OpWLS wavelength shifting
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

* **Straightforward but a bit involved as widespread modifications needed**


For details see:

* https://bitbucket.org/simoncblyth/opticks/src/master/notes/tasks/G4WLS_translation.rst
* :doc:`G4WLS_translation`


Summary of the steps:
 
1. adding WLSABSLENGTH to the standard material props and getting it thru into the GPU boundary texture 
2. using the wlsabsorption_length to give wlsabsorption_distance in propagate.h:propagate_to_boundary
3. during geometry translation assert that WLSMEANNUMBERPHOTONS is not present or has value of 1
4. ggeo/GWLSLib analogous to ggeo/GScintillatorLib that collects WLS materials and prepares the icdf buffer (equiv to BuildPhysicsTable)
5. optixrap/OWLSLib analogous to optixrap/OScintillatorLib that converts the buffer from GWLSLib into a GPU texture
6. optixrap/cu/wavelength_lookup.h  wls_lookup similar to reemission_lookup 
 

**C3** : add support for G4OpMieHG scattering
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

* source/processes/optical/src/G4OpMieHG.cc

* :doc:`G4OpMieHG_translation`

A brief look suggests this is straightforward, will entail adding more properties to the 
standard Opticks subset.

As MieHG scattering is not important for many experiements, all 
changes to support it will need to be made in an optional 
manner for example with compilation options to include it.

What needs to be done:


1. modify Opticks material property handling and boundary texture to include the 
   additional kMIEHG properties.  
   Changes needed in ggeo/GMaterialLib ggeo/GBndLib extg4/X4MaterialLib.
   The boundary texture currently has two float4 with five of the eight properties occupies, 
   the four properties needed for MIE scattering would require changing the boundary texture
   shape to accomodate these.  

2. implement the CUDA optixrap/cu/mie.h based on source/processes/optical/src/G4OpMieHG.cc
   in an analogous manner to how optixrap/cu/rayleigh.h is based on source/processes/optical/src/G4OpRayleigh.cc 

3. modify oxrap/cu/generate.cu to access the expanded boundary texture and add the process by 
   adding more random generation to yield a miescattering distance and compare with aborption and rayleight scattering 
   lengths to decide history.

4. validate the ported code by comparisons with Geant4 




**C4** : add support for more G4OpBoundaryProcess surface types
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Only the small portion of G4OpBoundaryProcess surface types needed for JUNO are ported.

* :doc:`G4OpBoundaryProcess_groundfrontpainted`


**C5** : find a way to handle G4Torus which doesnt kill performance
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Direct approach of solving quartics is horrible due 
to the very large range of coefficients. Even analytic 
solutions using double precision lead to poor precision roots.
Many approaches have been tried, but no robust solution. 
And the use of complex double precision math on the GPU is 
terrible for performance.

With JUNO all torus can be removed, so the problem is avoided 
but nevetherless other detectors will need to support torus.

Sphere tracing and SDF geometry modelling is an iterative way 
to find intersections that could provide performant ray torus
intersections without using doubles.


**C6** : Iterative intersection via Sphere tracing and SDF geometry modelling 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This is a very different way of finding intersects in an iterative manner 
that removes the need to solve polynomials instead you just 
need to provide a bound on the distance to the surface from any position, 
ie provide an SDF (signed distance function).
The boolean composability of SDFs via min max (Rvachev functions) 
allows bounds on the distance to highly complex shapes to be obtained 
simply by evaluating the composed SDF.  Instead of having 
a tree of primitives that your CSG algorithm has to traverse 
that tree gets encoded down into a single SDF function.
This technique could be drastically faster for complex shapes despite 
it being iterative. 

Sphere tracing is an old graphics technique that iteratively and optimally 
steps towards intersects using signed distance functions, making it  
perfect for GPUs because of the very little state and just simple flops 
to find intersects.

About "sphere tracing" :
   http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.48.3825&rep=rep1&type=pdf

"Sphere Tracing: A Geometric Method for the Antialiased Ray Tracing of Implicit Surfaces" 
John C. Hart
   "Given a function returning the distance to an object, sphere tracing marches
    along the ray toward its first intersection in steps guaranteed not to
    penetrate the implicit surface."

Thinking about the sphere tracing approach it seems to me that 
it could perhaps be made to work along curved tracks too.


But SDFs have challenges with some shapes like ellipsoids, 

* https://www.iquilezles.org/www/articles/ellipsoids/ellipsoids.htm

However even with such challenges I expect that complex CSG combination solids that can be 
modelled with SDFs that will provide drastic performance leaps by using the NVIDIA BVH 
to pick the bbox and sphere tracing in the intersect.



**C7** : expand geometry support to more complex geometries
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Although the Opticks translation of detector geometries is general 
in its approach it has been developed and tested in the context of  
neutrino and dark matter search experiments with detector geometries 
that are much simpler than those of LHC experiments. Generalizing 
to work with more complex geometries is expected to be a significant effort.

The way to start with this is straightforward : point the Opticks 
translation at progressively more complex GDML getting the translation and 
translated geometry to work. 
This work would also need to be involved with the migration to OptiX 7 
as that may have a big impact on performance.  
Experimentation with the geometry modelling is needed to optimize the 
BVH accelerated intersect performance. How much experimentation is  
needed will depend on the performance obtained with geometries 
of LHC detector complexity. 


**C8** : support for more complex (G4Boolean abuse) CSG solids
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Some detector geometries abuse G4Boolean taking some shape and 
subtracting hundreds of holes from it.  The Opticks CSG implementation 
is based on a complete binary tree serialization which makes it 
extremely inefficient for complex solid CSG trees of say more than 32 nodes.  
Opticks can balance trees to push out the complexity boundary a bit but 
this is a workaround rather than a solution.

Moving away from the use of complete binary trees for CSG serialization 
is one way to work better with complex CSG : however that would effectively 
require a full reimplementation of the CSG intersection algorithm.


**C9** : supporting custom solid primitives 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Solids which are difficult to model in a performant way with 
generic CSG can be implemented with a custom ray geometry intersection primitive. 
Implementing a primitive requires two CUDA functions:

1. axis aligned bounds 
2. distance to intersect(t) and surface normal at intersect 
   for a particular ray_origin, ray_direction and t_min   

Examples are in : optixrap/cu/csg_intersect_primitive.h 

Currently the set of Opticks primitives cannot be extended at runtime but 
in principal this could be done using NVRTC (NVIDIA run time compilation)
done at geometry translation time.

Another aspect of this is how similar functionality might be 
added to Geant4 in order to have both CPU and GPU implemetations
for validation. 


**C10** : supporting hairy geometry, eg lots of wires  
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

* https://news.developer.nvidia.com/optix-sdk-7-1/

NVIDIA OptiX 7.1 adds a new curve primitive for hair and fur
which might be used for the simulation of wires



**C11** : investigate partitioned solids   
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The current CSG intersect algorithm for OptiX 6 runs **within** the OptiX primitive
so it will not need to change much when going to OptiX 7.

This "complex CSG primitive" approach is convenient but performance
would be improved by partitioning solids at the intersection boundaries 
of the constituent primitives. Intersects with such partitioned solids is expected to 
be faster as it makes better use of the BVH acceleration structure. The problem 
is that this is not a general approach : only certain more simple solids could be optimized 
in this way and it is difficult to automate the chopping up of solids.
However the typical shape of PMTs is expected to work with this approach.

The technique is similar to the partlist approach which was used 
for analytic modelling PMTs prior to the implementation of general CSG.
So trying this approach could start by reviving the partlist and comparing 
performance with general CSG for applicable shapes.


**C12** : "higher level" CSG    
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The CSG algorithm operates on a tree of constituent shapes which are currently implemented
within the OptiX primitive. Perhaps it might be possible to expose the constituent shapes
and their bbox to OptiX and implement the CSG on top of that : ie implementing CSG above the 
level of the OptiX primitive in a kind of compound primitive that contains a bunch of 
other sub-primitives. 
This would in principal allow for the CSG intersects to benefit more from BVH than they currently do.

This is a lot more ambitious than the **C11** above, as it is unclear if the technicalities 
would even allow this to be possible.  This is something to attempt after the OptiX 7 transition
has been completed and web trawls have been made searching for prior work on CSG with OptiX.


