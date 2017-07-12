#pragma once

#include <string>

typedef enum
{  
   FRAME_MODEL, 
   FRAME_LOCAL, 
   FRAME_GLOBAL 

} NNodeFrameType ;


typedef enum
{  
   POINT_INSIDE  = 0x1 << 0, 
   POINT_SURFACE = 0x1 << 1, 
   POINT_OUTSIDE = 0x1 << 2  

} NNodePointType ;


typedef enum {
    JOIN_UNCLASSIFIED, 
    JOIN_COINCIDENT, 
    JOIN_OVERLAP, 
    JOIN_SPLIT
} NNodeJoinType ; 




#include "NPY_API_EXPORT.hh"

class NPY_API NNodeEnum
{
    public:
        static const char* FRAME_MODEL_ ;
        static const char* FRAME_LOCAL_;
        static const char* FRAME_GLOBAL_ ;
        static const char* FrameType(NNodeFrameType fr);

        static const char* POINT_INSIDE_;
        static const char* POINT_SURFACE_;
        static const char* POINT_OUTSIDE_;
        static const char* PointType(NNodePointType pt);

        static const char* JOIN_UNCLASSIFIED_ ;
        static const char* JOIN_COINCIDENT_ ;
        static const char* JOIN_OVERLAP_ ;
        static const char* JOIN_SPLIT_ ;
        static const char* JoinType(NNodeJoinType join);
        static NNodeJoinType JoinClassify( float za, float zb, float epsilon);

 
        static NNodePointType PointClassify( float sdf_ , float epsilon );
        static std::string PointMask(unsigned mask);


};
