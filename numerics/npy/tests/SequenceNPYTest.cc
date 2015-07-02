#include "NPY.hpp"
#include "Types.hpp"
#include "PhotonsNPY.hpp"
#include "RecordsNPY.hpp"
#include "SequenceNPY.hpp"
#include "Index.hpp"

#include "assert.h"
#include "stdlib.h"

int main(int argc, char** argv)
{
    const char* idpath = getenv("IDPATH");
    const char* tag = "1" ;

    NPY<float>* photons = NPY<float>::load("oxcerenkov", tag);
    NPY<short>* records = NPY<short>::load("rxcerenkov", tag);
    NPY<float>* dom = NPY<float>::load("domain","1");
    NPY<int>*   idom = NPY<int>::load("idomain","1");
    unsigned int maxrec = idom->getValue(0,0,3) ; 
    assert(maxrec == 10);

    Types types ; 
    types.readFlags("$ENV_HOME/graphics/ggeoview/cu/photon.h");
    types.dumpFlags();
    //types.readMaterialsOld(idpath, "GMaterialIndexLocal.json");
    types.readMaterials(idpath, "GMaterialIndex");
    types.dumpMaterials();

    RecordsNPY r(records, maxrec);
    r.setTypes(&types);
    r.setDomains(dom);

    SequenceNPY s(photons);
    s.setTypes(&types);
    s.setRecs(&r);

    s.countMaterials();
    s.dumpUniqueHistories();
    s.indexSequences();

    NPY<unsigned long long>* seqhisnpy = s.getSeqHisNpy();
    seqhisnpy->save("/tmp/SequenceNPYTest.npy");

    NPY<unsigned char>* seqidx = s.getSeqIdx();
    seqidx->setVerbose();
    seqidx->save("seqidx", tag);

    return 0 ;
}

