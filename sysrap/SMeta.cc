#include <iostream>
#include <sstream>
#include <fstream>

#include "SMeta.hh"
#include "SPath.hh"
#include "SLOG.hh"

const plog::Severity SMeta::LEVEL = SLOG::EnvLevel("SMeta", "DEBUG"); 



SMeta* SMeta::Load(const char* dir, const char* name) // static 
{
    int create_dirs = 0 ; // 0:nop 
    const char* path = SPath::Resolve(dir, name, create_dirs); 
    return Load(path);   
}

SMeta* SMeta::Load(const char* path) // static 
{
    std::ifstream in(path, std::ios::in);
    if(!in.is_open()) 
    {   
        LOG(fatal) << "failed to open " << path ; 
        return nullptr;
    }   

    SMeta* sm = new SMeta ; 
    in >> sm->js ; 

    return sm ; 
}





void SMeta::save(const char* dir, const char* reldir, const char* name) const
{
    bool create_dirs = 1 ;  // 1:filepath
    const char* path = SPath::Resolve(dir, reldir, name, create_dirs); 
    save(path); 
}
void SMeta::save(const char* dir, const char* name) const
{
    bool create_dirs = 1 ;  // 1:filepath 
    const char* path = SPath::Resolve(dir, name, create_dirs); 
    save(path); 
}
void SMeta::save(const char* path) const 
{
    std::ofstream out(path, std::ios::out);
    if(!out.is_open())
    {
        LOG(fatal) << "SMeta::save failed to open (directories must exist already unlike with BMeta)" << path ;
        return ;
    }
    out << js ;
    out.close();
}




