#pragma once


#if defined(__CUDACC__) || defined(__CUDABE__)
   #define QUTIL_METHOD __device__
#else
   #define QUTIL_METHOD 
#endif 


struct qutil
{
    QUTIL_METHOD static void rotateUz(float3& d, const float3& u ); 
}; 


/**
qutil::rotateUz
-----------------

This rotates the reference frame of a vector such that the original Z-axis will lie in the
direction of *u*. Many rotations would accomplish this; the one selected
uses *u* as its third column and is given by the below matrix.

The below CUDA implementation follows the CLHEP implementation used by Geant4::

     // geant4.10.00.p01/source/externals/clhep/src/ThreeVector.cc
     72 Hep3Vector & Hep3Vector::rotateUz(const Hep3Vector& NewUzVector) {
     73   // NewUzVector must be normalized !
     74 
     75   double u1 = NewUzVector.x();
     76   double u2 = NewUzVector.y();
     77   double u3 = NewUzVector.z();
     78   double up = u1*u1 + u2*u2;
     79 
     80   if (up>0) {
     81       up = std::sqrt(up);
     82       double px = dx,  py = dy,  pz = dz;
     83       dx = (u1*u3*px - u2*py)/up + u1*pz;
     84       dy = (u2*u3*px + u1*py)/up + u2*pz;
     85       dz =    -up*px +             u3*pz;
     86     }
     87   else if (u3 < 0.) { dx = -dx; dz = -dz; }      // phi=0  teta=pi
     88   else {};
     89   return *this;
     90 }

This implements rotation of (px,py,pz) vector into (dx,dy,dz) 
using the below rotation matrix, the columns of which must be 
orthogonal unit vectors.::

                |  u.x * u.z / up   -u.y / up    u.x  |        
        d  =    |  u.y * u.z / up   +u.x / up    u.y  |      p
                |   -up               0.         u.z  |      
    
Taking dot products between and within columns shows that to 
be the case for normalized u. See oxrap/rotateUz.h for the algebra. 

Special cases:

u = [0,0,1] (up=0.) 
   does nothing, effectively identity matrix

u = [0,0,-1] (up=0., u.z<0. ) 
   flip x, and z which is a rotation of pi/2 about y 

               |   -1    0     0   |
      d =      |    0    1     0   |   p
               |    0    0    -1   |
           
**/

inline QUTIL_METHOD void qutil::rotateUz(float3& d, const float3& u ) 
{
    float up = u.x*u.x + u.y*u.y ;
    if (up>0.f) 
    {   
        up = sqrt(up);
        float px = d.x ;
        float py = d.y ;
        float pz = d.z ;
        d.x = (u.x*u.z*px - u.y*py)/up + u.x*pz;
        d.y = (u.y*u.z*px + u.x*py)/up + u.y*pz;
        d.z =    -up*px +                u.z*pz;
    }   
    else if (u.z < 0.f ) 
    {   
        d.x = -d.x; 
        d.z = -d.z; 
    }      
}



