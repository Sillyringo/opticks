#include "OPTICKS_LOG.hh"
#include "SPath.hh"
#include "SSys.hh"
#include "SStr.hh"
#include "NPFold.h"

#include "G4Material.hh"
#include "U4Material.hh"
#include "U4MaterialPropertyVector.h"


void test_MakePropertyFold_one_material()
{
    U4Material::LoadOri(); 

    const char* material = SSys::getenvvar("MATERIAL", "Air"); 
    G4Material* mat = G4Material::GetMaterial(material); 
    NPFold* fold = U4Material::MakePropertyFold(mat);
 
    const char* dir = SPath::Resolve("$TMP/U4MaterialPropertyVectorTest", material, DIRPATH ); 
    fold->save(dir); 
}

void test_MakePropertyFold_all_material()
{
    U4Material::LoadOri(); 
    NPFold* fold = U4Material::MakePropertyFold();
    const char* dir = SPath::Resolve("$TMP/U4MaterialPropertyVectorTest", DIRPATH ); 
    fold->save(dir); 
}



int main(int argc, char** argv)
{
    OPTICKS_LOG(argc, argv); 

    //test_MakePropertyFold_one_material(); 
    test_MakePropertyFold_all_material(); 

    return 0 ; 
}
