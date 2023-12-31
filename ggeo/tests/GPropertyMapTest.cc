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

#include <cassert>
// op --gpropertymap

#include "BFile.hh"
#include "BOpticksResource.hh"

#include "GProperty.hh"
#include "GDomain.hh"
#include "GMaterialLib.hh"
#include "GPropertyLib.hh"
#include "GPropertyMap.hh"

#include "OPTICKS_LOG.hh"

const char* TMPDIR = "$TMP/ggeo/GPropertyMapTest" ; 


int main(int argc, char** argv)
{
    OPTICKS_LOG(argc, argv);

    BOpticksResource* rsc = BOpticksResource::Get(NULL) ;  // sets internal envvar OPTICKS_INSTALL_PREFIX
    assert(rsc); 

    const char* path = "$OPTICKS_INSTALL_PREFIX/opticksaux/refractiveindex/tmp/glass/schott/F2.npy";
    GProperty<double>* f2 = GProperty<double>::load(path);
    if(!f2)
    {
        LOG(error) << " failed to load " << path ; 
        return 0 ;      
    } 

    assert(f2);

    f2->Summary("F2 ri", 100);

    GDomain<double>* sd = GPropertyLib::getDefaultDomain();

    const char* matname = "FlintGlass" ;

    GPropertyMap<double>* pmap = new GPropertyMap<double>(matname);

    pmap->setStandardDomain(sd);

    const char* ri = GMaterialLib::refractive_index ;

    pmap->addPropertyStandardized(ri, f2 );
   
    GProperty<double>* rip = pmap->getProperty(ri);
    rip->save(TMPDIR, NULL, "f2.npy");




    std::string idir = BFile::CreateDir(TMPDIR, "GPropertyMapTest_Interpolated"); 
    GPropertyMap<double>* imap = pmap->spawn_interpolated(1.f);
    imap->save(idir.c_str());




    std::string mdir = BFile::CreateDir(TMPDIR, "GPropertyMapTest_Material" ); 
    const char* matdir = mdir.c_str() ;
    pmap->save(matdir);

    GPropertyMap<double>* qmap = GPropertyMap<double>::load(matdir, matname, "material");
    assert(qmap);
    qmap->dump("qmap", 10);


    return 0 ; 
}

