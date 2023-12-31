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
#include "SLOG.hh"
#include "SArgs.hh"

#include "SYSRAP_LOG.hh"


int main(int argc, char** argv)
{
    SLOG_(argc, argv);
    SYSRAP_LOG__ ; 

    plog::Severity level = info ; 

    sLOG(level, -3) << "-3" ; 
    sLOG(level, -2) << "-2" ; 
    sLOG(level, -1) << "-1" ; 
    sLOG(level, 0) << "0" ; 
    sLOG(level, 1) << "1" ; 
    sLOG(level, 2) << "2" ; 
    sLOG(level, 3) << "3" ; 



    std::cout << R"(


    Use sLOG(level, delta) to adjust logging level 
    required for the output of the message relative to a base level. 
    Note the inversion as are changing the level to get an output:

       -ve : message will appear *more*
       +ve : message will appear *less* 

    This allows a base severity level to be set for a class, 
    with some delta variations off that.

    Usage::

       plog::Severity m_level = info ;  //  typically class member with base severity 

       sLOG(m_level, -1) << "Decrease needed log level below base : so will appear more " ;  
       sLOG(m_level,  0) << "Leave loglevel asis, same as LOG(m_level) << ... " ;  
       sLOG(m_level, +1) << "Increase needed log level above base : so will appear less " ;  


    This avoids manual adjustment of lots of output levels, whilst debugging a class 
    up the base by fixing m_level eg m_level(info) and establish an output level 
    structure : 

    * logging inside loops should have +1/+2 relative to base
    * infrequent high level flow output -2/-1 so they appear usually  

    Then once the class is behaving dial down m_level, to reduce output while 
    still leaving the high level flow visible.

    You can also of course change the cut, which adjusts all levels for the
    entire package not just the class being debugged:

       OKX4Test --cfg4 debug 
       OKX4Test --cfg4 warning 


    To shut a class up set m_level(verbose) then only -2s will appear with 
    default level of info 


    

    Note the limited range::

    enum Severity
    {   
        none = 0,
        fatal = 1,
        error = 2,
        warning = 3,
        info = 4,
        debug = 5,
        verbose = 6 
    };  

)" ; 


    return 0 ; 
}

 
