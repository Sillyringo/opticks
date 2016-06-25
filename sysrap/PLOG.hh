
// NB DANGEROUS DEFINES : INCLUDE AS THE LAST HEADER
//    OTHERWISE RANDOMLY REPLACES STRINGS IN SYSTEM HEADERS

#include <cstddef>
#include <plog/Log.h>

// translate from boost log levels to plog 
using plog::fatal ;
using plog::error ;
using plog::warning ;
using plog::info ;
using plog::debug ;
using plog::verbose ;

// hmm dangerous but what alternative 
#define trace plog::verbose 

#include "SYSRAP_API_EXPORT.hh"
struct SYSRAP_API PLOG 
{
    int   level ; 

    PLOG(int argc, char** argv, const char* fallback="INFO");
    PLOG(int argc, char** argv, const char* fallback, const char* prefix );
    const char* name(); 

    static int parse(int argc, char** argv, const char* fallback);
    static int prefix_parse(int argc, char** argv, const char* fallback, const char* prefix);
    static void dump(const char* msg, int argc, char** argv);

};



#include "PLOG_INIT.hh"


