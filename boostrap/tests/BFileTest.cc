/*
 * Copyright (c) 2019 Opticks Team. All Rights Reserved.
 *
 * This file is part of Opticks
 * (see https://bitbucket.org/simoncblyth/opticks).
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); 
 * you may not use this file except in compliance with the License.  
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software 
 * distributed under the License is distributed on an "AS IS" BASIS, 
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  
 * See the License for the specific language governing permissions and 
 * limitations under the License.
 */

// om-; TEST=BFileTest om-t 

#include <vector>
#include <string>
#include <cassert>


#include "BOpticksResource.hh"
#include "BFile.hh"

#include "OPTICKS_LOG.hh"


const plog::Severity LEVEL = info ; 


void test_FindFile_(const char* dirlist, const char* sub, const char* name)
{
    std::string aa = BFile::FindFile( dirlist, sub, name );
    if(!aa.empty())
    {
        printf("found sub %s name %s at %s \n", sub, name, aa.c_str());  
    }
    else
    {
        printf("NOT found sub %s name %s \n", sub, name);  
    }
} 

void test_FindFile()
{
   const char* dirlist = "$HOME/.opticks;$OPTICKS_PREFIX/.opticks" ;
   test_FindFile_( dirlist, "OpticksResource", "OpticksColors.json");
}

void test_IsAllowedEnvvar()
{
    assert( BFile::IsAllowedEnvvar("TMP") == true ) ; 
}

void test_ResolveKey()
{
    const char* key = "TMP" ; 
    assert( BFile::IsAllowedEnvvar(key) == true ) ; 
    std::string evalue = BFile::ResolveKey(key) ; 

    LOG(info) << " key " << key << " evalue " << evalue ; 
}





void test_ExistsDir()
{

    std::vector<std::string> ss ; 
    ss.push_back("$OPTICKS_HOME/optickscore/OpticksPhoton.h");
    ss.push_back("$HOME/.opticks/GColors.json");
    ss.push_back("$HOME/.opticks");
    ss.push_back("$HOME/");
    ss.push_back("$HOME");
    ss.push_back("$OPTICKS_HOME");
    ss.push_back("$HOME/$OPTICKS_HOME");
    ss.push_back("$TMP");

    for(unsigned int i=0 ; i < ss.size() ; i++)
    {
       std::string s = ss[i] ;
       std::string x = BFile::FormPath(s.c_str());

       bool xdir = BFile::ExistsDir(s.c_str());
       bool xfile = BFile::ExistsFile(s.c_str());

       bool xdir2 = BFile::ExistsNativeDir(x);
       bool xfile2 = BFile::ExistsNativeFile(x);

       assert( xdir == xdir2 );
       assert( xfile == xfile2 );


       printf("  BFile::FormPath(\"%s\") -->  [%s] dir %d file %d  \n", s.c_str(), x.c_str(), xdir, xfile);
    }
}


void test_CreateDir()
{
   BFile::CreateDir("$TMP/a/b/c");
}

void test_RemoveDir()
{
   BFile::CreateDir("$TMP/a/b/c");
   BFile::RemoveDir("$TMP/a/b/c");
}



void test_RemoveDir_2()
{
   BFile::CreateDir("$TMP","b","c");
   BFile::RemoveDir("$TMP","b","c");
}


void test_RemoveFile()
{
   LOG(info) << "." ; 

   BFile::CreateFile("$TMP/a/b/c.txt");
   BFile::RemoveFile("$TMP/a/b/c.txt");
}




void test_ParentDir()
{
    std::vector<std::string> ss ; 
    ss.push_back("$OPTICKS_HOME/optickscore/OpticksPhoton.h");
    ss.push_back("$HOME/.opticks/GColors.json");
    ss.push_back("C:\\tmp");
    ss.push_back("C:\\tmp\\TestIDPath");
 
    for(unsigned int i=0 ; i < ss.size() ; i++)
    {
       std::string s = ss[i] ;
       std::string x = BFile::FormPath(s.c_str());


       std::string p = BFile::ParentDir(s.c_str());

       LOG(info) 
               << " s " << std::setw(40) << s  
               << " x " << std::setw(40) << x  
               << " p " << std::setw(40) << p
               ;  

    } 

}

void test_ParentParentDir()
{
    const char* path = "/home/blyth/local/opticks/lib/OKX4Test" ; 
    const char* xpp =  "/home/blyth/local/opticks" ; 
 
    std::string spp = BFile::ParentParentDir(path);
    assert( strcmp( spp.c_str(), xpp ) == 0 );
}




void test_FormPath_reldir()
{
    std::string x = BFile::FormPath("$TMP", "some/deep/reldir", "name.txt");

    LOG(info) << "test_FormPath_reldir"
              << " " << x 
              ;


}


void test_FormPath_nulldir()
{
    std::string x = BFile::FormPath(nullptr, "name.txt");
    LOG(info) << "test_FormPath_nulldir"
              << " [" << x  << "]"
              ;

}




void test_FormPath_edge()
{
    std::string x = BFile::FormPath("", "g4ok.gltf");
    LOG(info) << "test_FormPath_edge"
              << " " << x 
              ;
}



void test_FormPath()
{
    std::vector<std::string> ss ; 
    ss.push_back("$OPTICKS_HOME/optickscore/OpticksPhoton.h");
    ss.push_back("$OPTICKS_INSTALL_PREFIX/include/optickscore/OpticksPhoton.h");
    ss.push_back("$OPTICKS_INSTALL_PREFIX/externals/config/geant4.ini") ;
    ss.push_back("$OPTICKS_INSTALL_PREFIX/opticksdata/config/opticksdata.ini") ;

    ss.push_back("$OPTICKS_INSTALL_PREFIX/include/OpticksCore/DemoCfgTest.cfg");
    ss.push_back("$OPTICKS_PREFIX/include/OpticksCore/DemoCfgTest.cfg");
    ss.push_back("$INSTALL_PREFIX/include/OpticksCore/DemoCfgTest.cfg");
    ss.push_back("$PREFIX/include/OpticksCore/DemoCfgTest.cfg");

    ss.push_back("$OPTICKS_EVENT_BASE/evt/dayabay/cerenkov/1") ; 

    ss.push_back("$HOME/.opticks/GColors.json");
    ss.push_back("/path/with/dollar/inside/$TMP");
 
    for(unsigned int i=0 ; i < ss.size() ; i++)
    {
       std::string s = ss[i] ;
       std::string x = BFile::FormPath(s.c_str());

       LOG(info) 
               << " s " << std::setw(40) << s  
               << " x " << std::setw(40) << x  
               ;  
    }
}




void test_Name_ParentDir()
{
    const char* path = "$TMP/opticks/blyth/somefile.txt" ; 

    std::string name = BFile::Name(path) ;
    std::string dir = BFile::ParentDir(path) ;


    LOG(info) << " path " << path
              << " name " << name
              << " dir " << dir
              ;
 
}


void test_ChangeExt()
{
    const char* path = "$TMP/somedir/somefile.txt" ; 
    std::string name = BFile::Name(path) ;
    std::string stem = BFile::Stem(path);
    std::string dir = BFile::ParentDir(path) ;

    std::string chg = BFile::ChangeExt(path, ".json");


    LOG(info) << " path " << path
              << " name " << name
              << " stem " << stem
              << " dir " << dir
              << " chg " << chg
              ;
 

    
}

void test_SomeDir()
{
    //const char* path = "$TMP/somedir/someotherdir" ; 
    //const char* path = "/dd/Geometry/PoolDetails/lvVertiCableTray#pvVertiCable0xbf5e7f0" ;
    const char* path = "/usr/local/opticks/opticksdata/export/DayaBay_VGDX_20140414-1300/g4_00.gltf" ; 

    std::string name = BFile::Name(path) ;
    std::string stem = BFile::Stem(path);
    std::string dir = BFile::ParentDir(path) ;


    LOG(info) 
              << "test_SomeDir"
              << " path " << path
              << " name " << name
              << " stem " << stem
              << " dir " << dir
              ;
 
}

void test_SomePath()
{
    //const char* path = "$TMP/somedir/someotherdir" ; 
    //const char* path = "/dd/Geometry/PoolDetails/lvVertiCableTray#pvVertiCable0xbf5e7f0" ;
    const char* path = "/usr/local/opticks/opticksdata/export/DayaBay_VGDX_20140414-1300/g4_00.gltf" ; 

    std::string name = BFile::Name(path) ;
    std::string stem = BFile::Stem(path);
    std::string dir = BFile::ParentDir(path) ;

    LOG(info) 
              << "test_SomePath"
              << " path " << path
              << " name " << name
              << " stem " << stem
              << " dir " << dir
              ;

    assert( name.compare("g4_00.gltf") == 0 ); 
    assert( stem.compare("g4_00") == 0 ); 
    assert( dir.compare("/usr/local/opticks/opticksdata/export/DayaBay_VGDX_20140414-1300") == 0 ); 
 
}





void test_LastWriteTime()
{
    LOG(info) << "test_LastWriteTime" ; 

    const char* path = "$TMP/a/b/c" ;
    std::time_t* lwt = BFile::LastWriteTime(path);
    std::time_t  now = std::time(NULL) ;


    if(!lwt)
    {
        std::cout 
          << " path " << path 
          << " DOESNT EXIST "
          << std::endl ; 
    }
    else
    {
        std::time_t age = (now - *lwt);
        std::cout 
          << " path " << path 
          << " now " << now
          << " age (s) " << age
          << " BFile::LastWriteTime(path) " << *lwt
          << std::endl 
          ; 
    }
}


void test_SinceLastWriteTime()
{
    LOG(info) << "test_SinceLastWriteTime" ; 
    //const char* path = "$TMP/a/b/c" ;
    const char* path = "$TMP/a/b" ;
    std::time_t* age = BFile::SinceLastWriteTime(path) ;
    if(age)
    {
        std::cout 
          << " path " << path 
          << " age : BFile::SinceLastWriteTime(path) " << *age
          << std::endl 
          ; 
    }
}



void test_LooksLikePath()
{
    LOG(info) << "test_LooksLikePath" ; 

    assert( BFile::LooksLikePath("$TMP/a/b") == true );
    assert( BFile::LooksLikePath("/a/b") == true );
    assert( BFile::LooksLikePath("1,2") == false );
    assert( BFile::LooksLikePath(NULL) == false );
    assert( BFile::LooksLikePath("1") == false );
}


void test_ParentName(const char* path, const char* expect)
{
    std::string pn = BFile::ParentName(path);

    LOG(info) << "test_ParentName"
              << " path [" << path << "]" 
              << " pn [" << pn << "]" 
              << " expect [" << expect << "]" 
              ; 

    if( expect == NULL )
    {
        assert( pn.empty() );
    }
    else
    {
        assert( pn.compare(expect) == 0 );
    }
}


void test_ParentName()
{
    test_ParentName( "/usr/local/opticks/opticksdata/export/DayaBay_VGDX_20140414-1300/g4_00.dae", "DayaBay_VGDX_20140414-1300" );
    test_ParentName( "DayaBay_VGDX_20140414-1300/g4_00.dae", "DayaBay_VGDX_20140414-1300" );
    test_ParentName( "g4_00.dae", NULL );
    test_ParentName( NULL, NULL );
}


void test_SplitPath(const char* path)
{
    std::vector<std::string> elem ; 
    BFile::SplitPath(elem, path);

    LOG(info) << " path " << path 
              << " nelem " << elem.size()
              ;

    for(unsigned i=0 ; i < elem.size() ; i++)
    {
        std::cout 
             << std::setw(4) << i 
             << " " << elem[i]
             << std::endl 
             ; 
    }
}


void test_SplitPath()
{
    const char* idpath_0="/usr/local/opticks/opticksdata/export/DayaBay_VGDX_20140414-1300/g4_00.96ff965744a2f6b78c24e33c80d3a4cd.dae" ; 
    test_SplitPath(idpath_0);

    //const char* idpath_1="/usr/local/opticks/geocache/DayaBay_VGDX_20140414-1300/g4_00.dae/96ff965744a2f6b78c24e33c80d3a4cd/1" ;  
    //test_SplitPath(idpath_1);
}






void test_prefixShorten_1()
{
    std::string path_ = BFile::FormPath("/some/other/dir/cfg4/DsG4OpBoundaryProcess.cc" ); 
    const char* path = path_.c_str();
    std::string abbr = BFile::prefixShorten(path, "$OPTICKS_HOME/" ); 
    LOG(info) 
             << " path [" << path << "]"
             << " abbr [" << abbr << "]"
             ; 

}


void test_prefixShorten_0()
{
    std::string path_ = BFile::FormPath("$OPTICKS_HOME/cfg4/DsG4OpBoundaryProcess.cc" ); 
    const char* path = path_.c_str();
    std::string abbr = BFile::prefixShorten(path, "$OPTICKS_HOME/" ); 
    LOG(info) 
             << " path [" << path << "]"
             << " abbr [" << abbr << "]"
             ; 

}

void test_FileSize()
{
    const char* path0 = "$TMP/test_getBufferSize.npy" ; 
    const char* path1 = "$TMP/test_getBufferSize_NonExisting.npy" ; 
    LOG(info) 
              << " path0 " << path0
              << " FileSize0 " << BFile::FileSize( path0 ) 
              ;

    LOG(info) 
              << " path1 " << path1
              << " FileSize1 " << BFile::FileSize( path1 ) 
              ;
}


void test_CWD()
{
    std::string cwd = BFile::CWD(); 
    LOG(info) << cwd ; 
}

void test_CurrentDirectoryName()
{
    const char* cdn = BFile::CurrentDirectoryName(); 
    LOG(info) << cdn ; 
}

void test_Absolute()
{
    std::vector<std::string> rr = {"tests", ".", "..", "../.." , "../../.." } ; 
    for(unsigned i=0 ; i < rr.size() ; i++)
    {
         std::string r = rr[i] ; 
         std::string a = BFile::Absolute(r.c_str()); 
         std::string c = BFile::AbsoluteCanonical(r.c_str()); 
         LOG(info) 
               << " r " << std::setw(15) << r 
               << " a " << std::setw(50) << a
               << " c " << std::setw(50) << c
               ; 
    } 
}

void test_preparePath()
{
    const char* p = "$TMP/extg4/X4GDMLParserTest/out.gdml" ; 

    std::string s = BFile::preparePath(p) ; 

    LOG(LEVEL) 
        << " p " << p  
        << " s " << s
        ;  
}

void test_OPTICKS_USER_HOME()
{
    const char* p = "$HOME/a/b/c/d" ; 

    std::string s = BFile::preparePath(p) ; 

    LOG(info) 
        << " p " << p  
        << " s " << s
        ;  

}

void test_ResolveScript()
{
    LOG(info); 
    BOpticksResource* rsc = BOpticksResource::Get(NULL) ;  // sets envvar OPTICKS_INSTALL_PREFIX internally 
    assert(rsc); 
    //rsc->Summary();

    const char* script = "tboolean.py" ; 
    std::vector<const char*> fallback_dirs = {
         "$OPTICKS_INSTALL_PREFIX/py/opticks/ana",
         "$OPTICKS_PREFIX/py/opticks/ana",
         "$PREFIX/py/opticks/ana"
        };  

    for(unsigned i=0 ; i < fallback_dirs.size() ; i++)
    {
        const char* fallback_dir = fallback_dirs[i];  
        const char* path = BFile::ResolveScript(script,fallback_dir); 
        std::cout << "BFile::ResolveScript(\"" << script << "\", \"" << fallback_dir << "\") = " << path << std::endl ;    
    }
}

void test_UserTmpPath()
{
    const char* utp = BFile::UserTmpPath(); 
    LOG(info) << utp ; 
}



const char* test_expandvar_ = R"LITERAL(

$TMP/okop/OpSnapTest

)LITERAL";


void test_expandvar()
{
    LOG(info); 
    std::stringstream ss ; 
    ss.str(test_expandvar_); 
    std::string s;
    while (std::getline(ss, s, '\n')) 
    {   
        if(!(s.empty() || s.c_str()[0] == '#')) 
        {   
            std::string str = BFile::expandvar(s.c_str()) ; 
            std::cout 
                << std::setw(30) << s
                << " : "
                << std::setw(30) << str
                << std::endl 
                ;
        }
    }
}

 

int main(int argc, char** argv)
{
   OPTICKS_LOG(argc, argv);

   BOpticksResource* rsc = BOpticksResource::Get(NULL) ;  // sets envvar OPTICKS_INSTALL_PREFIX internally 
   rsc->Summary();

   //test_FindFile();
   //test_ExistsDir();
   //test_CreateDir();
   //test_ParentDir();
   //test_FormPath();
   test_Name_ParentDir();
   //test_ChangeExt();

   //test_FormPath_reldir();
   //test_SomeDir();
   //test_SomePath();
   //test_RemoveDir();
   //test_RemoveDir_2();
   //test_RemoveFile();


   //test_LastWriteTime();
   //test_SinceLastWriteTime();
   //test_LooksLikePath();
   //test_ParentName();
   //test_SplitPath();

   //test_prefixShorten_0();
   //test_prefixShorten_1();

   //test_FileSize();

   //test_FormPath_edge();

   //test_IsAllowedEnvvar();
   //test_ResolveKey();

   //test_CWD(); 
   //test_Absolute(); 
   //test_preparePath();
   //test_ParentParentDir();

   //test_OPTICKS_USER_HOME(); 

   //test_CurrentDirectoryName(); 
   //test_ResolveScript(); 

   //test_UserTmpPath();  
   //test_expandvar();  
   //test_FormPath_nulldir();  

   return 0 ; 
}



