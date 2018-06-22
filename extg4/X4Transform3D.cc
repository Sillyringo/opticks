#include "G4VPhysicalVolume.hh"
#include "X4Transform3D.hh"
#include "NGLM.hpp"
#include "GLMFormat.hpp"
#include "SDigest.hh"
#include "PLOG.hh"

std::string X4Transform3D::Digest(const G4Transform3D&  t)
{
    glm::mat4* mat = Convert(t);
    return SDigest::digest( (void*)mat, sizeof(glm::mat4) );
}

glm::mat4* X4Transform3D::GetLocalTransform(const G4VPhysicalVolume* const pv, bool frame)
{
    glm::mat4* transform = NULL ; 
    if(frame)
    {
        const G4RotationMatrix* rotp = pv->GetFrameRotation() ;
        G4ThreeVector    tla = pv->GetFrameTranslation() ;
        G4Transform3D    tra(rotp ? *rotp : G4RotationMatrix(),tla);
        transform = Convert( tra );
    }
    else
    {
        G4RotationMatrix rot = pv->GetObjectRotationValue() ;  // obj relative to mother
        G4ThreeVector    tla = pv->GetObjectTranslation() ;
        G4Transform3D    tra(rot,tla);
        transform = Convert( tra ) ;
    }
    return transform ; 
}

glm::mat4* X4Transform3D::Convert( const G4Transform3D& t ) // static
{
    // M44T
    std::array<float, 16> a ; 
    a[ 0] = t.xx() ; a[ 1] = t.yx() ; a[ 2] = t.zx() ; a[ 3] = 0.f    ; 
    a[ 4] = t.xy() ; a[ 5] = t.yy() ; a[ 6] = t.zy() ; a[ 7] = 0.f    ; 
    a[ 8] = t.xz() ; a[ 9] = t.yz() ; a[10] = t.zz() ; a[11] = 0.f    ; 
    a[12] = t.dx() ; a[13] = t.dy() ; a[14] = t.dz() ; a[15] = 1.f    ;

    unsigned n = checkArray(a);
    if(n > 0) LOG(fatal) << "nan/inf array values";
    assert( n == 0); 

    glm::mat4* mat = new glm::mat4(glm::make_mat4(a.data()));
    return mat ; 
}


G4Transform3D* X4Transform3D::Convert( const glm::mat4& trs ) // static
{
    float xx = trs[0][0] ; 
    float xy = trs[0][1] ; 
    float xz = trs[0][2] ; 
    float xw = trs[0][3] ;

    float yx = trs[1][0] ; 
    float yy = trs[1][1] ; 
    float yz = trs[1][2] ; 
    float yw = trs[1][3] ; 

    float zx = trs[2][0] ; 
    float zy = trs[2][1] ; 
    float zz = trs[2][2] ; 
    float zw = trs[2][3] ; 

    float wx = trs[3][0] ; 
    float wy = trs[3][1] ; 
    float wz = trs[3][2] ; 
    float ww = trs[3][3] ; 

    float dx = wx ; 
    float dy = wy ; 
    float dz = wz ; 

    float eps = 1e-5 ; 
    assert( std::abs( xw - 0) < eps ); 
    assert( std::abs( yw - 0 ) < eps ); 
    assert( std::abs( zw - 0 ) < eps ); 
    assert( std::abs( ww - 1 ) < eps ); 

    G4ThreeVector colX(xx, xy, xz);
    G4ThreeVector colY(yx, yy, yz);
    G4ThreeVector colZ(zx, zy, zz);

    G4RotationMatrix rotate( colX, colY, colZ ); 
    G4ThreeVector    translate(dx, dy, dz);
    G4Transform3D    transform( rotate, translate );

    return new G4Transform3D(transform) ; 
}


unsigned X4Transform3D::checkArray(const std::array<float,16>& a) // static
{
    unsigned num_nan(0);
    unsigned num_inf(0);

    for(unsigned i=0 ; i < 16 ; i++) if(std::isnan(a[i])) num_nan++ ; 
    for(unsigned i=0 ; i < 16 ; i++) if(std::isinf(a[i])) num_inf++ ; 

    bool zero_nan = num_nan == 0 ; 
    bool zero_inf = num_inf == 0 ; 

    if(!zero_nan || !zero_inf)
    {
        LOG(fatal) 
            << " num_nan " << num_nan  
            << " num_inf " << num_inf 
            ; 
    }
    return num_nan + num_inf ; 
}









X4Transform3D::X4Transform3D(const G4Transform3D&  t_, Mapping_t mapping_) 
    :
    t(t_),
    mapping(mapping_),
    ar(),
    mat(NULL)
{
    init();
}

void X4Transform3D::init()
{
    copyToArray(ar, t, mapping);
    unsigned n = checkArray(ar);
    if(n > 0) dump("bad array values");
    assert( n == 0); 

    mat = new glm::mat4(glm::make_mat4(ar.data()));
}


void X4Transform3D::copyToArray(std::array<float,16>& a, const G4Transform3D& t, Mapping_t appr ) // static
{
    switch(appr)
    {
        case M43  : copyToArray_M43(a,t)  ; break ; 
        case M44  : copyToArray_M44(a,t)  ; break ; 
        case M44T : copyToArray_M44T(a,t) ; break ; 
    }
}

void X4Transform3D::copyToArray_M43(std::array<float,16>& a, const G4Transform3D& t ) // static
{
    //LOG(info) << "M43" ; 
    a[ 0] = t.xx() ; a[ 1] = t.xy() ; a[ 2] = t.xz() ; a[ 3] = t.dx() ; 
    a[ 4] = t.yx() ; a[ 5] = t.yy() ; a[ 6] = t.yz() ; a[ 7] = t.dy() ; 
    a[ 8] = t.zx() ; a[ 9] = t.zy() ; a[10] = t.zz() ; a[11] = t.dz() ; 
    a[12] = 0.f    ; a[13] =  0.f   ; a[14] = 0.f    ; a[15] = 1.f    ;
}
void X4Transform3D::copyToArray_M44(std::array<float,16>& a, const G4Transform3D& t ) // static
{
    //LOG(info) << "M44" ; 
    a[ 0] = t.xx() ; a[ 1] = t.xy() ; a[ 2] = t.xz() ; a[ 3] = 0.f    ; 
    a[ 4] = t.yx() ; a[ 5] = t.yy() ; a[ 6] = t.yz() ; a[ 7] = 0.f    ; 
    a[ 8] = t.zx() ; a[ 9] = t.zy() ; a[10] = t.zz() ; a[11] = 0.f    ; 
    a[12] = t.dx() ; a[13] = t.dy() ; a[14] = t.dz() ; a[15] = 1.f    ;
}
void X4Transform3D::copyToArray_M44T(std::array<float,16>& a, const G4Transform3D& t ) // static
{
    //LOG(info) << "M44T" ; 
    a[ 0] = t.xx() ; a[ 1] = t.yx() ; a[ 2] = t.zx() ; a[ 3] = 0.f    ; 
    a[ 4] = t.xy() ; a[ 5] = t.yy() ; a[ 6] = t.zy() ; a[ 7] = 0.f    ; 
    a[ 8] = t.xz() ; a[ 9] = t.yz() ; a[10] = t.zz() ; a[11] = 0.f    ; 
    a[12] = t.dx() ; a[13] = t.dy() ; a[14] = t.dz() ; a[15] = 1.f    ;
}


void X4Transform3D::dump(const char* msg) const 
{
    LOG(info) << msg ; 
    std::cout << gpresent( "mat", *mat ) << std::endl ; 
}



