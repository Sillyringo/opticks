
#include <limits>

#include "scuda.h"
#include "squad.h"
#include "sphoton.h"
#include "srec.h"
#include "sseq.h"
#include "sstate.h"
#include "stag.h"
#include "sevent.h"
#include "sctx.h"
#include "sdebug.h"
#include "stran.h"


#include "PLOG.hh"
#include "SSys.hh"
#include "SStr.hh"
#include "NP.hh"
#include "NPFold.h"
#include "SPath.hh"
#include "SGeo.hh"
#include "SEvt.hh"
#include "SEvent.hh"
#include "SEventConfig.hh"
#include "SFrameGenstep.hh"
#include "SOpticksResource.hh"
#include "OpticksGenstep.h"
#include "OpticksPhoton.h"
#include "OpticksPhoton.hh"
#include "SComp.h"
//#include "SCF.h"

const plog::Severity SEvt::LEVEL = PLOG::EnvLevel("SEvt", "DEBUG"); 
const int SEvt::GIDX = SSys::getenvint("GIDX",-1) ;
const int SEvt::PIDX = SSys::getenvint("PIDX",-1) ;
const int SEvt::MISSING_INDEX = std::numeric_limits<int>::max() ; 
const char* SEvt::DEFAULT_RELDIR = "ALL" ; 
//const SCF* SEvt::CF = SCF::Create() ;   // TODO: REMOVE 

SEvt* SEvt::INSTANCE = nullptr ; 

SEvt::SEvt()
    :
    cfgrc(SEventConfig::Initialize()),  // config depends on SEventConfig::SetEventMode OR OPTICKS_EVENTMODE envvar 
    index(MISSING_INDEX),
    reldir(DEFAULT_RELDIR),
    selector(new sphoton_selector(SEventConfig::HitMask())),
    evt(new sevent),
    dbg(new sdebug),
    input_photon(nullptr),
    input_photon_transformed(nullptr),
    random(nullptr),
    provider(this),   // overridden with SEvt::setCompProvider for device running from QEvent::init 
    fold(new NPFold),
    cf(nullptr),
    hostside_running_resize_done(false),
    gather_done(false)
{ 
    init(); 
}

const char* SEvt::getSaveDir() const 
{
   return fold->savedir ;  
}
const char* SEvt::getLoadDir() const 
{
   return fold->loaddir ;  
}


/**
SEvt::getSearchCFbase
----------------------

Search for CFBase geometry folder corresponding to event arrays based on 
the loaded/saved SEvt directories. SOpticksResource::SearchCFBase uses::

    SPath::SearchDirUpTreeWithFile(dir, "CSGFoundry/solid.npy")

As the start dirs tried are loaddir and savedir this has no possibility of working prior 
to a load or save having been done. As this is typically used for loaded SEvt 
this is not much of a limitation. 

For this to succeed to find the geometry requires event arrays are saved 
into folders with suitable proximity to the corresponding geometry folders. 

For example use this to load an event and associate it with a geometry, 
allowing dumping or access to the photons in any frame::

    SEvt* sev = SEvt::Load() ;
    const char* cfbase = sev->getSearchCFBase() ;
    const CSGFoundry* fd = CSGFoundry::Load(cfbase);
    sev->setGeo(fd);
    sev->setFrame(39216);
    std::cout << sev->descFull() ; 

See CSG/tests/CSGFoundry_SGeo_SEvt_Test.sh

**/

const char* SEvt::getSearchCFBase() const 
{
    const char* loaddir = getLoadDir();
    const char* savedir = getSaveDir();
    const char* cfbase = nullptr ; 
    if(cfbase == nullptr && loaddir) cfbase = SOpticksResource::SearchCFBase(loaddir) ;
    if(cfbase == nullptr && savedir) cfbase = SOpticksResource::SearchCFBase(savedir) ;
    return cfbase ; 
}


/**
SEvt::init
-----------

Only configures limits, no allocation yet. 
Device side allocation happens in QEvent::setGenstep QEvent::setNumPhoton

Initially SEvt is set as its own SCompProvider, 
allowing U4RecorderTest/SEvt::save to gather the component 
arrays provided from SEvt.

For device running the SCompProvider is overridden to 
become QEvent allowing SEvt::save to persist the 
components gatherered from device buffers. 

**/

void SEvt::init()
{
    LOG(LEVEL) << "[" ; 
    INSTANCE = this ; 
    evt->init(); 
    dbg->zero(); 
    LOG(LEVEL) << evt->desc() ; // mostly zeros at this juncture

    SEventConfig::CompList(comp); 

    LOG(LEVEL) << " SEventConfig::CompMaskLabel "  << SEventConfig::CompMaskLabel() ; 
    LOG(LEVEL) << descComp() ; 

    initInputPhoton(); 
    LOG(LEVEL) << "]" ; 
}


const char* SEvt::INPUT_PHOTON_DIR = SSys::getenvvar("SEvt_INPUT_PHOTON_DIR", "$HOME/.opticks/InputPhotons") ; 

/**
SEvt::LoadInputPhoton
----------------------

Resolving the input string to a path is done in one of two ways:

1. if the string starts with a letter A-Za-z eg "inphoton.npy" or "RandomSpherical10.npy" 
   it is assumed to be the name of a .npy file within the default SEvt_INPUT_PHOTON_DIR 
   of $HOME/.opticks/InputPhotons. 

   Create such files with ana/input_photons.py  

2. if the string does not start with a letter eg /path/to/some/dir/file.npy or $TOKEN/path/to/file.npy 
   it is passed unchanged to  SPath::Resolve

**/

NP* SEvt::LoadInputPhoton() // static 
{
    const char* ip = SEventConfig::InputPhoton(); 
    return ip ? LoadInputPhoton(ip) : nullptr ; 
}

NP* SEvt::LoadInputPhoton(const char* ip)
{
    assert(strlen(ip) > 0 && SStr::EndsWith(ip, ".npy") ); 
    const char* path = SStr::StartsWithLetterAZaz(ip) ?  SPath::Resolve(INPUT_PHOTON_DIR, ip, NOOP) : SPath::Resolve( ip, NOOP ) ; 

    NP* a = NP::Load(path); 
    if( a == nullptr ) LOG(fatal) << " FAILED to load input photon from path " << path << " SEventConfig::InputPhoton " << ip ; 

    assert( a ) ; 
    assert( a->has_shape(-1,4,4) ); 
    assert( a->shape[0] > 0 );  

    LOG(LEVEL) 
        << " SEventConfig::InputPhoton " << ip
        << " path " << path 
        << " a.sstr " << a->sstr()
        ;

    return a ; 
}



/**
SEvt::initInputPhoton
-----------------------

This is invoked by SEvt::init on instanciating the SEvt instance  
The default "SEventConfig::InputPhoton()" is nullptr meaning no input photons.
This can be changed by setting an envvar in the script that runs the executable, eg::

   export OPTICKS_INPUT_PHOTON=CubeCorners.npy
   export OPTICKS_INPUT_PHOTON=$HOME/reldir/path/to/inphoton.npy
 
Or within the code of the executable, typically in the main prior to SEvt instanciation, 
using eg::

   SEventConfig::SetInputPhoton("CubeCorners.npy")
   SEventConfig::SetInputPhoton("$HOME/reldir/path/to/inphoton.npy")

When non-null it is resolved into a path and the array loaded at SEvt instanciation
by SEvt::LoadInputPhoton

**/

void SEvt::initInputPhoton()
{
    NP* ip = LoadInputPhoton() ;
    setInputPhoton(ip); 
}

void SEvt::setInputPhoton(NP* p)
{
    if(p == nullptr) return ; 
    input_photon = p ; 
    assert( input_photon->has_shape(-1,4,4) ); 
    int numphoton = input_photon->shape[0] ; 
    assert( numphoton > 0 ); 
}
 
NP* SEvt::getInputPhoton_() const { return input_photon ; }
NP* SEvt::getInputPhoton() const {  return input_photon_transformed ? input_photon_transformed : input_photon  ; }
bool SEvt::hasInputPhoton() const { return input_photon != nullptr ; }



/**
SEvt::setFrame
------------------

As it is necessary to have the geometry to provide the frame this 
is now split from eg initInputPhotons.  

For simtrace and input photon running with or without a transform 
it is necessary to call this prior to calling QSim::simulate.

**/


const bool SEvt::setFrame_WIDE_INPUT_PHOTON = SSys::getenvbool("SEvt_setFrame_WIDE_INPUT_PHOTON") ; 

void SEvt::setFrame(const sframe& fr )
{
    frame = fr ; 

    if(SEventConfig::IsRGModeSimtrace())
    {   
        NP* gs = SFrameGenstep::MakeCenterExtentGensteps(frame);  
        LOG(LEVEL) << " simtrace gs " << ( gs ? gs->sstr() : "-" ) ; 
        addGenstep(gs); 

        if(frame.is_hostside_simtrace()) setFrame_HostsideSimtrace(); 

    }   
    else if(SEventConfig::IsRGModeSimulate() && hasInputPhoton())
    {   
        assert( genstep.size() == 0 ) ; // cannot mix input photon running with other genstep running  

        addGenstep(MakeInputPhotonGenstep(input_photon, frame)); 

        bool normalize = true ;  // normalize mom and pol after doing the transform 

        NP* ipt = frame.transform_photon_m2w( input_photon, normalize ); 

        if(setFrame_WIDE_INPUT_PHOTON)  // see notes/issues/G4ParticleChange_CheckIt_warnings.rst
        {
            input_photon_transformed = ipt ;
        }
        else
        {
            input_photon_transformed = ipt->ebyte == 8 ? NP::MakeNarrow(ipt) : ipt ;
            // narrow here to prevent immediate A:B difference with Geant4 seeing double precision 
            // and Opticks float precision 
        }

    }   
}


/**
SEvt::setFrame_HostsideSimtrace
---------------------------------

Called from SEvt::setFrame when sframe::is_hostside_simtrace, eg at behest of X4Simtrace::simtrace

**/

void SEvt::setFrame_HostsideSimtrace()
{
    unsigned num_photon_gs = getNumPhotonFromGenstep(); 
    unsigned num_photon_evt = evt->num_photon ; 
    LOG(error) 
         << "frame.is_hostside_simtrace" 
         << " num_photon_gs " << num_photon_gs
         << " num_photon_evt " << num_photon_evt
         ;
    
    assert( num_photon_gs == num_photon_evt ); 
    setNumSimtrace( num_photon_gs ); 

    LOG(info) << " before hostside_running_resize simtrace.size " << simtrace.size() ; 

    hostside_running_resize(); 

    LOG(info) << " after hostside_running_resize simtrace.size " << simtrace.size() ; 

    SFrameGenstep::GenerateSimtracePhotons( simtrace, genstep );
}

void SEvt::setGeo(const SGeo* cf_)
{
    cf = cf_ ; 
}

void SEvt::setFrame(unsigned ins_idx)
{
    if(cf == nullptr) LOG(fatal) << "must SEvt::setGeo before being can access frames " ; 
    assert(cf); 
    sframe fr ; 
    int rc = cf->getFrame(fr, ins_idx) ; 
    assert( rc == 0 );  
    fr.prepare();     

    setFrame(fr); 
}






quad6 SEvt::MakeInputPhotonGenstep(const NP* input_photon, const sframe& fr )
{
    quad6 ipgs ; 
    ipgs.zero(); 
    ipgs.set_gentype( OpticksGenstep_INPUT_PHOTON ); 
    ipgs.set_numphoton(  input_photon->shape[0]  ); 
    fr.m2w.write(ipgs); // copy fr.m2w into ipgs.q2,q3,q4,q5 
    return ipgs ; 
}



void SEvt::setCompProvider(const SCompProvider* provider_)
{
    provider = provider_ ; 
    LOG(LEVEL) << descProvider() ; 
}

bool SEvt::isSelfProvider() const {   return provider == this ; }

std::string SEvt::descProvider() const 
{
    bool is_self_provider = isSelfProvider() ; 
    std::stringstream ss ; 
    ss << "SEvt::descProvider provider: " << provider << " that address is: " << ( is_self_provider ? "SELF" : "another object" ) ; 
    std::string s = ss.str(); 
    return s ; 
}


NP* SEvt::gatherDomain() const
{
    quad4 dom[2] ;
    evt->get_domain(dom[0]);
    evt->get_config(dom[1]);
    NP* domain = NP::Make<float>( 2, 4, 4 );
    domain->read2<float>( (float*)&dom[0] );
    // actually it makes more sense to place metadata on domain than hits 
    // as domain will always be available
    domain->set_meta<unsigned>("hitmask", selector->hitmask );
    domain->set_meta<std::string>("creator", "SEvt::gatherDomain" );
    return domain ;
}

SEvt* SEvt::Get(){ return INSTANCE ; }
void SEvt::Check()
{
    if(INSTANCE == nullptr) std::cout << "FATAL: must instanciate SEvt before using most SEvt methods" << std::endl ; 
    assert(INSTANCE); 
}


void SEvt::AddTag(unsigned stack, float u ){  INSTANCE->addTag(stack,u);  } 
int  SEvt::GetTagSlot(){ return INSTANCE->getTagSlot() ; }
sgs SEvt::AddGenstep(const quad6& q){ Check(); return INSTANCE->addGenstep(q);  }
sgs SEvt::AddGenstep(const NP* a){    Check(); return INSTANCE->addGenstep(a); }
void SEvt::AddCarrierGenstep(){ AddGenstep(SEvent::MakeCarrierGensteps()); }
void SEvt::AddTorchGenstep(){   AddGenstep(SEvent::MakeTorchGensteps());   }


SEvt* SEvt::Load()  // static 
{
    SEvt* se = new SEvt ; 
    se->load() ; 
    return se ; 
}

void SEvt::Clear(){ Check() ; INSTANCE->clear();  }
void SEvt::Save(){  Check() ; INSTANCE->save(); }
void SEvt::Save(const char* dir){                  Check() ; INSTANCE->save(dir); }
void SEvt::Save(const char* dir, const char* rel){ Check() ; INSTANCE->save(dir, rel ); }

void SEvt::SetIndex(int index){ assert(INSTANCE) ; INSTANCE->setIndex(index) ; }
void SEvt::UnsetIndex(){        assert(INSTANCE) ; INSTANCE->unsetIndex() ;  }
int SEvt::GetIndex(){           return INSTANCE ? INSTANCE->getIndex()  :  0 ; }



// SetReldir can be used with the default SEvt::save() changing the last directory element before the index if present
void        SEvt::SetReldir(const char* reldir){ assert(INSTANCE) ; INSTANCE->setReldir(reldir) ; }
const char* SEvt::GetReldir(){  return INSTANCE ? INSTANCE->getReldir() : nullptr ; }

int SEvt::GetNumPhotonFromGenstep(){  return INSTANCE ? INSTANCE->getNumPhotonFromGenstep() : UNDEF ; }
int SEvt::GetNumGenstepFromGenstep(){ return INSTANCE ? INSTANCE->getNumGenstepFromGenstep() : UNDEF ; }
int SEvt::GetNumHit(){  return INSTANCE ? INSTANCE->getNumHit() : UNDEF ; }


NP* SEvt::GatherGenstep() {   return INSTANCE ? INSTANCE->gatherGenstep() : nullptr ; }
NP* SEvt::GetInputPhoton() {  return INSTANCE ? INSTANCE->getInputPhoton() : nullptr ; }
void SEvt::SetInputPhoton(NP* p) {  assert(INSTANCE) ; INSTANCE->setInputPhoton(p) ; }
bool SEvt::HasInputPhoton(){  return INSTANCE ? INSTANCE->hasInputPhoton() : false ; }

/**
SEvt::clear
-------------

HMM: most of the arrays are only relevant to hostside running, 
so its kinda confusing clearing them 

**/

void SEvt::clear()
{
    LOG(LEVEL) << "[" ; 

    genstep.clear();
    gs.clear();
    pho0.clear(); 
    pho.clear(); 
    slot.clear(); 
    photon.clear(); 
    record.clear(); 
    rec.clear(); 
    seq.clear(); 
    prd.clear(); 
    tag.clear(); 
    flat.clear(); 
    simtrace.clear(); 

    if(fold) fold->clear(); 

    LOG(LEVEL) << "]" ; 
}

void SEvt::setIndex(int index_){ index = index_ ; }
void SEvt::unsetIndex(){         index = MISSING_INDEX ; }
int SEvt::getIndex() const { return index ; }

void SEvt::setReldir(const char* reldir_){ reldir = reldir_ ? strdup(reldir_) : nullptr ; }
const char* SEvt::getReldir() const { return reldir ; }



unsigned SEvt::getNumGenstepFromGenstep() const 
{
    assert( genstep.size() == gs.size() ); 
    return genstep.size() ; 
}

/**
SEvt::getNumPhotonFromGenstep
----------------------------------

Total photons by summation over all collected genstep 

**/

unsigned SEvt::getNumPhotonFromGenstep() const 
{
    unsigned tot = 0 ; 
    for(unsigned i=0 ; i < genstep.size() ; i++) tot += genstep[i].numphoton() ; 
    return tot ; 
}


/**
SEvt::addGenstep
------------------

The sgs summary struct of the last genstep is returned. 

**/

sgs SEvt::addGenstep(const NP* a)
{
    int num_gs = a ? a->shape[0] : -1 ; 
    assert( num_gs > 0 ); 
    quad6* qq = (quad6*)a->bytes(); 
    sgs s = {} ; 
    for(int i=0 ; i < num_gs ; i++) s = addGenstep(qq[i]) ; 
    return s ; 
}

bool SEvt::RECORDING = true ;  // TODO: needs to be normally false, Q:what uses this ? 

/**
SEvt::addGenstep
------------------

The GIDX envvar is used whilst debugging to restrict to collecting 
a single genstep chosen by its index.  This is implemented by 
always collecting all genstep labels, but only collecting 
actual gensteps for the enabled index. 

**/


sgs SEvt::addGenstep(const quad6& q_)
{
    dbg->addGenstep++ ; 


    unsigned gentype = q_.gentype(); 
    unsigned matline_ = q_.matline(); 


    bool input_photon_with_normal_genstep = input_photon && OpticksGenstep_::IsInputPhoton(gentype) == false  ; 
    if(input_photon_with_normal_genstep)
    {
        LOG(fatal)
            << "input_photon_with_normal_genstep " << input_photon_with_normal_genstep
            << " MIXING input photons with other gensteps is not allowed "
            << " for example avoid defining OPTICKS_INPUT_PHOTON when doing simtrace"
            ; 
    }
    assert( input_photon_with_normal_genstep  == false ); 


    int gidx = int(gs.size())  ;  // 0-based genstep label index
    bool enabled = GIDX == -1 || GIDX == gidx ; 

    quad6& q = const_cast<quad6&>(q_);   
    if(!enabled) q.set_numphoton(0);   
    // simplify handling of disabled gensteps by simply setting numphoton to zero for them

    if(matline_ >= G4_INDEX_OFFSET )
    {
        unsigned mtindex = matline_ - G4_INDEX_OFFSET ; 
        int matline = cf ? cf->lookup_mtline(mtindex) : 0 ;
        q.set_matline(matline); 

        LOG(debug) 
            << " matline_ " << matline_ 
            << " mtindex " << mtindex
            << " matline " << matline
            ;
    }


    sgs s = {} ;                  // genstep summary struct 
    s.index = genstep.size() ;    // 0-based genstep index since last clear  
    s.photons = q.numphoton() ;   // numphoton in this genstep 
    s.offset = getNumPhotonFromGenstep() ;   // sum numphotons from all previously collected gensteps (since last reset)
    s.gentype = q.gentype() ; 

    gs.push_back(s) ; 
    genstep.push_back(q) ; 

    int tot_photon = s.offset+s.photons ; 

    if(enabled) LOG(debug) << " s.desc " << s.desc() << " gidx " << gidx << " enabled " << enabled << " tot_photon " << tot_photon ; 

    if( tot_photon != evt->num_photon )
    {
        setNumPhoton(tot_photon); 
    }
    return s ; 
}

/**
SEvt::setNumPhoton
----------------------

This is called from SEvt::addGenstep, updating evt.num_photon 
according to the additional genstep collected and evt.num_seq/tag/flat/record/rec/prd
depending on the configured max which when zero will keep the counts zero.  



Also called by QEvent::setNumPhoton prior to device side allocations. 

**/

void SEvt::setNumPhoton(unsigned num_photon)
{
    bool num_photon_allowed = int(num_photon) <= evt->max_photon ; 
    if(!num_photon_allowed) LOG(fatal) << " num_photon " << num_photon << " evt.max_photon " << evt->max_photon ;
    assert( num_photon_allowed );

    evt->num_photon = num_photon ; 
    evt->num_seq    = evt->max_seq   > 0 ? evt->num_photon : 0 ;
    evt->num_tag    = evt->max_tag  == 1 ? evt->num_photon : 0 ;
    evt->num_flat   = evt->max_flat == 1 ? evt->num_photon : 0 ;

    evt->num_record = evt->max_record * evt->num_photon ;
    evt->num_rec    = evt->max_rec    * evt->num_photon ;
    evt->num_prd    = evt->max_prd    * evt->num_photon ;

    LOG(debug)
        << " evt->num_photon " << evt->num_photon
        << " evt->num_tag " << evt->num_tag
        << " evt->num_flat " << evt->num_flat
        ;

    hostside_running_resize_done = false ;    
    gather_done = false ;    // hmm perhaps should be in ::clear 
}

void SEvt::setNumSimtrace(unsigned num_simtrace)
{
    bool num_simtrace_allowed = int(num_simtrace) <= evt->max_simtrace ;
    if(!num_simtrace_allowed) LOG(fatal) << " num_simtrace " << num_simtrace << " evt.max_simtrace " << evt->max_simtrace ;
    assert( num_simtrace_allowed );
    LOG(LEVEL) << " num_simtrace " << num_simtrace ;  

    evt->num_simtrace = num_simtrace ;
}


/**
SEvt::hostside_running_resize
-------------------------------

NB evt->photon, evt->record, ...  pointers are used to hold device side 
addresses for QEvent running and CPU side addresses for SEvt based running.

This makes it necessary to defer resizing, to give QEvent 
time to be instanciated and declare itself as the comp provider 

**/

void SEvt::hostside_running_resize()
{
    bool is_self_provider = isSelfProvider() ; 
    LOG(LEVEL) 
        << " is_self_provider " << is_self_provider 
        << " hostside_running_resize_done " << hostside_running_resize_done
        ;

    assert( hostside_running_resize_done == false ); 
    assert( is_self_provider ); 
    hostside_running_resize_done = true ; 


    if(evt->num_photon > 0) pho.resize(  evt->num_photon );  
    if(evt->num_photon > 0) slot.resize( evt->num_photon ); 

     // HMM: what about tag_slot 

    if(evt->num_photon > 0) 
    { 
        photon.resize(evt->num_photon);
        evt->photon = photon.data() ; 
    }
    if(evt->num_record > 0) 
    {
        record.resize(evt->num_record); 
        evt->record = record.data() ; 
    }
    if(evt->num_rec > 0) 
    {
        rec.resize(evt->num_rec); 
        evt->rec = rec.data() ; 
    }
    if(evt->num_seq > 0) 
    {
        seq.resize(evt->num_seq); 
        evt->seq = seq.data() ; 
    }
    if(evt->num_prd > 0) 
    {
        prd.resize(evt->num_prd); 
        evt->prd = prd.data() ; 
    }
    if(evt->num_tag > 0) 
    {
        tag.resize(evt->num_tag); 
        evt->tag = tag.data() ; 
    }
    if(evt->num_flat > 0) 
    {
        flat.resize(evt->num_flat); 
        evt->flat = flat.data() ; 
    }
    if(evt->num_simtrace > 0) 
    {
        simtrace.resize(evt->num_simtrace); 
        evt->simtrace = simtrace.data() ; 
    }


    LOG(LEVEL) 
        << " is_self_provider " << is_self_provider 
        << std::endl 
        << evt->desc() 
        ; 
}



/**
SEvt::get_gs
--------------

Lookup sgs genstep label corresponding to spho photon label 

**/

const sgs& SEvt::get_gs(const spho& label) const 
{
    assert( label.gs < int(gs.size()) ); 
    const sgs& _gs =  gs[label.gs] ; 
    return _gs ; 
}

unsigned SEvt::get_genflag(const spho& label) const 
{
    const sgs& _gs = get_gs(label);  
    int gentype = _gs.gentype ;
    unsigned genflag = OpticksGenstep_::GenstepToPhotonFlag(gentype); 
    assert( genflag == CERENKOV || genflag == SCINTILLATION || genflag == TORCH ); 
    return genflag ; 
}


/**
SEvt::beginPhoton : only used for hostside running eg with U4RecorderTest
---------------------------------------------------------------------------

**/
void SEvt::beginPhoton(const spho& label)
{
    if(!hostside_running_resize_done) hostside_running_resize(); 

    dbg->beginPhoton++ ; 
    LOG(LEVEL) ; 
    LOG(LEVEL) << label.desc() ; 

    unsigned idx = label.id ; 

    bool in_range = idx < pho.size() ; 
    if(!in_range) LOG(error) 
        << " not in_range " 
        << " idx " << idx 
        << " pho.size  " << pho.size() 
        << " label " << label.desc() 
        ;  
    assert(in_range);  

    unsigned genflag = get_genflag(label);  

    pho0.push_back(label);    // push_back asis for debugging
    pho[idx] = label ;        // slot in the photon label  
    slot[idx] = 0 ;           // slot/bounce incremented only at tail of SEvt::pointPhoton

    current_pho = label ; 
    current_prd.zero() ;   

    sctx& ctx = current_ctx ; 
    ctx.zero(); 

    //LOG(info) << " idx " << idx ; 

    ctx.idx = idx ;  
    ctx.evt = evt ; 
    ctx.prd = &current_prd ;   // current_prd is populated by InstrumentedG4OpBoundaryProcess::PostStepDoIt 

    ctx.p.set_idx(idx); 
    ctx.p.set_flag(genflag); 

    assert( ctx.p.flagmask_count() == 1 ); // should only be a single bit in the flagmask at this juncture 
}

unsigned SEvt::getCurrentPhotonIdx() const { return current_pho.id ; }


/**
SEvt::rjoinPhoton : only used for hostside running
---------------------------------------------------

Called from U4Recorder::PreUserTrackingAction_Optical for G4Track with 
spho label indicating a reemission generation greater than zero.

Note that this will mostly be called for photons that originate from 
scintillation gensteps BUT it will also happen for Cerenkov (and Torch) genstep 
generated photons within a scintillator due to reemission. 

HMM: could directly change photon[idx] via ref ? 
But are here taking a copy to current_photon
and relying on copyback at SEvt::endPhoton

**/
void SEvt::rjoinPhoton(const spho& label)
{
    dbg->rjoinPhoton++ ; 
    LOG(LEVEL); 
    LOG(LEVEL) << label.desc() ; 

    unsigned idx = label.id ; 
    assert( idx < pho.size() );  

    // check labels of parent and child are as expected
    const spho& parent_label = pho[idx]; 
    assert( label.isSameLineage( parent_label) ); 
    assert( label.gn == parent_label.gn + 1 ); 

    const sgs& _gs = get_gs(label);  
    bool expected_gentype = OpticksGenstep_::IsExpected(_gs.gentype); 
    assert(expected_gentype);  
    // within a scintillator the photons from any genstep type may undergo reemission  

    const sphoton& parent_photon = photon[idx] ; 
    unsigned parent_idx = parent_photon.idx() ; 
    assert( parent_idx == idx ); 

    // replace label and current_photon
    pho[idx] = label ;   
    current_pho = label ; 

    int& bounce = slot[idx] ; assert( bounce > 0 );   
    int prior = bounce - 1 ; 

    if( evt->photon )
    {
        //current_photon = photon[idx] ; 
        current_ctx.p = photon[idx] ; 
        sphoton& current_photon = current_ctx.p ; 

        rjoinPhotonCheck(current_photon); 
        current_photon.flagmask &= ~BULK_ABSORB  ; // scrub BULK_ABSORB from flagmask
        current_photon.set_flag(BULK_REEMIT) ;     // gets OR-ed into flagmask 
    }

    // at truncation point and beyond cannot compare or do rejoin fixup
    if( evt->seq && prior < evt->max_seq )
    {
        //current_seq = seq[idx] ; 
        current_ctx.seq = seq[idx] ; 
        sseq& current_seq = current_ctx.seq ; 

        unsigned seq_flag = current_seq.get_flag(prior);
        rjoinSeqCheck(seq_flag); 
        current_seq.set_flag(prior, BULK_REEMIT);  
    }

    // at truncation point and beyond cannot compare or do rejoin fixup
    if( evt->record && prior < evt->max_record )  
    {
        sphoton& rjoin_record = evt->record[evt->max_record*idx+prior]  ; 
        rjoinRecordCheck(rjoin_record, current_ctx.p); 
        rjoin_record.flagmask &= ~BULK_ABSORB ; // scrub BULK_ABSORB from flagmask  
        rjoin_record.set_flag(BULK_REEMIT) ; 
    } 
    // TODO: rec  (compressed record)
}


void SEvt::rjoinRecordCheck(const sphoton& rj, const sphoton& ph  ) const
{
    assert( rj.idx() == ph.idx() );
    unsigned idx = rj.idx();  
    bool d12match = sphoton::digest_match( rj, ph, 12 ); 
    if(!d12match) dbg->d12match_fail++ ; 
    if(!d12match) ComparePhotonDump(rj, ph) ; 
    if(!d12match) std::cout 
        << " idx " << idx
        << " slot[idx] " << slot[idx]
        << " evt.max_record " << evt->max_record 
        << " d12match " << ( d12match ? "YES" : "NO" ) << std::endl
        ;
    assert( d12match ); 
}

void SEvt::ComparePhotonDump(const sphoton& a, const sphoton& b )  // static
{
    unsigned a_flag = a.flag() ; 
    unsigned b_flag = a.flag() ; 
    std::cout 
        << " a.flag "  << OpticksPhoton::Flag(a_flag)  
        << std::endl 
        << a.desc()
        << std::endl 
        << " b.flag "  << OpticksPhoton::Flag(b_flag)   
        << std::endl 
        << b.desc()
        << std::endl 
        ;
}



/**
SEvt::rjoinPhotonCheck
------------------------

Would expect all rejoin to have BULK_ABSORB, but with 
very artifical single volume of Scintillator geometry keep getting photons 
hitting world boundary giving MISS. 

What about perhaps reemission immediately after reemission ? Nope, I think 
a new point would always be minted so the current_photon flag should 
never be BULK_REEMIT when rjoin runs. 

**/

void SEvt::rjoinPhotonCheck(const sphoton& ph ) const 
{
    dbg->rjoinPhotonCheck++ ; 

    unsigned flag = ph.flag();
    unsigned flagmask = ph.flagmask ; 

    bool flag_AB = flag == BULK_ABSORB ;  
    bool flag_MI = flag == MISS ;  
    bool flag_xor = flag_AB ^ flag_MI ;  

    if(flag_AB) dbg->rjoinPhotonCheck_flag_AB++ ; 
    if(flag_MI) dbg->rjoinPhotonCheck_flag_MI++ ; 
    if(flag_xor) dbg->rjoinPhotonCheck_flag_xor++ ; 

    bool flagmask_AB = flagmask & BULK_ABSORB  ; 
    bool flagmask_MI = flagmask & MISS ; 
    bool flagmask_or = flagmask_AB | flagmask_MI ; 

    if(flagmask_AB) dbg->rjoinPhotonCheck_flagmask_AB++ ; 
    if(flagmask_MI) dbg->rjoinPhotonCheck_flagmask_MI++ ; 
    if(flagmask_or) dbg->rjoinPhotonCheck_flagmask_or++ ; 
    
    bool expect = flag_xor && flagmask_or ; 
    if(!expect) LOG(fatal)
        << "rjoinPhotonCheck : unexpected flag/flagmask" 
        << " flag_AB " << flag_AB
        << " flag_MI " << flag_MI
        << " flag_xor " << flag_xor
        << " flagmask_AB " << flagmask_AB
        << " flagmask_MI " << flagmask_MI
        << " flagmask_or " << flagmask_or
        << ph.descFlag()
        << std::endl
        << ph.desc()
        << std::endl
        ;
    assert(expect); 

}

void SEvt::rjoinSeqCheck(unsigned seq_flag) const 
{
    dbg->rjoinSeqCheck++ ; 

    bool flag_AB = seq_flag == BULK_ABSORB ;
    bool flag_MI = seq_flag == MISS ;
    bool flag_xor = flag_AB ^ flag_MI ;          

    if(flag_AB)  dbg->rjoinSeqCheck_flag_AB++ ; 
    if(flag_MI)  dbg->rjoinSeqCheck_flag_MI++ ; 
    if(flag_xor) dbg->rjoinSeqCheck_flag_xor++ ; 

    if(!flag_xor) LOG(fatal) << " flag_xor FAIL " << OpticksPhoton::Abbrev(seq_flag) << std::endl ;  
    assert( flag_xor ); 
}

/**
SEvt::pointPhoton : only used for hostside running
---------------------------------------------------

Invoked from U4Recorder::UserSteppingAction_Optical to cause the 
current photon to be recorded into record vector. 

The pointPhoton and finalPhoton methods need to do the hostside equivalent of 
what CSGOptiX/CSGOptiX7.cu:simulate does device side,
so have setup the environment to match::

ctx.point looks like it is populating sevent arrays, but actually are also populating 
the vectors thanks to setting of the sevent pointers in SEvt::hostside_running_resize
so that the pointers follow around the vectors as they get reallocated. 

TODO: truncation : bounce < max_bounce 
at truncation ctx.point/ctx.trace stop writing anything but bounce keeps incrementing 
**/

void SEvt::pointPhoton(const spho& label)
{
    dbg->pointPhoton++ ; 

    assert( label.isSameLineage(current_pho) ); 
    unsigned idx = label.id ; 
    sctx& ctx = current_ctx ; 
    assert( ctx.idx == idx ); 
    bool first_point = ctx.p.flagmask_count() == 1 ; 
    int& bounce = slot[idx] ; 

    if(first_point == false) ctx.trace(bounce); 
    ctx.point(bounce); 

    sseq& seq = ctx.seq ;  

    LOG(LEVEL) 
        << " idx " << idx 
        << " bounce " << bounce 
        << " first_point " << first_point 
        << " evt.max_record " << evt->max_record
        << " evt.max_rec    " << evt->max_rec
        << " evt.max_seq    " << evt->max_seq
        << " evt.max_prd    " << evt->max_prd
        << " evt.max_tag    " << evt->max_tag
        << " evt.max_flat    " << evt->max_flat
        << " label.desc " << label.desc() 
        << " seq.desc_seqhis " << seq.desc_seqhis() ; 
        ;

    bounce += 1 ; 
}

/**
SEvt::addTag
-------------

HMM: this needs to be called following every random consumption ... see U4Random::flat 

**/

void SEvt::addTag(unsigned tag, float flat)
{
    if(evt->tag == nullptr) return  ; 
    stagr& tagr = current_ctx.tagr ; 
    unsigned idx = current_ctx.idx ; 

    if( int(idx) == PIDX ) 
    {
        LOG(info) 
             << " idx " << idx
             << " PIDX " << PIDX
             << " tag " << tag 
             << " flat " << flat 
             << " evt.tag " << evt->tag 
             << " tagr.slot " << tagr.slot
              ; 
    }
    tagr.add(tag,flat)  ; 


    if(random)
    {
        int flat_cursor = random->getFlatCursor(); 
        assert( flat_cursor > -1 ); 
        double flat_prior = random->getFlatPrior(); 
        bool cursor_slot_match = unsigned(flat_cursor) == tagr.slot ; 
        if(!cursor_slot_match)
        {
            LOG(error)
                << " idx " << idx
                << " cursor_slot_match " << cursor_slot_match
                << " flat " << flat 
                << " tagr.slot " << tagr.slot 
                << " ( from SRandom "
                << " flat_prior " << flat_prior 
                << " flat_cursor " << flat_cursor 
                << "  ) " 
                << std::endl
                << " MISMATCH MEANS ONE OR MORE PRIOR CONSUMPTIONS WERE NOT TAGGED "
                ;
                ; 
        }
        assert( cursor_slot_match ); 
    }



}

int SEvt::getTagSlot() const 
{
    if(evt->tag == nullptr) return -1 ; 
    const stagr& tagr = current_ctx.tagr ; 
    return tagr.slot ; 
}


/**
SEvt::finalPhoton : only used for hostside running
------------------------------------------------------

Canonically called from U4Recorder::PostUserTrackingAction_Optical

**/

void SEvt::finalPhoton(const spho& label)
{
    dbg->finalPhoton++ ; 
    LOG(LEVEL) << label.desc() ; 
    assert( label.isSameLineage(current_pho) ); 
    unsigned idx = label.id ; 
    sctx& ctx = current_ctx ; 
    assert( ctx.idx == idx ); 

    ctx.end(); 
    evt->photon[idx] = ctx.p ;
}

void SEvt::checkPhotonLineage(const spho& label) const 
{
    assert( label.isSameLineage(current_pho) ); 
}



////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////
///////// below methods handle gathering arrays and persisting, not array content //////////
////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////


NP* SEvt::gatherPho0() const { return NP::Make<int>( (int*)pho0.data(), int(pho0.size()), 4 ); }
NP* SEvt::gatherPho() const {  return NP::Make<int>( (int*)pho.data(), int(pho.size()), 4 ); }
NP* SEvt::gatherGS() const {   return NP::Make<int>( (int*)gs.data(),  int(gs.size()), 4 );  }


NP* SEvt::gatherGenstep() const { return NP::Make<float>( (float*)genstep.data(), int(genstep.size()), 6, 4 ) ; }

NP* SEvt::gatherPhoton() const 
{ 
    if( evt->photon == nullptr ) return nullptr ; 
    NP* p = makePhoton(); 
    p->read2( (float*)evt->photon ); 
    return p ; 
} 
NP* SEvt::gatherRecord() const 
{ 
    if( evt->record == nullptr ) return nullptr ; 
    NP* r = makeRecord(); 
    r->read2( (float*)evt->record ); 
    return r ; 
} 
NP* SEvt::gatherRec() const 
{ 
    if( evt->rec == nullptr ) return nullptr ; 
    NP* r = makeRec(); 
    r->read2( (short*)evt->rec ); 
    return r ; 
} 
NP* SEvt::gatherSeq() const 
{ 
    if( evt->seq == nullptr ) return nullptr ; 
    NP* s = makeSeq(); 
    s->read2( (unsigned long long*)evt->seq ); 
    return s ; 
} 
NP* SEvt::gatherPrd() const 
{ 
    if( evt->prd == nullptr ) return nullptr ; 
    NP* p = makePrd(); 
    p->read2( (float*)evt->prd ); 
    return p ; 
} 
NP* SEvt::gatherTag() const 
{ 
    if( evt->tag == nullptr ) return nullptr ; 
    NP* p = makeTag(); 
    p->read2( (unsigned long long*)evt->tag ); 
    return p ; 
} 

NP* SEvt::gatherFlat() const 
{ 
    if( evt->flat == nullptr ) return nullptr ; 
    NP* p = makeFlat(); 
    p->read2( (float*)evt->flat ); 
    return p ; 
} 
NP* SEvt::gatherSimtrace() const 
{ 
    if( evt->simtrace == nullptr ) return nullptr ; 
    NP* p = makeSimtrace(); 
    p->read2( (float*)evt->simtrace ); 
    return p ; 
}






NP* SEvt::makePhoton() const 
{
    return NP::Make<float>( evt->num_photon, 4, 4 ); 
}
NP* SEvt::makeRecord() const 
{ 
    NP* r = NP::Make<float>( evt->num_photon, evt->max_record, 4, 4 ); 
    r->set_meta<std::string>("rpos", "4,GL_FLOAT,GL_FALSE,64,0,false" );  // eg used by examples/UseGeometryShader
    return r ; 
}
NP* SEvt::makeRec() const 
{
    NP* r = NP::Make<short>( evt->num_photon, evt->max_rec, 2, 4);   // stride:  sizeof(short)*2*4 = 2*2*4 = 16   
    r->set_meta<std::string>("rpos", "4,GL_SHORT,GL_TRUE,16,0,false" );  // eg used by examples/UseGeometryShader
    return r ; 
}
NP* SEvt::makeSeq() const 
{
    return NP::Make<unsigned long long>( evt->num_seq, 2); 
}
NP* SEvt::makePrd() const 
{
    return NP::Make<float>( evt->num_photon, evt->max_prd, 2, 4); 
}

/**
SEvt::makeTag
---------------

The evt->max_tag are packed into the stag::NSEQ of each *stag* struct 

For example with stag::NSEQ = 2 there are two 64 bit "unsigned long long" 
in the *stag* struct which with 5 bits per tag is room for 2*12 = 24 tags  

**/

NP* SEvt::makeTag() const 
{
    assert( sizeof(stag) == sizeof(unsigned long long)*stag::NSEQ ); 
    return NP::Make<unsigned long long>( evt->num_photon, stag::NSEQ);   // 
}
NP* SEvt::makeFlat() const 
{
    assert( sizeof(sflat) == sizeof(float)*sflat::SLOTS ); 
    return NP::Make<float>( evt->num_photon, sflat::SLOTS );   // 
}
NP* SEvt::makeSimtrace() const 
{
    return NP::Make<float>( evt->num_simtrace, 4, 4 ); 
}







// SCompProvider methods

std::string SEvt::getMeta() const 
{
    return meta ; 
}



/**
SEvt::gatherComponent
------------------------

NB this is for hostside running only, for device-side running 
the SCompProvider is the QEvent not the SEvt, so this method
is not called

**/

NP* SEvt::gatherComponent(unsigned comp) const 
{
    unsigned mask = SEventConfig::CompMask(); 
    return mask & comp ? gatherComponent_(comp) : nullptr ; 
}

NP* SEvt::gatherComponent_(unsigned comp) const 
{
    NP* a = nullptr ; 
    switch(comp)
    {   
        case SCOMP_INPHOTON:  a = getInputPhoton() ; break ;   

        case SCOMP_GENSTEP:   a = gatherGenstep()  ; break ;   
        case SCOMP_DOMAIN:    a = gatherDomain()   ; break ;   
        case SCOMP_PHOTON:    a = gatherPhoton()   ; break ;   
        case SCOMP_RECORD:    a = gatherRecord()   ; break ;   
        case SCOMP_REC:       a = gatherRec()      ; break ;   
        case SCOMP_SEQ:       a = gatherSeq()      ; break ;   
        case SCOMP_PRD:       a = gatherPrd()      ; break ;   
        case SCOMP_TAG:       a = gatherTag()      ; break ;   
        case SCOMP_FLAT:      a = gatherFlat()     ; break ;   

        case SCOMP_SEED:      a = gatherSeed()     ; break ;   
        case SCOMP_HIT:       a = gatherHit()      ; break ;   
        case SCOMP_SIMTRACE:  a = gatherSimtrace() ; break ;   
    }   
    return a ; 
}


NP* SEvt::gatherSeed() const   // COULD BE IMPLEMENTED : IF NEEDED TO DEBUG  SLOT->GS ASSOCIATION 
{ 
    LOG(fatal) << " not implemented for hostside running : getting this error indicates CompMask mixup " ; 
    assert(0); 
    return nullptr ; 
}
NP* SEvt::gatherHit() const   // TODO: IMPLEMENT THIS 
{ 
    LOG(fatal) << " not yet implemented for hostside running : getting this error indicates CompMask mixup" ; 
    assert(0); 
    return nullptr ; 
}




/**
SEvt::gatherGenstep
-----------------

The returned array takes a full copy of the genstep quad6 vector
with all gensteps collected since the last SEvt::clear. 
The array is thus independent from quad6 vector, and hence is untouched
by SEvt::clear 

**/




void SEvt::saveGenstep(const char* dir) const  // HMM: NOT THE STANDARD SAVE 
{
    NP* a = gatherGenstep(); 
    if(a == nullptr) return ; 
    LOG(LEVEL) << a->sstr() << " dir " << dir ; 
    a->save(dir, "gs.npy"); 
}

std::string SEvt::descGS() const 
{
    std::stringstream ss ; 
    for(unsigned i=0 ; i < getNumGenstepFromGenstep() ; i++) ss << gs[i].desc() << std::endl ; 
    std::string s = ss.str(); 
    return s ; 
}

std::string SEvt::descDir() const 
{
    const char* savedir = getSaveDir(); 
    const char* loaddir = getLoadDir(); 
    std::stringstream ss ; 
    ss 
       << " savedir " << ( savedir ? savedir : "-" )
       << std::endl
       << " loaddir " << ( loaddir ? loaddir : "-" )
       << std::endl
       ;
    std::string s = ss.str(); 
    return s ; 
}

std::string SEvt::descFold() const 
{
    return fold->desc(); 
}


std::string SEvt::desc() const 
{
    std::stringstream ss ; 
    ss << evt->desc()
       << std::endl
       << descDir()
       << std::endl
       << dbg->desc()
       ; 
    std::string s = ss.str(); 
    return s ; 
}

/**
SEvt::gather
--------------------------

Collects the components configured by SEventConfig::CompMask
into NPFold from the SCompProvider which can either be:

* this SEvt instance for hostside running, eg U4RecorderTest, X4SimtraceTest
* the qudarap/QEvent instance for deviceside running, eg G4CXSimulateTest

**/

void SEvt::gather() 
{
    if(gather_done) 
    {
        LOG(error) << "gather_done already skip gather " ; 
        return ; 
    }

    gather_done = true ;   // SEvt::setNumPhoton which gets called by adding gensteps resets this to false

    for(unsigned i=0 ; i < comp.size() ; i++)
    {
        unsigned cmp = comp[i] ;   
        const char* k = SComp::Name(cmp);    
        NP* a = provider->gatherComponent(cmp); 
        LOG(LEVEL) << " k " << std::setw(15) << k << " a " << ( a ? a->brief() : "-" ) ; 
        if(a == nullptr) continue ;  
        fold->add(k, a); 
    }
    fold->meta = provider->getMeta();  
    // persisted metadata will now be in NPFold_meta.txt (previously fdmeta.txt)
}



/**
SEvt::save
--------------

This was formerly implemented up in qudarap/QEvent but it makes no 
sense for CPU only tests that need to save events to reach up to qudarap 
to control persisting. 

The component arrays are gathered by SEvt::gather_components
into the NPFold and then saved. Which components to gather and save 
are configured via SEventConfig::SetCompMask using the SComp enumeration. 

The arrays are gathered from the SCompProvider object, which 
may be QEvent for on device running or SEvt itself for U4Recorder 
Geant4 tests. 

SEvt::save persists NP arrays into the default directory 
or the directory argument provided.

The FALLBACK_DIR which is used for the SEvt::DefaultDir is obtained from SEventConfig::OutFold 
which is normally "$DefaultOutputDir" $TMP/GEOM/ExecutableName
This can be overriden using SEventConfig::SetOutFold or by setting the 
envvar OPTICKS_OUT_FOLD.

It is normally much easier to use the default of "$DefaultOutputDir" as this 
takes care of lots of the bookkeeping automatically.
However in some circumstances such as with the B side of aligned running (U4RecorderTest) 
it is appropriate to use the override code or envvar to locate B side outputs together 
with the A side. 



**/


void SEvt::save() 
{
    const char* dir = DefaultDir(); 
    LOG(LEVEL) << "SGeo::DefaultDir " << dir ; 
    save(dir); 
}
void SEvt::load()
{
    const char* dir = DefaultDir(); 
    LOG(LEVEL) << "SGeo::DefaultDir " << dir ; 
    load(dir); 
}

const char* SEvt::DefaultDir() // static
{
    return SGeo::DefaultDir() ; 
}


void SEvt::save(const char* base, const char* reld1 ) 
{
    const char* dir = SPath::Resolve(base, reld1, DIRPATH); 
    save(dir); 
}

void SEvt::save(const char* base, const char* reld1, const char* reld2 ) 
{
    const char* dir = SPath::Resolve(base, reld1, reld2,  DIRPATH); 
    save(dir); 
}




/**
SEvt::getOutputDir
--------------------

Returns the directory that will be used by SEvt::save so long as
the same base_ argument is used, which may be nullptr to use the default. 

**/

const char* SEvt::getOutputDir(const char* base_) const 
{
    const char* base = base_ ? base_ : SGeo::DefaultDir() ; 
    bool with_index = index != MISSING_INDEX ;  
    const char* dir = with_index ? 
                                SPath::Resolve(base, reldir, index, DIRPATH ) 
                             :
                                SPath::Resolve(base, reldir,  DIRPATH) 
                             ;
    return dir ;  
}


/**
SEvt::save
------------

If an index has been set with SEvt::setIndex SEvt::SetIndex 
and not unset with SEvt::UnsetIndex SEvt::unsetIndex
then the directory is suffixed with the index::

    /some/directory/n001
    /some/directory/z000
    /some/directory/p001
   
HMM: what about a save following a gather ? does the download happen twice ?

**/

void SEvt::save(const char* dir_) 
{
    const char* dir = getOutputDir(dir_); 
    LOG(info) << " dir " << dir ; 

    LOG(LEVEL) << "[ gather " ; 
    gather(); 
    LOG(LEVEL) << "] gather " ; 
    
    LOG(LEVEL) << descComponent() ; 
    LOG(LEVEL) << descFold() ; 

    LOG(LEVEL) << "[ fold.save " ; 
    fold->save(dir); 
    LOG(LEVEL) << "] fold.save " ; 

    saveLabels(dir); 
    saveFrame(dir); 
}
void SEvt::load(const char* dir_) 
{
    const char* dir = getOutputDir(dir_); 
    LOG(LEVEL) << " dir " << dir ; 
    if( dir == nullptr ) LOG(fatal) << " null dir : probably missing environment : run script, not executable directly " ;   
    assert(dir); 
    fold->load(dir); 
}




/**
SEvt::saveLabels : hostside running only 
--------------------------------------------

**/

void SEvt::saveLabels(const char* dir) const 
{
    LOG(LEVEL) << "[ dir " << dir ; 

    NP* a0 = gatherPho0();  
    if(a0) a0->save(dir, "pho0.npy"); 

    NP* a = gatherPho();  
    if(a) a->save(dir, "pho.npy"); 

    NP* g = gatherGS(); 
    if(g) g->save(dir, "gs.npy"); 

    LOG(LEVEL) << "] dir " << dir ; 
}


void SEvt::saveFrame(const char* dir) const 
{
    LOG(LEVEL) << "[ dir " << dir ; 
    frame.save(dir); 
    LOG(LEVEL) << "] dir " << dir ; 
}

std::string SEvt::descComp() const 
{
    std::stringstream ss ; 
    ss << "SEvt::descComp " 
       << " comp.size " << comp.size() 
       << " SComp::Desc " << SComp::Desc(comp)
       ; 
    std::string s = ss.str(); 
    return s ; 
}

std::string SEvt::descComponent() const 
{
    const NP* genstep  = fold->get(SComp::Name(SCOMP_GENSTEP)) ; 
    const NP* seed     = fold->get(SComp::Name(SCOMP_SEED)) ;  
    const NP* photon   = fold->get(SComp::Name(SCOMP_PHOTON)) ; 
    const NP* hit      = fold->get(SComp::Name(SCOMP_HIT)) ; 
    const NP* record   = fold->get(SComp::Name(SCOMP_RECORD)) ; 
    const NP* rec      = fold->get(SComp::Name(SCOMP_REC)) ;  
    const NP* seq      = fold->get(SComp::Name(SCOMP_SEQ)) ; 
    const NP* domain   = fold->get(SComp::Name(SCOMP_DOMAIN)) ; 
    const NP* simtrace = fold->get(SComp::Name(SCOMP_SIMTRACE)) ; 

    std::stringstream ss ; 
    ss << "SEvt::descComponent" 
       << std::endl 
       << std::setw(20) << " SEventConfig::CompMaskLabel " << SEventConfig::CompMaskLabel() << std::endl  
       << std::setw(20) << "hit" << " " 
       << std::setw(20) << ( hit ? hit->sstr() : "-" ) 
       << " "
       << std::endl
       << std::setw(20) << "seed" << " " 
       << std::setw(20) << ( seed ? seed->sstr() : "-" ) 
       << " "
       << std::endl
       << std::setw(20) << "genstep" << " " 
       << std::setw(20) << ( genstep ? genstep->sstr() : "-" ) 
       << " "
       << std::setw(30) << "SEventConfig::MaxGenstep" 
       << std::setw(20) << SEventConfig::MaxGenstep()
       << std::endl

       << std::setw(20) << "photon" << " " 
       << std::setw(20) << ( photon ? photon->sstr() : "-" ) 
       << " "
       << std::setw(30) << "SEventConfig::MaxPhoton"
       << std::setw(20) << SEventConfig::MaxPhoton()
       << std::endl
       << std::setw(20) << "record" << " " 
       << std::setw(20) << ( record ? record->sstr() : "-" ) 
       << " " 
       << std::setw(30) << "SEventConfig::MaxRecord"
       << std::setw(20) << SEventConfig::MaxRecord()
       << std::endl
       << std::setw(20) << "rec" << " " 
       << std::setw(20) << ( rec ? rec->sstr() : "-" ) 
       << " "
       << std::setw(30) << "SEventConfig::MaxRec"
       << std::setw(20) << SEventConfig::MaxRec()
       << std::endl
       << std::setw(20) << "seq" << " " 
       << std::setw(20) << ( seq ? seq->sstr() : "-" ) 
       << " " 
       << std::setw(30) << "SEventConfig::MaxSeq"
       << std::setw(20) << SEventConfig::MaxSeq()
       << std::endl
       << std::setw(20) << "domain" << " " 
       << std::setw(20) << ( domain ? domain->sstr() : "-" ) 
       << " "
       << std::endl
       << std::setw(20) << "simtrace" << " " 
       << std::setw(20) << ( simtrace ? simtrace->sstr() : "-" ) 
       << " "
       << std::endl
       ;
    std::string s = ss.str(); 
    return s ; 
}





const NP* SEvt::getPhoton() const { return fold->get(SComp::PHOTON_) ; }
const NP* SEvt::getHit() const {    return fold->get(SComp::HIT_) ; }

unsigned SEvt::getNumPhoton() const { return fold->get_num(SComp::PHOTON_) ; }
unsigned SEvt::getNumHit() const    { return fold->get_num(SComp::HIT_) ; }

void SEvt::getPhoton(sphoton& p, unsigned idx) const  // global
{
    const NP* photon = getPhoton(); 
    sphoton::Get(p, photon, idx ); 
}
void SEvt::getHit(sphoton& p, unsigned idx) const 
{
    const NP* hit = getHit(); 
    sphoton::Get(p, hit, idx ); 
}

/**
SEvt::getLocalPhoton SEvt::getLocalHit
-----------------------------------------

sphoton::iindex instance index used to get instance frame
from (SGeo*)cf which is used to transform the photon  

**/

void SEvt::getLocalPhoton(sphoton& lp, unsigned idx) const 
{
    getPhoton(lp, idx); 

    sframe fr ; 
    getPhotonFrame(fr, lp); 
    fr.transform_w2m(lp); 
}
void SEvt::getLocalHit(sphit& ht, sphoton& lp, unsigned idx) const 
{
    getHit(lp, idx); 

    sframe fr ; 
    getPhotonFrame(fr, lp); 
    fr.transform_w2m(lp); 

    ht.iindex = fr.inst() ; 
    ht.sensor_identifier = fr.sensor_identifier(); 
    ht.sensor_index = fr.sensor_index(); 
}

void SEvt::getPhotonFrame( sframe& fr, const sphoton& p ) const 
{
    assert(cf); 
    cf->getFrame(fr, p.iindex); 
    fr.prepare(); 
}


std::string SEvt::descNum() const 
{
    std::stringstream ss ; 
    ss << "SEvt::descNum" 
       << " getNumGenstepFromGenstep "  <<  getNumGenstepFromGenstep() 
       << " getNumPhotonFromGenstep "   <<  getNumPhotonFromGenstep() 
       << " getNumPhoton(from NPFold) " <<  getNumPhoton()
       << " getNumHit(from NPFold) "    <<  getNumHit()
       << std::endl 
       ;

    std::string s = ss.str(); 
    return s ; 
}

std::string SEvt::descPhoton(unsigned max_print) const 
{
    unsigned num_photon = getNumPhoton(); 
    unsigned num_print = std::min(max_print, num_photon); 

    std::stringstream ss ; 
    ss << "SEvt::descPhoton" 
       << " num_photon " <<  num_photon 
       << " max_print " <<  max_print 
       << " num_print " <<  num_print 
       << std::endl 
       ;

    sphoton p ; 
    for(unsigned idx=0 ; idx < num_print ; idx++)
    {   
        getPhoton(p, idx); 
        ss << p.desc() << std::endl  ;   
    }   

    std::string s = ss.str(); 
    return s ; 
}

/**
SEvt::descLocalPhoton SEvt::descFramePhoton
----------------------------------------------

Note that SEvt::descLocalPhoton uses the frame of the 
instance index of each photon whereas the SEvt::descFramePhoton
uses a single frame that is provided by SEvt::setFrame methods. 

Hence caution wrt which frame is applicable for local photon. 

**/

std::string SEvt::descLocalPhoton(unsigned max_print) const 
{
    unsigned num_photon = getNumPhoton(); 
    unsigned num_print = std::min(max_print, num_photon) ; 

    std::stringstream ss ; 
    ss << "SEvt::descLocalPhoton"
       << " num_photon " <<  num_photon 
       << " max_print " <<  max_print 
       << " num_print " <<  num_print 
       << std::endl 
       ; 

    sphoton lp ; 
    for(unsigned idx=0 ; idx < num_print ; idx++)
    {   
        getLocalPhoton(lp, idx); 
        ss << lp.desc() << std::endl  ;   
    }   
    std::string s = ss.str(); 
    return s ; 
}


std::string SEvt::descFramePhoton(unsigned max_print) const 
{
    unsigned num_photon = getNumPhoton(); 
    unsigned num_print = std::min(max_print, num_photon) ; 

    bool zero_frame = frame.is_zero() ; 


    std::stringstream ss ; 
    ss << "SEvt::descFramePhoton"
       << " num_photon " <<  num_photon 
       << " max_print " <<  max_print 
       << " num_print " <<  num_print 
       << " zero_frame " << zero_frame
       << std::endl 
       ; 

    if(zero_frame) 
    {
        ss << "CANNOT getFramePhoton WITHOUT FRAME SET " << std::endl ; 
    }
    else
    {
        sphoton fp ; 
        for(unsigned idx=0 ; idx < num_print ; idx++)
        {   
            getFramePhoton(fp, idx); 
            ss << fp.desc() << std::endl  ;   
        }   
    }

    std::string s = ss.str(); 
    return s ; 
}


std::string SEvt::descFull(unsigned max_print) const
{
    std::stringstream ss ; 
    ss << "[ SEvt::descFull "  << std::endl ; 
    ss << ( cf ? cf->descBase() : "no-cf" ) << std::endl ; 
    ss << descDir() << std::endl ;  
    ss << descNum() << std::endl ; 
    ss << descComponent() << std::endl ; 

    ss << descPhoton(max_print) << std::endl ; 
    ss << descLocalPhoton(max_print) << std::endl ; 
    ss << descFramePhoton(max_print) << std::endl ; 

    ss << ( cf ? cf->descBase() : "no-cf" ) << std::endl ; 
    ss << ( fold ? fold->desc() : "no-fold" ) << std::endl ; 
    ss << "] SEvt::descFull "  << std::endl ; 
    std::string s = ss.str(); 
    return s ; 
}




/**
SEvt::getFramePhoton SEvt::getFrameHit
---------------------------------------

frame set by SEvt::setFrame is used to transform the photon 

**/

void SEvt::getFramePhoton(sphoton& lp, unsigned idx) const 
{
    getPhoton(lp, idx); 

    bool zero_frame = frame.is_zero() ; 
    if(zero_frame) LOG(fatal) << " must setFrame before can getFramePhoton " ; 
    assert(!zero_frame);  

    frame.transform_w2m(lp); 
}
void SEvt::getFrameHit(sphoton& lp, unsigned idx) const 
{
    getHit(lp, idx); 

    bool zero_frame = frame.is_zero() ; 
    if(zero_frame) LOG(fatal) << " must setFrame before can getFrameHit " ; 
    assert(!zero_frame);  

    frame.transform_w2m(lp); 
}


