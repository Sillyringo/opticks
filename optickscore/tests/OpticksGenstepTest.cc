// TEST=OpticksGenstepTest om-t

#include "OPTICKS_LOG.hh"
#include "NPY.hpp"
#include "BOpticksResource.hh"
#include "OpticksGenstep.hh"

int main(int argc, char** argv)
{
    OPTICKS_LOG(argc, argv); 

    //const char* def = "/usr/local/opticks/opticksdata/gensteps/dayabay/natural/1.npy" ; 
    const char* def = "$DATADIR/gensteps/dayabay/natural/1.npy" ; 
    const char* path = argc > 1 ? argv[1] : def ; 

    bool testgeo(false); 
    BOpticksResource bor(testgeo) ;  // needed ro resolve internal "envvar" DATADIR, see BResourceTest, BFile 

    NPY<float>* np = NPY<float>::load(path) ; 
    if(np == NULL) return 0 ; 

    OpticksGenstep* gs = new OpticksGenstep(np) ; 

    unsigned modulo = 1000 ; 
    unsigned margin = 10 ;  
    gs->dump( modulo, margin ) ; 

    return 0 ; 
}
