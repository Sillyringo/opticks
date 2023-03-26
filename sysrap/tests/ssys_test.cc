// ./ssys_test.sh

#include <string>
#include <iostream>
#include <iomanip>
#include "ssys.h"

void test_popen_0()
{
    const char* cmd = "md5 -q -s hello" ; 
    bool chomp = false ; 
    std::string ret = ssys::popen(cmd, chomp); 

    std::cout 
        << " cmd [" << cmd << "]" 
        << std::endl 
        << " ret [" << ret << "]" 
        << std::endl 
        ; 
}

void test_popen_1()
{
    std::cout << ssys::popen("md5 -q -s hello") << std::endl ; 
}

void test_getenv_()
{
    int i = ssys::getenv_<int>("i",0) ; 
    unsigned u = ssys::getenv_<unsigned>("u",0) ; 
    float f = ssys::getenv_<float>("f",0.f) ; 
    double d = ssys::getenv_<double>("d",0.) ; 
    std::string s = ssys::getenv_<std::string>("s","string") ; 

    std::cout 
        << "i " << i << std::endl 
        << "u " << u << std::endl 
        << "f " << f << std::endl 
        << "d " << d << std::endl 
        << "s " << s << std::endl 
        ; 
}

void test_getenv_vec()
{
    std::vector<int>* ivec = ssys::getenv_vec<int>("IVEC", "1,2,3,4" ); 
    assert( ivec->size() == 4 ); 
    std::cout << "IVEC " << ssys::desc_vec<int>(ivec) << std::endl ; 
}

void test_getenvvec_string_1()
{
    std::vector<std::string>* svec = ssys::getenv_vec<std::string>("SVEC", "A1,B2,C3,D4" ); 
    assert( svec->size() == 4 ); 
    std::cout << "SVEC " << ssys::desc_vec<std::string>(svec) << std::endl ; 
}



void test_getenvvec_string_2()
{
    std::vector<std::string>* svec = ssys::getenv_vec<std::string>("SVEC", "A1,B2,C3,D4" ); 
    std::cout << "SVEC " << ssys::desc_vec<std::string>(svec) << std::endl ; 

    for(int i=0 ; i < svec->size() ; i++)
    {
        const std::string& s = (*svec)[i] ; 
        std::cout << "[" << s << "]" << std::endl ; 
    }
}



void test_getenv_kv()
{
    typedef std::pair<std::string, std::string> KV ; 
    std::vector<KV> kvs ; 

    const char* kk = R"LITERAL(

    HOME
    CHECK
    OPTICKS_HOME
    COMMANDLINE
    ${GEOM}_GEOMList 
    
)LITERAL" ;

    //std::cout << "test_getenv_kv" << kk << std::endl ; 

    ssys::getenv_(kvs, kk); 

    for(int i=0 ; i < int(kvs.size()); i++)
    {
        const KV& kv = kvs[i]; 
        std::cout << std::setw(20) << kv.first << " : " << kv.second << std::endl ; 
    }
}

void test__getenv()
{
    const char* k = "${GEOM}_GEOMList" ; 
    char* v = ssys::_getenv(k) ;  

    std::cout 
        << " k " << k 
        << " v " << ( v ? v : "-" )
        << std::endl
        ;

}

void test_replace_envvar_token()
{
    const char* kk = R"LITERAL(
    HOME
    ${HOME}_HELLO
    WORLD_${HOME}_HELLO

    ALL${VERSION}HELLO 
    ALL${VERSIONX}HELLO 

)LITERAL" ;

    std::stringstream ss(kk) ; 
    std::string str ; 
    while (std::getline(ss, str))  // newlines are swallowed by getline
    {   
        if(str.empty()) continue ;   

        std::string rep = ssys::replace_envvar_token(str.c_str()); 

        std::cout 
            << "[" << str << "]" 
            << "[" << rep << "]" 
            << std::endl
            ; 
    }


}




int main(int argc, char** argv)
{
    /*
    test_popen_0(); 
    test_popen_1();
    test_getenv_vec(); 
    test_getenv_(); 
    test_getenvvec_string_1(); 
    test_getenvvec_string_2(); 
    test__getenv(); 
    test_getenv_kv(); 
    test_replace_envvar_token(); 
    */
    test_getenv_kv(); 

 
    return 0 ; 
}
