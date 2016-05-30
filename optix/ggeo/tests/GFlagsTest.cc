// op --flags 

#include "Opticks.hh"
#include "GCache.hh"
#include "GFlags.hh"
#include "GAttrSeq.hh"
#include "Index.hpp"

int main(int argc, char** argv)
{
    Opticks ok(argc, argv) ; 

    GCache gc(&ok);

    GFlags gf(&gc);

    GAttrSeq* q = gf.getAttrIndex(); 

    q->dump();

    Index* idx = gf.getIndex();

    idx->setExt(".ini");

    idx->save(gc.getIdPath());



    return 0 ; 
}
