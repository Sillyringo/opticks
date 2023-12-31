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

// TEST=GPtsTest om-t

#include "OPTICKS_LOG.hh"

#include "BFile.hh"
#include "BStr.hh"

#include "Opticks.hh"
#include "GPt.hh"
#include "GPts.hh"
#include "GParts.hh"

#include "GBndLib.hh"
#include "GMeshLib.hh"
#include "GMergedMesh.hh"
#include "GGeoLib.hh"

/**
GPtsTest
============

This is checking the postcache creation of a merged GParts instance
using the persisted higher level GPts. 

Suspect can no longer do this comparison as the GParts has been dropped ?

See notes/issues/x016.rst 

**/


struct testGPts
{
    const Opticks* ok ; 
    const GMeshLib* meshlib ; 
    GBndLib*  bndlib ; 
    const std::vector<const NCSG*>& solids  ; 
    const GMergedMesh* mm ;
    int imm ; 
    GParts* parts ; 
    GPts*   pts ; 
    unsigned verbosity ; 
    unsigned num_mismatch_pt ; 
    std::vector<glm::mat4> mismatch_placements ; 
    GParts* parts2 ; 
    std::string path ; 
    int rc ; 

    testGPts( const Opticks* ok_, const GMeshLib* meshlib_, GBndLib* bndlib_, const GMergedMesh* mm_ ) 
        :
        ok(ok_),
        meshlib(meshlib_),
        bndlib(bndlib_),
        solids(meshlib->getSolids()),
        mm(mm_), 
        imm(mm->getIndex()),
        parts(mm->getParts()),
        pts(mm->getPts()),
        verbosity(1), 
        num_mismatch_pt(0),
        parts2(GParts::Create( ok, pts, solids, &num_mismatch_pt, &mismatch_placements)),
        path(BFile::FormPath("$TMP/ggeo/GPtsTest",BStr::itoa(imm))),
        rc(0)
    {
        assert( num_mismatch_pt == 0);
        assert( mismatch_placements.size() == 0);
        init();
        compare(); 
        save(); 
    } 

    void init()
    {
        //assert( parts ); 
        assert( parts2 ); 
        parts2->setBndLib(bndlib); 
        parts2->close(); 
    }

    void dump()
    {
        if(parts)  parts->dump("parts"); 
        parts2->dump("parts2"); 
    }
    void save()
    {
        LOG(info) << path ; 
        if(parts)  parts->save(path.c_str(), "parts"); 
        parts2->save(path.c_str(), "parts2"); 
    }

    void compare()
    {
        if(parts && parts2)
        {    
            rc = GParts::Compare( parts, parts2, false );
            if( rc > 0)  GParts::Compare( parts, parts2, true ); 
        }
        else
        {
            rc = 0 ; 
            LOG(fatal) << " cannot compare : suspect deferred GParts as standard makes this test useless " ;    
        }         

        LOG(info) 
            << " mm.index " << mm->getIndex()
            << " meshlib.solids " << solids.size() 
            << " RC " << rc 
            ; 
    }
};



int main(int argc, char** argv)
{
    OPTICKS_LOG(argc, argv);

    Opticks ok(argc, argv, "--envkey" );
    ok.configure();
    if(!ok.isDirect())
    {
        LOG(fatal) << "this is a direct only test : that means use --envkey option and have a valid OPTICKS_KEY envvar "  ; 
        return 0 ; 
    }

    GMeshLib* meshlib = GMeshLib::Load(&ok);
    //meshlib->dump(); 
    bool constituents = true ; 
    GBndLib* bndlib = GBndLib::load(&ok, constituents);
    bndlib->closeConstituents();   // required otherwise GParts::close asserts
    

    GGeoLib* geolib = GGeoLib::Load(&ok, bndlib); 
    //geolib->dump("geolib");

    unsigned nmm = geolib->getNumMergedMesh(); 
    LOG(info) << " geolib.nmm " << nmm ; 

    unsigned i0 = 0 ; 
    unsigned i1 = nmm ; 
    //unsigned i0 = 3 ; 
    //unsigned i1 = 4 ; 

    int rc(0) ;  
    for(unsigned i=i0 ; i < i1 ; i++)
    {
        LOG(info) << "testing mm " << i ;    
        GMergedMesh* mm = geolib->getMergedMesh(i);
        testGPts t(&ok, meshlib, bndlib, mm); 
        rc += t.rc ; 
    }

    return rc ;
}

