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

#include <cstdlib>

#include "NPY.hpp"
#include "GLMPrint.hpp"
#include "Types.hpp"

#include "PhotonsNPY.hpp"
#include "RecordsNPY.hpp"
#include "BoundariesNPY.hpp"

#include "OPTICKS_LOG.hh"
// see ggv-/tests/BoundariesNPYTest.cc

int main(int argc, char** argv)
{
    OPTICKS_LOG(argc, argv);


    // npy is lower level than okc- so cannot 
    // bring in Opticks here
    // nevertheless we here need path to geocache 

    const char* idpath = getenv("IDPATH");
    if(idpath == NULL)
    {
       std::cout << argv[0] << " missing envvar IDPATH " << std::endl ; 
       return 0 ;    
    }

    const char* tag = "1" ;
    const char* det = "dayabay" ; 

    NPY<float>* dpho = NPY<float>::load("oxcerenkov", tag, det);

    Types types ; 
    types.dumpFlags();
    types.readMaterials(idpath, "GMaterialLib");
    types.dumpMaterials();

    if(dpho == NULL)
    {
        LOG(warning) << "failed to load evt " ;
        return 0 ; 
    } 

    BoundariesNPY b(dpho);
    b.setTypes(&types);
    b.indexBoundaries();
    b.dump();

    glm::ivec4 sel = b.getSelection() ;
    print(sel, "boundaries selection");

    return 0 ;
}



/*

In [5]: p = oxc_(1)
INFO:env.g4dae.types:loading /usr/local/env/oxcerenkov/1.npy 
-rw-r--r--  1 blyth  staff  39221904 Jun 24 18:11 /usr/local/env/oxcerenkov/1.npy


In [10]: p.view(np.int32)[:,3]
Out[10]: 
array([[   -14,      0,     11,   2057],
       [     0,      1,     10,   3077],
       [   -12,      2,     10,      9],
       ..., 
       [     0, 612838,      0,      4],
       [     0, 612839,      0,      4],
       [     0, 612840,      0,      4]], dtype=int32)

In [11]: p.view(np.int32)[:,3,0]
Out[11]: array([-14,   0, -12, ...,   0,   0,   0], dtype=int32)

In [12]: count_unique(p.view(np.int32)[:,3,0])
Out[12]: 
array([[   -55,     28],
       [   -54,    770],
       [   -51,   8666],
       [   -33,    267],
       [   -32,    693],
       [   -26,     59],
       [   -25,  34010],
       [   -23,   2326],
       [   -21,    744],
       [   -20,    941],
       [   -18,   1650],
       [   -17,  28494],
       [   -16,   1886],
       [   -15,  23733],
       [   -14,    664],
       [   -13,   3661],
       [   -12,   1443],
       [     0, 159004],
       [    12,   5041],
       [    13,  56601],
       [    14,  10188],
       [    15,   3705],
       [    16,  28922],
       [    17,  42319],
       [    18, 180003],
       [    20,     23],
       [    23,   1600],
       [    25,    871],
       [    27,  11816],
       [    32,    149],
       [    51,   2562],
       [    54,      2]])

*/



