#include "TorchStepNPY.hpp"
#include "NPY.hpp"

int main(int argc, char** argv)
{
    TorchStepNPY* m_torchstep ; 

    //const char* config = "target:3153 photons:10000 dir:0,1,0" ;
    const char* config = NULL ;

    m_torchstep = new TorchStepNPY(0, config);

    NPY<float>* npy = m_torchstep->makeNPY();

    m_torchstep->dump();

    npy->save("/tmp/torchstep.npy");


    return 0 ;
}; 
