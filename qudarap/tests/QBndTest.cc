/**
QBndTest.cc
------------

**/

#include "scuda.h"
#include "NPFold.h"
#include "QBnd.hh"

int main(int argc, char** argv)
{
    NP* bnd = NP::Load("$HOME/.opticks/GEOM/$GEOM/CSGFoundry/SSim/stree/standard/bnd.npy") ; 
    std::cout << " bnd " << ( bnd ? bnd->sstr() : "-" ) << std::endl ; 
    if(bnd == nullptr) return 1 ; 

    QBnd qb(bnd) ; 
    qb.save("/tmp/$USER/opticks/QBndTest"); 

    return 0 ; 
}
