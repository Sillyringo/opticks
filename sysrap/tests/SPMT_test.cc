#include "SPMT.h"

int main(int argc, char** argv)
{
    SPMT* pmt = SPMT::Load(); 
    if(pmt == nullptr) return 1 ; 

    std::cout << pmt->desc() << std::endl ; 

    pmt->save("$FOLD"); 

    NPFold* f = pmt->make_testfold(); 
    f->save("$FOLD/test") ; 

    return 0 ; 
}
