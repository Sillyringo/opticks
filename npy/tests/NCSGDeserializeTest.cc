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

/**
Tests directories of multiple trees::

    NCSGDeserializeTest $TMP/tboolean-csg-two-box-minus-sphere-interlocked-py-

**/

#include <iostream>

#include "SSys.hh"
#include "BFile.hh"
#include "BStr.hh"

#include "NPY.hpp"
#include "NCSGList.hpp"
#include "NCSG.hpp"
#include "NNode.hpp"
#include "NGLMExt.hpp"
#include "GLMFormat.hpp"

#include "OPTICKS_LOG.hh"


 
void test_Deserialize(const char* basedir, int verbosity)
{

     NCSGList* ls = NCSGList::Load(basedir, verbosity );
     if( ls == NULL )
     {
          LOG(warning) << "failed to NCSGList::Load from " << basedir ; 
          return ; 
     }

    unsigned ntree = ls->getNumTrees();
    for(unsigned i=0 ; i < ntree ; i++)
    {
        NCSG* tree = ls->getTree(i); 
        nnode* root = tree->getRoot();
        LOG(info) << " root.desc : " << root->desc() ;

        NPY<float>* nodes = tree->getNodeBuffer();
        nodes->dump(); 

    }

}

int main(int argc, char** argv)
{
    OPTICKS_LOG(argc, argv);

    int verbosity = SSys::getenvint("VERBOSITY", 0 );
    LOG(info) << " argc " << argc 
              << " argv[0] " << argv[0] 
              << " VERBOSITY " << verbosity 
              ;  

    test_Deserialize( argc > 1 ? argv[1] : "$TMP/csg_py", verbosity);

    return 0 ; 
}


