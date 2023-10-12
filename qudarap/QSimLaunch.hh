#pragma once 
#include <cstdio>
#include <cassert>
#include <cstring>

enum { 
   UNKNOWN,
   RNG_SEQUENCE,
   BOUNDARY_LOOKUP_ALL,
   BOUNDARY_LOOKUP_WATER,
   BOUNDARY_LOOKUP_LS,
   WAVELENGTH_SCINTILLATION,
   WAVELENGTH_CERENKOV,
   SCINT_GENERATE,
   CERENKOV_GENERATE,
   CERENKOV_GENERATE_ENPROP_FLOAT,
   CERENKOV_GENERATE_ENPROP_DOUBLE,
   CERENKOV_GENERATE_EXPT,
   GENERATE_PHOTON_G,
   BOUNDARY_LOOKUP_LINE_LS_L,
   PROP_LOOKUP_Y,
   FILL_STATE_0,
   FILL_STATE_1,
   RAYLEIGH_SCATTER_ALIGN,
   PROPAGATE_TO_BOUNDARY,
   PROPAGATE_AT_BOUNDARY,

   HEMISPHERE_S_POLARIZED,
   HEMISPHERE_P_POLARIZED,
   HEMISPHERE_X_POLARIZED,

   PROPAGATE_AT_BOUNDARY_NORMAL_INCIDENCE,

   PROPAGATE_AT_BOUNDARY_S_POLARIZED,
   PROPAGATE_AT_BOUNDARY_P_POLARIZED,
   PROPAGATE_AT_BOUNDARY_X_POLARIZED,

   PROPAGATE_AT_MULTIFILM_S_POLARIZED,
   PROPAGATE_AT_MULTIFILM_P_POLARIZED,
   PROPAGATE_AT_MULTIFILM_X_POLARIZED,


   QGEN_RANDOM_DIRECTION_MARSAGLIA, 
   QGEN_LAMBERTIAN_DIRECTION,
   QGEN_SMEAR_NORMAL_SIGMA_ALPHA,
   QGEN_SMEAR_NORMAL_POLISH,

   REFLECT_DIFFUSE, 
   REFLECT_SPECULAR, 
   PROPAGATE_AT_SURFACE, 

   FAKE_PROPAGATE,
   GENTORCH,
   MULTIFILM_LOOKUP,
   RANDGAUSSQ_SHOOT 
   
};
 
struct QSimLaunch
{
    static unsigned    Type(const char* name) ;  
    static bool        IsMutate(unsigned type) ;  
    static bool        IsSurface(unsigned type) ;  
    static unsigned    MutateSource(unsigned type); 
    static const char* Name(unsigned type ); 

    static constexpr const char* RNG_SEQUENCE_ = "rng_sequence" ; 
    static constexpr const char* BOUNDARY_LOOKUP_ALL_ = "boundary_lookup_all" ; 
    static constexpr const char* BOUNDARY_LOOKUP_WATER_ = "boundary_lookup_water" ; 
    static constexpr const char* BOUNDARY_LOOKUP_LS_ = "boundary_lookup_ls" ; 

    static constexpr const char* WAVELENGTH_SCINTILLATION_ = "wavelength_scintillation" ; 
    static constexpr const char* WAVELENGTH_CERENKOV_ = "wavelength_cerenkov" ; 

    static constexpr const char* CERENKOV_GENERATE_ = "cerenkov_generate" ; 
    static constexpr const char* CERENKOV_GENERATE_ENPROP_FLOAT_ = "cerenkov_generate_enprop_float" ; 
    static constexpr const char* CERENKOV_GENERATE_ENPROP_DOUBLE_ = "cerenkov_generate_enprop_double" ; 
    static constexpr const char* CERENKOV_GENERATE_EXPT_ = "cerenkov_generate_expt" ; 

    static constexpr const char* SCINT_GENERATE_ = "scint_generate" ; 


    static constexpr const char* FILL_STATE_0_ = "fill_state_0" ;
    static constexpr const char* FILL_STATE_1_ = "fill_state_1" ;
    static constexpr const char* RAYLEIGH_SCATTER_ALIGN_ = "rayleigh_scatter_align" ;
    static constexpr const char* PROPAGATE_TO_BOUNDARY_ = "propagate_to_boundary" ; 
    static constexpr const char* PROPAGATE_AT_BOUNDARY_ = "propagate_at_boundary" ; 

    static constexpr const char* HEMISPHERE_S_POLARIZED_ = "hemisphere_s_polarized" ; 
    static constexpr const char* HEMISPHERE_P_POLARIZED_ = "hemisphere_p_polarized" ; 
    static constexpr const char* HEMISPHERE_X_POLARIZED_ = "hemisphere_x_polarized" ; 
 
    static constexpr const char* PROPAGATE_AT_BOUNDARY_NORMAL_INCIDENCE_ = "propagate_at_boundary_normal_incidence" ; 

    static constexpr const char* PROPAGATE_AT_BOUNDARY_S_POLARIZED_ = "propagate_at_boundary_s_polarized" ; 
    static constexpr const char* PROPAGATE_AT_BOUNDARY_P_POLARIZED_ = "propagate_at_boundary_p_polarized" ; 
    static constexpr const char* PROPAGATE_AT_BOUNDARY_X_POLARIZED_ = "propagate_at_boundary_x_polarized" ; 
    
    static constexpr const char* PROPAGATE_AT_MULTIFILM_S_POLARIZED_ = "propagate_at_multifilm_s_polarized";
    static constexpr const char* PROPAGATE_AT_MULTIFILM_P_POLARIZED_ = "propagate_at_multifilm_p_polarized";
    static constexpr const char* PROPAGATE_AT_MULTIFILM_X_POLARIZED_ = "propagate_at_multifilm_x_polarized";

    
    static constexpr const char* QGEN_RANDOM_DIRECTION_MARSAGLIA_ = "random_direction_marsaglia" ;
    static constexpr const char* QGEN_LAMBERTIAN_DIRECTION_ = "lambertian_direction" ;
    static constexpr const char* QGEN_SMEAR_NORMAL_SIGMA_ALPHA_ = "smear_normal_sigma_alpha" ; 
    static constexpr const char* QGEN_SMEAR_NORMAL_POLISH_ = "smear_normal_polish" ; 


    static constexpr const char* REFLECT_DIFFUSE_ = "reflect_diffuse" ;
    static constexpr const char* REFLECT_SPECULAR_ = "reflect_specular" ;
    static constexpr const char* PROPAGATE_AT_SURFACE_ = "propagate_at_surface" ;
    static constexpr const char* FAKE_PROPAGATE_ = "fake_propagate" ;
    static constexpr const char* GENTORCH_ = "gentorch" ;
    static constexpr const char* MULTIFILM_LOOKUP_ = "multifilm_lookup";
    static constexpr const char* RANDGAUSSQ_SHOOT_ = "randgaussq_shoot" ; 

};


inline unsigned QSimLaunch::Type( const char* name )
{
   unsigned test = UNKNOWN ;  

   if(strcmp(name,CERENKOV_GENERATE_) == 0 )               test = CERENKOV_GENERATE ;
   if(strcmp(name,CERENKOV_GENERATE_ENPROP_FLOAT_) == 0 )  test = CERENKOV_GENERATE_ENPROP_FLOAT ;
   if(strcmp(name,CERENKOV_GENERATE_ENPROP_DOUBLE_) == 0 ) test = CERENKOV_GENERATE_ENPROP_DOUBLE ;
   if(strcmp(name,CERENKOV_GENERATE_EXPT_) == 0 )          test = CERENKOV_GENERATE_EXPT ;

   if(strcmp(name,SCINT_GENERATE_) == 0 ) test = SCINT_GENERATE ;

   if(strcmp(name,WAVELENGTH_SCINTILLATION_) == 0 ) test = WAVELENGTH_SCINTILLATION ;
   if(strcmp(name,WAVELENGTH_CERENKOV_) == 0 )      test = WAVELENGTH_CERENKOV ;


   if(strcmp(name,"G") == 0 ) test = GENERATE_PHOTON_G ;
   if(strcmp(name,"L") == 0 ) test = BOUNDARY_LOOKUP_LINE_LS_L ;
   if(strcmp(name,"Y") == 0 ) test = PROP_LOOKUP_Y ;

   if(strcmp(name,RNG_SEQUENCE_) == 0 )          test = RNG_SEQUENCE ; 
   if(strcmp(name,BOUNDARY_LOOKUP_ALL_) == 0 )   test = BOUNDARY_LOOKUP_ALL ; 
   if(strcmp(name,BOUNDARY_LOOKUP_WATER_) == 0 ) test = BOUNDARY_LOOKUP_WATER ;
   if(strcmp(name,BOUNDARY_LOOKUP_LS_) == 0 )    test = BOUNDARY_LOOKUP_LS ;


   if(strcmp(name,FILL_STATE_0_) == 0)           test = FILL_STATE_0 ;
   if(strcmp(name,FILL_STATE_1_) == 0)           test = FILL_STATE_1 ;
   if(strcmp(name,RAYLEIGH_SCATTER_ALIGN_) == 0) test = RAYLEIGH_SCATTER_ALIGN ;

   if(strcmp(name,PROPAGATE_TO_BOUNDARY_) == 0)  test = PROPAGATE_TO_BOUNDARY ;
   if(strcmp(name,PROPAGATE_AT_BOUNDARY_) == 0)  test = PROPAGATE_AT_BOUNDARY ;

   if(strcmp(name,HEMISPHERE_S_POLARIZED_) == 0) test = HEMISPHERE_S_POLARIZED ;
   if(strcmp(name,HEMISPHERE_P_POLARIZED_) == 0) test = HEMISPHERE_P_POLARIZED ;
   if(strcmp(name,HEMISPHERE_X_POLARIZED_) == 0) test = HEMISPHERE_X_POLARIZED ;

   if(strcmp(name,PROPAGATE_AT_BOUNDARY_NORMAL_INCIDENCE_) == 0)  test = PROPAGATE_AT_BOUNDARY_NORMAL_INCIDENCE ;

   if(strcmp(name,PROPAGATE_AT_BOUNDARY_S_POLARIZED_) == 0)  test = PROPAGATE_AT_BOUNDARY_S_POLARIZED ;
   if(strcmp(name,PROPAGATE_AT_BOUNDARY_P_POLARIZED_) == 0)  test = PROPAGATE_AT_BOUNDARY_P_POLARIZED ;
   if(strcmp(name,PROPAGATE_AT_BOUNDARY_X_POLARIZED_) == 0)  test = PROPAGATE_AT_BOUNDARY_X_POLARIZED ;
   
   if(strcmp(name,PROPAGATE_AT_MULTIFILM_S_POLARIZED_ )  == 0) test = PROPAGATE_AT_MULTIFILM_S_POLARIZED; 
   if(strcmp(name,PROPAGATE_AT_MULTIFILM_P_POLARIZED_ )  == 0) test = PROPAGATE_AT_MULTIFILM_P_POLARIZED;
   if(strcmp(name,PROPAGATE_AT_MULTIFILM_X_POLARIZED_ )  == 0) test = PROPAGATE_AT_MULTIFILM_X_POLARIZED;

   if(strcmp(name,QGEN_RANDOM_DIRECTION_MARSAGLIA_) == 0)  test = QGEN_RANDOM_DIRECTION_MARSAGLIA ;
   if(strcmp(name,QGEN_LAMBERTIAN_DIRECTION_) == 0)        test = QGEN_LAMBERTIAN_DIRECTION ;
   if(strcmp(name,QGEN_SMEAR_NORMAL_SIGMA_ALPHA_) == 0)    test = QGEN_SMEAR_NORMAL_SIGMA_ALPHA ;
   if(strcmp(name,QGEN_SMEAR_NORMAL_POLISH_) == 0)         test = QGEN_SMEAR_NORMAL_POLISH ;

   if(strcmp(name,REFLECT_DIFFUSE_) == 0)             test = REFLECT_DIFFUSE ;
   if(strcmp(name,REFLECT_SPECULAR_) == 0)            test = REFLECT_SPECULAR ;
   if(strcmp(name,PROPAGATE_AT_SURFACE_)  == 0)       test = PROPAGATE_AT_SURFACE ;
   if(strcmp(name,FAKE_PROPAGATE_)  == 0)             test = FAKE_PROPAGATE ;
   if(strcmp(name,GENTORCH_)  == 0)                   test = GENTORCH ;
   if(strcmp(name,MULTIFILM_LOOKUP_ )  == 0)          test = MULTIFILM_LOOKUP ;
   if(strcmp(name,RANDGAUSSQ_SHOOT_ )  == 0)          test = RANDGAUSSQ_SHOOT ;
  
   bool known =  test != UNKNOWN  ;
   if(!known) printf("QSimLaunch::Type name [%s] is unknown \n", name) ; 
   assert(known);  
   return test ; 
}


inline const char* QSimLaunch::Name( unsigned type )
{
    const char* s = nullptr ; 
    switch(type)
    {
        case RNG_SEQUENCE:           s = RNG_SEQUENCE_           ; break ; 
        case BOUNDARY_LOOKUP_ALL:    s = BOUNDARY_LOOKUP_ALL_    ; break ; 
        case BOUNDARY_LOOKUP_WATER:  s = BOUNDARY_LOOKUP_WATER_  ; break ; 
        case BOUNDARY_LOOKUP_LS:     s = BOUNDARY_LOOKUP_LS_     ; break ; 

        case WAVELENGTH_SCINTILLATION: s = WAVELENGTH_SCINTILLATION_   ; break ; 
        case WAVELENGTH_CERENKOV:      s = WAVELENGTH_CERENKOV_        ; break ; 

        case CERENKOV_GENERATE:                s = CERENKOV_GENERATE_                ; break ; 
        case CERENKOV_GENERATE_ENPROP_FLOAT:   s = CERENKOV_GENERATE_ENPROP_FLOAT_   ; break ; 
        case CERENKOV_GENERATE_ENPROP_DOUBLE:  s = CERENKOV_GENERATE_ENPROP_DOUBLE_  ; break ; 
        case CERENKOV_GENERATE_EXPT:           s = CERENKOV_GENERATE_EXPT_           ; break ; 

        case SCINT_GENERATE:                 s = SCINT_GENERATE_           ; break ; 


        case FILL_STATE_0:           s = FILL_STATE_0_           ; break ; 
        case FILL_STATE_1:           s = FILL_STATE_1_           ; break ; 
        case RAYLEIGH_SCATTER_ALIGN: s = RAYLEIGH_SCATTER_ALIGN_ ; break ;
        case PROPAGATE_TO_BOUNDARY:  s = PROPAGATE_TO_BOUNDARY_  ; break ;  
        case PROPAGATE_AT_BOUNDARY:  s = PROPAGATE_AT_BOUNDARY_  ; break ;  

        case PROPAGATE_AT_BOUNDARY_NORMAL_INCIDENCE:  s = PROPAGATE_AT_BOUNDARY_NORMAL_INCIDENCE_  ; break ;  

        case HEMISPHERE_S_POLARIZED: s = HEMISPHERE_S_POLARIZED_ ; break ; 
        case HEMISPHERE_P_POLARIZED: s = HEMISPHERE_P_POLARIZED_ ; break ; 
        case HEMISPHERE_X_POLARIZED: s = HEMISPHERE_X_POLARIZED_ ; break ; 


        case PROPAGATE_AT_BOUNDARY_S_POLARIZED:  s = PROPAGATE_AT_BOUNDARY_S_POLARIZED_  ; break ;  
        case PROPAGATE_AT_BOUNDARY_P_POLARIZED:  s = PROPAGATE_AT_BOUNDARY_P_POLARIZED_  ; break ;  
        case PROPAGATE_AT_BOUNDARY_X_POLARIZED:  s = PROPAGATE_AT_BOUNDARY_X_POLARIZED_  ; break ;  
        
        case PROPAGATE_AT_MULTIFILM_S_POLARIZED: s = PROPAGATE_AT_MULTIFILM_S_POLARIZED_ ; break ;
        case PROPAGATE_AT_MULTIFILM_P_POLARIZED: s = PROPAGATE_AT_MULTIFILM_P_POLARIZED_ ; break ;
        case PROPAGATE_AT_MULTIFILM_X_POLARIZED: s = PROPAGATE_AT_MULTIFILM_X_POLARIZED_ ; break ;


        case QGEN_RANDOM_DIRECTION_MARSAGLIA:   s = QGEN_RANDOM_DIRECTION_MARSAGLIA_   ; break ; 
        case QGEN_LAMBERTIAN_DIRECTION:         s = QGEN_LAMBERTIAN_DIRECTION_         ; break ; 
        case QGEN_SMEAR_NORMAL_SIGMA_ALPHA:     s = QGEN_SMEAR_NORMAL_SIGMA_ALPHA_     ; break ; 
        case QGEN_SMEAR_NORMAL_POLISH:          s = QGEN_SMEAR_NORMAL_POLISH_          ; break ; 


        case REFLECT_DIFFUSE:              s = REFLECT_DIFFUSE_              ; break ; 
        case REFLECT_SPECULAR:             s = REFLECT_SPECULAR_             ; break ; 
        case PROPAGATE_AT_SURFACE:         s = PROPAGATE_AT_SURFACE_         ; break ; 
        case FAKE_PROPAGATE:               s = FAKE_PROPAGATE_               ; break ; 
        case GENTORCH:                     s = GENTORCH_                     ; break ; 
        case MULTIFILM_LOOKUP:             s = MULTIFILM_LOOKUP_             ; break ;
        case RANDGAUSSQ_SHOOT:             s = RANDGAUSSQ_SHOOT_             ; break ;
    }
    return s; 
}


inline bool QSimLaunch::IsMutate( unsigned type )
{
    return 
        type == PROPAGATE_AT_BOUNDARY_S_POLARIZED || 
        type == PROPAGATE_AT_BOUNDARY_P_POLARIZED || 
        type == PROPAGATE_AT_BOUNDARY_X_POLARIZED || 
        type == PROPAGATE_AT_MULTIFILM_S_POLARIZED||
        type == PROPAGATE_AT_MULTIFILM_P_POLARIZED||
        type == PROPAGATE_AT_MULTIFILM_X_POLARIZED||
        type == FAKE_PROPAGATE 
        ; 
}

inline bool QSimLaunch::IsSurface( unsigned type )
{
    return type == REFLECT_DIFFUSE || type == REFLECT_SPECULAR ; 
}

inline unsigned QSimLaunch::MutateSource( unsigned type )
{
    unsigned src = UNKNOWN ; 
    switch(type)
    {
       case PROPAGATE_AT_MULTIFILM_S_POLARIZED:
       case PROPAGATE_AT_BOUNDARY_S_POLARIZED:  src = HEMISPHERE_S_POLARIZED ; break ; 
       case PROPAGATE_AT_MULTIFILM_P_POLARIZED:
       case PROPAGATE_AT_BOUNDARY_P_POLARIZED:  src = HEMISPHERE_P_POLARIZED ; break ;
       case PROPAGATE_AT_MULTIFILM_X_POLARIZED: 
       case PROPAGATE_AT_BOUNDARY_X_POLARIZED:  src = HEMISPHERE_X_POLARIZED ; break ; 
    } 
    return src ; 
}


