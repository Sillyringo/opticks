#pragma once

#include <cstddef>

#include "SYSRAP_API_EXPORT.hh"

class SYSRAP_API SSys {
  public:
     static const unsigned SIGNBIT32 ;
     static const unsigned OTHERBIT32 ; 
     static const char* fmt(const char* tmpl="hello%u.npy", unsigned val=0);
     static int run(const char* cmd);
     static int exec(const char* exe, const char* path);
     static int npdump(const char* path="$TMP/torchstep.npy", const char* nptype="np.int32", const char* postview=NULL );
     static void WaitForInput(const char* msg="Enter any key to continue...\n");
     static int getenvint( const char* envkey, int fallback=-1 );
     static int atoi_( const char* a );
     static const char* getenvvar( const char* envprefix, const char* envkey, const char* fallback=NULL );
     static const char* getenvvar( const char* envkey );
     static int setenvvar( const char* envprefix, const char* key, const char* value, bool overwrite=true );
     static int setenvvar( const char* ekey, const char* value, bool overwrite=true );
     static bool IsRemoteSession();
     static bool IsVERBOSE();
     static bool IsHARIKARI();
     static bool IsENVVAR(const char* envvar);
     static int GetInteractivityLevel();

};
