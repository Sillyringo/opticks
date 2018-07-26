#pragma once

#include "X4_API_EXPORT.hh"
#include "X4Entity.hh"
#include "X4SolidBase.hh"

#include <vector>
#include <string>


class G4Hype ; 
class G4Ellipsoid ; 
class G4Torus ; 
class G4Cons ; 
class G4Trd ; 
class G4Tubs ; 
class G4Sphere ; 
class G4Orb ; 

class G4VSolid ; 
struct nnode ; 
struct SId ; 

/**
X4SolidBase
=============

NB : the below generated convert implementation standins 
that assert on being called are intended to be left asis.  
Just implement overriding convert methods up in X4Solid, as
you handle each solid.

**/

class X4_API X4SolidBase 
{
   public:

        /*
        static const char* IDENTIFIERS ; 
        static int IDENTIFIER_IDX ; 
        static void ResetIdentifier(); 
        static const char* Identifier(bool reset); 
        */

        static SId* NODE_ID ; 
        static SId* OTHER_ID ; 

        const char* getIdentifier() const ; 

        static G4Hype*      MakeHyperboloid(const char* name, float rmin , float rmax, float inst, float outst, float hz ) ; 
        static G4Ellipsoid* MakeEllipsoid(const char* name, float ax, float by, float cz, float zcut1=0.f, float zcut2=0.f );
        static G4Torus*  MakeTorus(const char* name, float R, float r ); 
        static G4Cons*   MakeCone(const char* name, float z, float rmax1, float rmax2, float rmin1=0.f, float rmin2=0.f, float startphi=0.f, float deltaphi=360.f );
        static G4Trd*    MakeTrapezoidCube(const char* name, float sz);
        static G4Trd*    MakeTrapezoid(const char* name, float z,  float x1, float y1, float x2, float y2 );
        static G4Tubs*   MakeTubs(const char* name, float rmin, float rmax, float hz, float startphi=0.f, float deltaphi=360.f );
        static G4Orb*    MakeOrb(const char* name, float radius);
        static G4Sphere* MakeSphere(const char* name, float rmax, float rmin=0.f );
        static G4Sphere* MakeZSphere(const char* name, float rmin, float rmax, float startPhi=0.f, float deltaPhi=360.f,float startTheta=0.f, float deltaTheta=180.f );

    private:
        // invoked internally as a result of setG4Param, generated g4code is tacked onto the nnode instance
        template<typename T>
        static std::string GenInstanciate(const char* cls, const char* identifier, const char* name, const std::vector<T>& param);
    public:
        X4SolidBase(const G4VSolid* solid, bool top=false); 
        nnode* root() const ;
        std::string desc() const  ; 
        std::string brief() const  ; 
    private:
        void init();
    protected:
        void setRoot(nnode* root);

        template<typename T>
        void setG4Param(const std::vector<T>& param, const char* identifier=NULL );  // sets ordered parameters used by g4code instanciation generation
        void setG4Code( const char* g4code ); // sets generated Geant4 code to instanciate the solid, usually invoked by setG4Param but needed for booleans,polycone
        void addG4Code( const char* g4code ); 

        const char* getG4Code(const char* identifier) const ; 
    protected:
        // generated by x4-convert-hh- Mon Jun 11 20:43:47 HKT 2018 

        void convertBooleanSolid();
        void convertMultiUnion();
        void convertBox();
        void convertCons();
        void convertEllipticalCone();
        void convertEllipsoid();
        void convertEllipticalTube();
        void convertExtrudedSolid();
        void convertHype();
        void convertOrb();
        void convertPara();
        void convertParaboloid();
        void convertPolycone();
        void convertGenericPolycone();
        void convertPolyhedra();
        void convertSphere();
        void convertTessellatedSolid();
        void convertTet();
        void convertTorus();
        void convertGenericTrap();
        void convertTrap();
        void convertTrd();
        void convertTubs();
        void convertCutTubs();
        void convertTwistedBox();
        void convertTwistedTrap();
        void convertTwistedTrd();
        void convertTwistedTubs();

    protected:
        const G4VSolid* m_solid ;  
        bool            m_top ; 
        const char*     m_name ; 
        X4Entity_t      m_entityType ; 
        const char*     m_entityName ; 
        const char*     m_identifier ; 
        nnode*          m_root ; 

        std::vector<std::string> m_g4code ; 

};

