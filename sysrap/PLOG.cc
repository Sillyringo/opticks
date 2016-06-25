#include <cstring>
#include <iostream>
#include <algorithm>
#include <string>

#include "PLOG.hh"


void PLOG::dump(const char* msg, int argc, char** argv)
{
    std::cerr <<  msg
              << " argc " << argc ;

    for(unsigned i=0 ; i < argc ; i++) std::cerr << argv[i] ; 
    std::cerr << std::endl ;               
}

int PLOG::parse(int argc, char** argv, const char* fallback)
{
    // Simple commandline parse to find global logging level

    std::string ll = fallback ; 
    for(int i=1 ; i < argc ; ++i )
    {
        std::string arg(argv[i]);
        std::transform(arg.begin(), arg.end(), arg.begin(), ::tolower);
        if(arg.compare("--trace")==0)   ll = "VERBOSE" ; 
        if(arg.compare("--verbose")==0) ll = "VERBOSE" ; 
        if(arg.compare("--debug")==0)   ll = "DEBUG" ; 
        if(arg.compare("--info")==0)    ll = "INFO" ; 
        if(arg.compare("--warning")==0) ll = "WARNING" ; 
        if(arg.compare("--error")==0)   ll = "ERROR" ; 
        if(arg.compare("--fatal")==0)   ll = "FATAL" ; 
    }
     
    std::transform(ll.begin(), ll.end(), ll.begin(), ::toupper);
    plog::Severity severity = plog::severityFromString(ll.c_str()) ;

    int level = static_cast<int>(severity); 

    //dump("PLOG::parse", argc, argv );

    return level ;  
}

int PLOG::prefix_parse(int argc, char** argv, const char* fallback, const char* prefix)
{
    // Parse commandline to find project logging level  
    // looking for a single project prefix, eg 
    // with the below commandline and prefix of sysrap
    // the level "error" should be set.
    //
    // When no level is found the fallback level is used.
    //
    //    --okcore info --sysrap error --brap trace --npy trace
    //  
    // Both prefix and the arguments are lowercased before comparison.
    //

    std::string pfx(prefix);
    std::transform(pfx.begin(), pfx.end(), pfx.begin(), ::tolower);
    std::string apfx("--");
    apfx += pfx ;  

    std::string ll(fallback) ;
    for(int i=1 ; i < argc ; ++i )
    {
        std::string arg(argv[i]);
        std::transform(arg.begin(), arg.end(), arg.begin(), ::tolower);
        //std::cerr << arg << std::endl ; 

        if(arg.compare(apfx) == 0 && i + 1 < argc ) ll.assign(argv[i+1]) ;
    }

    std::transform(ll.begin(), ll.end(), ll.begin(), ::toupper);

    const char* llc = ll.c_str();
    plog::Severity severity = strcmp(llc, "TRACE")==0 ? plog::severityFromString("VERBOSE") : plog::severityFromString(llc) ;
    int level = static_cast<int>(severity); 

    //dump("PLOG::prefix_parse", argc, argv );

    return level ; 
}



PLOG::PLOG(int argc, char** argv, const char* fallback)  : level(info)
{
   level = parse(argc, argv, fallback); 
}

PLOG::PLOG(int argc, char** argv, const char* fallback, const char* prefix=NULL)
{
   level = prefix == NULL ? 
                             parse(argc, argv, fallback) 
                          : 
                             prefix_parse(argc, argv, fallback, prefix )
                          ;    
}


const char* PLOG::name()
{
   plog::Severity severity  = static_cast<plog::Severity>(level); 
   return plog::severityToString(severity);
}


