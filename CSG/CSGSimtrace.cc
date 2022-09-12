#include "PLOG.hh"
#include "SEventConfig.hh"
#include "SOpticksResource.hh"
#include "SSys.hh"
#include "SEvt.hh"
#include "SSim.hh"
#include "CSGFoundry.h"
#include "CSGSimtrace.hh"
#include "CSGQuery.h"
#include "CSGDraw.h"
#include "NP.hh"

const plog::Severity CSGSimtrace::LEVEL = PLOG::EnvLevel("CSGSimtrace", "DEBUG"); 

int CSGSimtrace::Preinit()    // static
{
    SEventConfig::SetRGModeSimtrace();
    return 0 ; 
}

CSGSimtrace::CSGSimtrace()
    :   
    prc(Preinit()),
    geom(SSys::getenvvar("GEOM", "nmskSolidMaskTail")),  
    sim(SSim::Create()),
    fd(CSGFoundry::Load()),
    evt(new SEvt),
    q(new CSGQuery(fd)),
    d(new CSGDraw(q,'Z')),
    SELECTION(getenv("SELECTION")),
    selection(SSys::getenvintvec("SELECTION",',')),  // when no envvar gives nullptr  
    num_selection(selection && selection->size() > 0 ? selection->size() : 0 ), 
    selection_simtrace(num_selection > 0 ? NP::Make<float>(num_selection, 4, 4) : nullptr ), 
    qss(selection_simtrace ? (quad4*)selection_simtrace->bytes() : nullptr)
{
    init(); 
}

void CSGSimtrace::init()
{
    d->draw("CSGSimtrace");
    frame.set_hostside_simtrace();  
    frame.ce = q->select_prim_ce ; 
    LOG(LEVEL) << " frame.ce " << frame.ce << " SELECTION " << SELECTION << " num_selection " << num_selection ; 
    evt->setFrame(frame);  

    if(selection_simtrace)
    {
        selection_simtrace->set_meta<std::string>("SELECTION", SELECTION) ; 
    }
}

int CSGSimtrace::simtrace()
{
    return qss ? simtrace_selection() : simtrace_all() ; 
}

int CSGSimtrace::simtrace_all()
{
    unsigned num_simtrace = evt->simtrace.size() ;
    int num_intersect = 0 ; 
    for(unsigned i=0 ; i < num_simtrace ; i++)
    {
        quad4& p = evt->simtrace[i] ; 
        bool valid_intersect = q->simtrace(p); 
        if(valid_intersect) num_intersect += 1 ;
    }
    LOG(LEVEL) 
        << " num_simtrace " << num_simtrace 
        << " num_intersect " << num_intersect 
        ; 
    return num_intersect ; 
}

int CSGSimtrace::simtrace_selection()
{
    int num_intersect = 0 ; 
    for(unsigned i=0 ; i < num_selection ; i++)
    {
        int j = (*selection)[i] ; 
        const quad4& p0 = evt->simtrace[j] ; 
        quad4& p = qss[i] ; 
        p = p0 ;   
        bool valid_intersect = q->simtrace(p); 
        if(valid_intersect) num_intersect += 1 ;
    }
    LOG(LEVEL) 
        << " num_selection " << num_selection 
        << " num_intersect " << num_intersect 
        ; 
    return num_intersect ; 
}


void CSGSimtrace::saveEvent()
{
    LOG(LEVEL) ; 
    if(num_selection > 0)
    {
        const char* outdir = evt->getOutputDir(); 
        LOG(LEVEL) << " outdir " << outdir << " num_selection " << num_selection ; 
        selection_simtrace->save(outdir, "simtrace_selection.npy") ; 
    }
    else
    {
        evt->save();  
    }
}



