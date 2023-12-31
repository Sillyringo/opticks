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

// TEST=SAbbrevTest SAbbrev=INFO om-t

#include <string>
#include <vector>

#include "SAbbrev.hh"

#include "OPTICKS_LOG.hh"

void test_0()
{
    LOG(info); 
    std::vector<std::string> ss = { 
        "Acrylic",
        "Air", 
        "Aluminium",
        "Bialkali",
        "DeadWater",
        "ESR",
        "Foam",
        "GdDopedLS",
        "IwsWater",
        "LiquidScintillator",
        "MineralOil",
        "Nitrogen",
        "NitrogenGas",
        "Nylon",
        "OwsWater",
        "PPE",
        "PVC",
        "Pyrex",
        "Rock",
        "StainlessSteel",
        "Tyvek",
        "UnstStainlessSteel",
        "Vacuum",
        "OpaqueVacuum",
        "Water",
        "GlassSchottF2"
    } ;

    SAbbrev ab(ss);
    ab.dump(); 

}

void test_1()
{
    LOG(info); 
    std::vector<std::string> ss = { 
        "Acrylic",
        "Air", 
        "Aluminium",
        "Bialkali",
        "DeadWater",
        "ESR",
        "Foam",
        "GdDopedLS",
        "IwsWater",
        "LiquidScintillator",
        "MineralOil",
        "Nitrogen",
        "NitrogenGas",
        "Nylon",
        "OwsWater",
        "PPE",
        "PVC",
        "Pyrex",
        "Rock",
        "StainlessSteel",
        "Tyvek",
        "UnstStainlessSteel",
        "Vacuum",
        "OpaqueVacuum",
        "Water",
        "GlassSchottF2",
        "photocathode",
        "photocathode_3inch",
        "photocathode_MCP20inch",
        "photocathode_MCP8inch",
        "photocathode_Ham20inch",
        "photocathode_Ham8inch",
        "photocathode_HZC9inch",
        "G4_STAINLESS-STEEL"
    } ;

    SAbbrev ab(ss);
    ab.dump(); 
}


void test_2()
{
    LOG(info); 
    std::vector<std::string> ss = {
        "PipeSteel",
        "PipeStainlessSteel"
    };
    SAbbrev ab(ss);
    ab.dump(); 
}

void test_3()
{
     SAbbrev::FromString(R"LITERAL(
Copper
PipeAl6061
C4F10
PipeAl2219F
VeloStainlessSteel
Vacuum
PipeBeTV56
PipeSteel316LN
PipeBe
Celazole
PipeTitaniumG5
AW7075
PipeAl6082
FutureFibre
Technora
Brass
PipeSteel
BakeOutAerogel
Rich2CarbonFibre
RichSoftIron
Rich1GasWindowQuartz
Kovar
HpdIndium
HpdWindowQuartz
HpdS20PhCathode
HpdChromium
HpdKapton
Supra36Hpd
RichHpdSilicon
RichHpdVacuum
Rich1Nitrogen
Rich1MirrorCarbonFibre
R1RadiatorGas
Rich1MirrorGlassSimex
Rich1Mirror2SupportMaterial
Rich1G10
Rich1PMI
Rich1DiaphramMaterial
Air
)LITERAL")->dump() ; 

} 






int main(int argc, char** argv )
{
    OPTICKS_LOG(argc, argv);
 
    //test_0(); 
    //test_1(); 
    //test_2(); 
    test_3(); 

    return 0 ; 
}

/*
        "ADTableStainlessSteel": "AS",
        "Acrylic": "Ac",
        "Air": "Ai",
        "Aluminium": "Al",
        "Bialkali": "Bk",
        "DeadWater": "Dw",
        "ESR": "ES",
        "Foam": "Fo",
        "GdDopedLS": "Gd",
        "IwsWater": "Iw",
        "LiquidScintillator": "LS",
        "MineralOil": "MO",
        "Nitrogen": "Ni",
        "NitrogenGas": "NG",
        "Nylon": "Ny",
        "OwsWater": "Ow",
        "PPE": "PP",
        "PVC": "PV",
        "Pyrex": "Py",
        "Rock": "Rk",
        "StainlessSteel": "SS",
        "Tyvek": "Ty",
        "UnstStainlessSteel": "US",
        "Vacuum": "Vm",
        "OpaqueVacuum": "OV",
        "Water": "Wt",
        "GlassSchottF2": "F2"
*/

