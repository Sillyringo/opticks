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

#include <iostream>
#include <iomanip>

#include "SStr.hh"
#include "SPPM.hh"
#include "SLOG.hh"
#include "BFile.hh"
#include "NPY.hpp"
#include "ImageNPY.hpp"

const plog::Severity ImageNPY::LEVEL = SLOG::EnvLevel("ImageNPY", "DEBUG"); 


NPY<unsigned char>* ImageNPY::LoadPPMConcat(const std::vector<std::string>& paths, const std::vector<std::string>& configs, const bool yflip, const unsigned ncomp, bool old_concat )  // static
{
    assert( paths.size() == configs.size() ); 
    unsigned layers = paths.size(); 
    std::vector<const NPYBase*> imgs ; 
    bool layer_dimension = true ; 
    for(unsigned i=0 ; i < layers ; i++ ) 
    {
        std::string path = paths[i]; 
        std::string config = configs[i] ; 
        NPY<unsigned char>* img = LoadPPM(path.c_str(), yflip, ncomp, config.c_str(), layer_dimension); 
        imgs.push_back(img); 
    }

    //std::cout << "ImageNPY::LoadPPMConcat LEVEL " << LEVEL << std::endl ; 

    LOG_IF(LEVEL, old_concat) << "using old_concat imp " ; 
    NPY<unsigned char>* comb = old_concat ? NPY<unsigned char>::old_concat(imgs) : NPY<unsigned char>::concat(imgs) ; 

    LOG(LEVEL) << "concat img shape " << comb->getShapeString() ; 
    return comb  ; 
}

/**
ImageNPY::SavePPMConcat
-------------------------

Requires a 4d array representing layered images, eg with shape::

     (3,      512,    1024,  3)   
     (layers, height, width, payload-rgb )

The path argument is expected to end with ".ppm".
Each layer of the array is saved to separate files
with path argument ending modified to "_0.ppm" "_1.ppm" "_2.ppm"

**/

void ImageNPY::SavePPMConcat(const NPY<unsigned char>* imgs, const char* path, const bool yflip)
{
    assert( imgs->getDimensions() == 4 );  
    assert( SStr::EndsWith(path, ".ppm")); 

    unsigned ni = imgs->getShape(0); 
    assert( ni < 10 ); 
    for(unsigned item=0 ; item < ni ; item++)
    { 
        const char* end = SStr::Concat("_", item, ".ppm" );              // _0.ppm
        const char* outpath = SStr::ReplaceEnd( path, ".ppm", end ); 
        LOG(info) << " outpath " << outpath ; 
        ImageNPY::SavePPM( outpath,  imgs, yflip, item );   
    }
}



/**
ImageNPY::LoadPPM
-------------------

1. readHeader of the PPM for dimensions
2. create NPY array sized appropriately
3. read the PPM image into the array 

**/

NPY<unsigned char>* ImageNPY::LoadPPM(const char* path, const bool yflip, const unsigned ncomp, const char* config, bool concat_dimension)  // static
{
    unsigned width(0) ; 
    unsigned height(0) ; 
    unsigned mode(0) ; 
    unsigned bits(0) ; 

    int rc0 = SPPM::readHeader(path, width, height, mode, bits ); 

    assert( rc0 == 0 && mode == 6 && bits == 255 ); 

    LOG(LEVEL) 
        << " path " << path 
        << " width " << width 
        << " height " << height 
        << " mode " << mode 
        << " bits " << bits 
        << " config " << config
        ;

    NPY<unsigned char>* img = NPY<unsigned char>::make( height, width, ncomp ) ;  
    img->zero(); 
    std::vector<unsigned char>& imgvec = img->vector();    

    int rc = SPPM::read(path, imgvec, width, height, ncomp, yflip ); 
    assert( rc == 0 );  

    bool add_border = SStr::Contains(config, "add_border"); 
    bool add_midline = SStr::Contains(config, "add_midline"); 
    bool add_quadline = SStr::Contains(config, "add_quadline"); 

    if(add_border)  SPPM::AddBorder( imgvec, width, height, ncomp, yflip); 
    if(add_midline) SPPM::AddMidline(imgvec, width, height, ncomp, yflip); 
    if(add_quadline) SPPM::AddQuadline(imgvec, width, height, ncomp, yflip); 

    if(concat_dimension)
    {
        assert( img->getDimensions() == 3 ); 
        LOG(LEVEL) << " reshaping original img (height, width, ncomp)  " << img->getShapeString() ;
        unsigned layers = 1 ; 
        img->reshape(layers,height,width,ncomp) ; // NB height before width matching PPM row major ordering 
        LOG(LEVEL) << " after reshape img (items,height,width,ncomp) " << img->getShapeString()  ;
        unsigned ncomp2 = img->getShape(-1) ;
        assert( ncomp2 == ncomp );
    }
    return img ; 
}




void ImageNPY::SavePPM(const char* dir, const char* name,  const NPY<unsigned char>* a, const bool yflip, int item)
{
    bool createdir = true ; 
    std::string path = BFile::preparePath(dir, name, createdir); 
    SavePPMImp(path.c_str(), a, yflip, item); 
}
void ImageNPY::SavePPM(const char* path_,  const NPY<unsigned char>* a, const bool yflip, int item)
{
    bool createdir = true ; 
    std::string path = BFile::preparePath(path_, createdir); 
    SavePPMImp(path.c_str(), a, yflip, item); 
}

void ImageNPY::SavePPMImp(const char* path, const NPY<unsigned char>* a, const bool yflip, int item)
{
    unsigned nd = a->getDimensions(); 

    unsigned layers ; 
    unsigned height ; 
    unsigned width ; 
    unsigned ncomp ; 
    const unsigned char* data = NULL ; 
    LOG(LEVEL) << " write to " << path ; 

    if( item == -1 )
    {
        assert( nd == 3 );
        layers = 0 ;  
        height = a->getShape(0); 
        width = a->getShape(1); 
        ncomp = a->getShape(2); 
        data = a->getValuesConst() ; 
    }
    else
    {
        assert( nd == 4 );  
        layers = a->getShape(0); 
        height = a->getShape(1); 
        width = a->getShape(2); 
        ncomp = a->getShape(3); 

        unsigned item_size = a->getNumBytes(1); 
        data = a->getValuesConst() + item_size*item ;  
    }
    SPPM::write(path, data , width, height, ncomp, yflip );

    LOG(LEVEL) 
        << " path " << path 
        << " layers " << layers 
        << " width " << width 
        << " height " << height 
        << " ncomp " << ncomp 
        << " yflip " << yflip
        << " item " << item
        ;

    
}
 





