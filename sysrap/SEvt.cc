
#include <limits>
#include <array>

#include "scuda.h"
#include "squad.h"
#include "squadx.h"
#include "stamp.h"

#include "sphoton.h"
#include "srec.h"
#include "sseq.h"
#include "ssys.h"
#include "sstate.h"
#include "stag.h"
#include "sevent.h"
#include "sctx.h"
#include "sdebug.h"
#include "stran.h"
#include "stimer.h"
#include "spath.h"

#include "SLOG.hh"
#include "ssys.h"
#include "SStr.hh"
#include "NP.hh"
#include "NPX.h"
#include "NPFold.h"
#include "SPath.hh"   // TODO: switch over to spath.h 
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

bool SEvt::IsDefined(unsigned val){ return val != UNDEF ; }

stimer* SEvt::TIMER = new stimer ;
void SEvt::TimerStart(){ TIMER->start(); }
double SEvt::TimerDone(){ return TIMER->done() ; }
uint64_t SEvt::TimerStartCount(){ return TIMER->start_count() ; }
std::string SEvt::TimerDesc(){ return TIMER->desc() ; }

NP* SEvt::RUN_META = NP::Make<float>(1) ; 

NP* SEvt::UU = nullptr ; 
NP* SEvt::UU_BURN = nullptr ; // NP::Load("$SEvt__UU_BURN") ; 

const plog::Severity SEvt::LEVEL = SLOG::EnvLevel("SEvt", "DEBUG"); 
const int SEvt::GIDX = ssys::getenvint("GIDX",-1) ;
const int SEvt::PIDX = ssys::getenvint("PIDX",-1) ;
const int SEvt::MISSING_INDEX = std::numeric_limits<int>::max() ; 
const int SEvt::MISSING_INSTANCE = std::numeric_limits<int>::max() ; 
const int SEvt::DEBUG_CLEAR = ssys::getenvint("SEvt__DEBUG_CLEAR",-1) ;


/**
Q: How is the reldir changed to ALL0 ALL1 namely ALL$VERSION
A: Thats done only when using SEvt::HighLevelCreate via envvar expansion. 
**/



std::array<SEvt*, SEvt::MAX_INSTANCE> SEvt::INSTANCES = {{ nullptr, nullptr }} ; 

std::string SEvt::DescINSTANCE()  // static
{
    std::stringstream ss ; 

    ss << "SEvt::DescINSTANCE" 
       << " Count() " << Count()
       << std::endl 
       << " Exists(0) " << ( Exists(0) ? "YES" : "NO " )
       << std::endl 
       << " Exists(1) " << ( Exists(1) ? "YES" : "NO " )
       << std::endl 
       ;   
    std::string str = ss.str(); 
    return str ; 
}







/**
SEvt::SEvt
-----------

Instanciation invokes SEventConfig::Initialize() 

The config used depends on:

1. envvars such as OPTICKS_EVENT_MODE that can change default config values 
2. static SEventConfig method calls done before SEvt instanciation 
   that change the default config values 

**/

SEvt::SEvt()
    :
    cfgrc(SEventConfig::Initialize()),  
    index(MISSING_INDEX),
    instance(MISSING_INSTANCE),
    t_BeginOfEvent(0),
    t_EndOfEvent(0),
    t_PenultimatePoint(0),
    t_LastPoint(0),
    selector(new sphoton_selector(SEventConfig::HitMask())),
    evt(new sevent),
    dbg(new sdebug),
    input_photon(nullptr),
    input_photon_transformed(nullptr),
    g4state(nullptr),
    random(nullptr),
    random_array(nullptr),
    provider(this),   // overridden with SEvt::setCompProvider for device running from QEvent::init 
    fold(new NPFold),
    cf(nullptr),
    hostside_running_resize_done(false),
    gather_done(false),
    is_loaded(false),
    is_loadfail(false),
    numgenstep_collected(0u),   // updated by addGenstep
    numphoton_collected(0u),   // updated by addGenstep
    numphoton_genstep_max(0u),
    clear_count(0),
    gather_total(0),
    genstep_total(0),
    photon_total(0),
    hit_total(0)
{   
    init(); 
}
/**
SEvt::init
-----------

Only configures array maxima, no allocation yet. 
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
    //INSTANCE = this ;    // no-longer automated, rely on static creation methods to set INSTANCES[0] or [1]

    evt->init();    // array maxima set according to SEventConfig values 
    dbg->zero(); 

    LOG(LEVEL) << evt->desc() ; // mostly zeros at this juncture

    SEventConfig::GatherCompList(gather_comp);  // populate gather_comp vector based on GatherCompMask
    SEventConfig::SaveCompList(save_comp);      // populate save_comp vector based on SaveCompMask


    LOG(LEVEL) << " SEventConfig::GatherCompLabel "  << SEventConfig::GatherCompLabel() ;   // CompMaskLabel 
    LOG(LEVEL) << descComp() ; 

    initInputPhoton(); 
    initG4State();        // HMM: G4State not an every-event thing ? first event only ?
    LOG(LEVEL) << "]" ; 
}


const char* SEvt::GetSaveDir(int idx) // static 
{ 
    return Exists(idx) ? Get(idx)->getSaveDir() : nullptr ; 
}
const char* SEvt::getSaveDir() const { return fold->savedir ; }
const char* SEvt::getLoadDir() const { return fold->loaddir ; }
int SEvt::getTotalItems() const { return fold->total_items() ; }

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


const char* SEvt::INPUT_PHOTON_DIR = ssys::getenvvar("SEvt__INPUT_PHOTON_DIR", "$HOME/.opticks/InputPhotons") ; 
/**
SEvt::LoadInputPhoton
----------------------

This is invoked by SEvt::initInputPhoton which is invoked by SEvt::init at instanciation.

Resolving the input string to a path is done in one of two ways:

1. if the string starts with a letter A-Za-z eg "inphoton.npy" or "RandomSpherical10.npy" 
   it is assumed to be the name of a .npy file within the default SEvt__INPUT_PHOTON_DIR 
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
    LOG_IF(fatal, a == nullptr) << " FAILED to load input photon from path " << path << " SEventConfig::InputPhoton " << ip ; 

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
   export OPTICKS_INPUT_PHOTON=$HOME/somedir/path/to/inphoton.npy
 
Or within the code of the executable, typically in the main prior to SEvt instanciation, 
using eg::

   SEventConfig::SetInputPhoton("CubeCorners.npy")
   SEventConfig::SetInputPhoton("$HOME/somedir/path/to/inphoton.npy")

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


/**
SEvt::getInputPhoton_
----------------------

This variant always provides the untransformed input photons.
That will be nullptr unless OPTICKS_INPUT_PHOTON is defined. 

**/
NP* SEvt::getInputPhoton_() const { return input_photon ; }
bool SEvt::hasInputPhoton() const { return input_photon != nullptr ; }


/**
SEvt::getInputPhoton
---------------------

Returns the transformed input photon if present. 
For the transformed photons to  be present it is necessary to have called SEvt::setFrame
That is done from on high by G4CXOpticks::setupFrame which gets invoked by G4CXOpticks::setGeometry

The frame and corresponding transform used can be controlled by several envvars, 
see CSGFoundry::getFrameE. Possible envvars include:

+------------------------------+----------------------------+
| envvar                       | Examples                   |
+==============================+============================+
| INST                         |                            |
+------------------------------+----------------------------+
| MOI                          | Hama:0:1000 NNVT:0:1000    |          
+------------------------------+----------------------------+
| OPTICKS_INPUT_PHOTON_FRAME   |                            |
+------------------------------+----------------------------+


**/
NP* SEvt::getInputPhoton() const {  return input_photon_transformed ? input_photon_transformed : input_photon  ; }
bool SEvt::hasInputPhotonTransformed() const { return input_photon_transformed != nullptr ; }


/**
SEvt::gatherInputPhoton
-------------------------

To avoid issues with inphoton and saving 2nd events, 
treat the inphoton more like other arrays by having a distinct
inphoton copy for each event. 

**/

NP* SEvt::gatherInputPhoton() const 
{
    NP* ip = getInputPhoton(); 
    NP* ipc = ip ? ip->copy() : nullptr ; 
    return ipc ; 
}


/**
SEvt::initG4State
-------------------

Called by SEvt::init but does nothing unless SEventConfig::IsRunningModeG4StateSave()

HMM: is this the right place ? It depends on the RunningMode.  

**/

void SEvt::initG4State()
{
    if(SEventConfig::IsRunningModeG4StateSave())
    {
        LOG(LEVEL) << "SEventConfig::IsRunningModeG4StateSave creating g4state array " ;  
        NP* state = makeG4State(); 
        setG4State(state); 
    }
}

/**
SEvt::makeG4State
---------------------

Not invoked by default. 

See U4Recorder::PreUserTrackingAction_Optical

* HMM: thats a bit spagetti, config control ?  

Item values of 2*17+4=38 is appropriate for the default Geant4 10.4.2 random engine: MixMaxRng 
See::

     g4-cls MixMaxRng 
     g4-cls RandomEngine 
     g4-cls Randomize


TODO: half the size with "unsigned" not "unsigned long", 
to workaround wasteful CLHEP API which uses "unsigned long" but only needs 32 bit elements

**/

NP* SEvt::makeG4State() const 
{
     const char* spec = SEventConfig::G4StateSpec() ; // default spec 1000:38

     std::vector<int> elem ; 
     sstr::split<int>(elem, spec, ':');  
     assert( elem.size() == 2 ); 

     int max_states = elem[0] ; 
     int item_values = elem[1] ;

     NP* a =  NP::Make<unsigned long>(max_states, item_values ); 


     LOG(info) 
         << " SEventConfig::G4StateSpec() " << spec  
         << " max_states " << max_states
         << " item_values " << item_values 
         << " a.sstr " << a->sstr()
         ;
     return a ; 
}

void SEvt::setG4State( NP* state) { g4state = state ; }
NP* SEvt::gatherG4State() const {  return g4state ; } // gather is used prior to persisting, get is used after loading 
const NP* SEvt::getG4State() const {  return fold->get(SComp::Name(SCOMP_G4STATE)) ; }


/**
SEvt::setFrame
------------------

As it is necessary to have the geometry to provide the frame this 
is now split from eg initInputPhotons.  

**simtrace running**
    MakeCenterExtentGensteps based on the given frame. 

**simulate inputphoton running**
    MakeInputPhotonGenstep and m2w (model-2-world) 
    transforms the photons using the frame transform

Formerly(?) for simtrace and input photon running with or without a transform 
it was necessary to call this for every event due to the former call to addFrameGenstep, 
but now that the genstep setup is moved to SEvt::BeginOfEvent it is only needed 
to call this for each frame, usually once only. 

**/


void SEvt::setFrame(const sframe& fr )
{
    frame = fr ; 
    // former call to addFrameGenstep() is relocated to SEvt::BeginOfEvent
    transformInputPhoton();  
}

void SEvt::setFramePlaceholder()
{
    sframe fr = sframe::Fabricate(0.f,0.f,0.f); 
    setFrame(fr); 
}



const bool SEvt::transformInputPhoton_WIDE = ssys::getenvbool("SEvt__transformInputPhoton_WIDE") ; 

/**
SEvt::transformInputPhoton
---------------------------

**/

void SEvt::transformInputPhoton()
{
    bool proceed = SEventConfig::IsRGModeSimulate() && hasInputPhoton() ; 
    LOG(LEVEL) << " proceed " << ( proceed ? "YES" : "NO " ) ; 
    if(!proceed) return ;    

    bool normalize = true ;  // normalize mom and pol after doing the transform 

    NP* ipt = frame.transform_photon_m2w( input_photon, normalize ); 

    if(transformInputPhoton_WIDE)  // see notes/issues/G4ParticleChange_CheckIt_warnings.rst
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




/**
SEvt::addFrameGenstep  (former static SEvt::AddFrameGenstep is removed)
-------------------------------------------------------------------------

This is invoked from SEvt::beginOfEvent together with SEvt::setIndex

For hostside simtrace and input photon running this must be called 
at the start of every event cycle to add the gensteps which trigger 
the allocations for result vectors.  

**/

void SEvt::addFrameGenstep()
{
    LOG(LEVEL); 
    if(SEventConfig::IsRGModeSimtrace())
    { 
        const char* frs = frame.get_frs() ; // nullptr when default -1 : meaning all geometry 
        if(frs)
        {
            LOG(LEVEL) << " non-default frs " << frs << " passed to SEvt::SetReldir " ; 
            SEvt::SetReldir(frs);  
        }
  
        NP* gs = SFrameGenstep::MakeCenterExtentGensteps(frame);  
        LOG(LEVEL) << " simtrace gs " << ( gs ? gs->sstr() : "-" ) ; 
        addGenstep(gs); 

        if(frame.is_hostside_simtrace()) setFrame_HostsideSimtrace(); 
    }   
    else if(SEventConfig::IsRGModeSimulate() && hasInputPhoton())
    {   
        int prior_genstep = genstep.size() ;  
        bool prior_genstep_expected =  prior_genstep == 0 ;

        LOG_IF(fatal, !prior_genstep_expected ) 
            << " CANNOT MIX input photon running with other genstep running  "
            << " index " << index
            << " instance " << instance
            << " prior_genstep_expected " << ( prior_genstep_expected ? "YES" : "NO " )
            << " prior_genstep " << prior_genstep 
            ; 
        assert( prior_genstep_expected ) ;

        addGenstep(MakeInputPhotonGenstep(input_photon, frame)); 
        // transformInputPhoton formerly done here, but thats too late for junosw
    }   
}

const char* SEvt::getFrameId()    const { return frame.getFrameId() ; }
const NP*   SEvt::getFrameArray() const { return frame.getFrameArray() ; }

const char* SEvt::GetFrameId(int idx){    return Exists(idx) ? Get(idx)->getFrameId() : nullptr ; }
const NP*   SEvt::GetFrameArray(int idx){ return Exists(idx) ? Get(idx)->getFrameArray() : nullptr ; } 


/**
SEvt::setFrame_HostsideSimtrace
---------------------------------

Called from SEvt::setFrame when sframe::is_hostside_simtrace, eg at behest of X4Simtrace::simtrace

**/

void SEvt::setFrame_HostsideSimtrace()
{
    unsigned num_photon_gs = getNumPhotonFromGenstep(); 
    unsigned num_photon_evt = evt->num_photon ; 
    LOG(LEVEL) 
         << "frame.is_hostside_simtrace" 
         << " num_photon_gs " << num_photon_gs
         << " num_photon_evt " << num_photon_evt
         ;
    
    assert( num_photon_gs == num_photon_evt ); 
    setNumSimtrace( num_photon_gs ); 

    LOG(LEVEL) << " before hostside_running_resize simtrace.size " << simtrace.size() ; 

    hostside_running_resize(); 

    LOG(LEVEL) << " after hostside_running_resize simtrace.size " << simtrace.size() ; 

    SFrameGenstep::GenerateSimtracePhotons( simtrace, genstep );
}



/**
SEvt::setGeo
-------------

Canonical invokation is from G4CXOpticks::setGeometry 
This connection between the SGeo geometry and SEvt is what allows 
the appropriate instance frame to be accessed. That is vital for 
looking up the sensor_identifier and sensor_index.  

SGeo is a protocol for geometry access fulfilled by both GGeo
and CSGFoundry 

TODO: replace this with stree.h based approach  

**/

void SEvt::setGeo(const SGeo* cf_)
{
    cf = cf_ ; 
}

/**
SEvt::setFrame
-----------------

This method asserts that SEvt::setGeo has been called 
as that is needed to provide access to sframe via 
the SGeo protocol base of either GGeo OR CSGFoundry

TODO: replace this with stree.h based approach 

**/
void SEvt::setFrame(unsigned ins_idx)
{
    LOG_IF(fatal, cf == nullptr) << "must SEvt::setGeo before being can access frames " ; 
    assert(cf); 
    sframe fr ; 
    int rc = cf->getFrame(fr, ins_idx) ; 
    assert( rc == 0 );  
    fr.prepare();     

    setFrame(fr); 
}


//// below impl order matches decl order : KEEP IT THAT WAY 


/**
SEvt::CreateSimtraceEvent
---------------------------

Experimental creation of a new SEvt 
that adopts the sframe of the prior SEvt 
and is configured for hostside Simtrace running.  

The appropriate place use this method is from EndOfRunAction
after standard (non-simtrace) usage of SEvt 
such as saving simulation SEvt is completed.  

**/

SEvt* SEvt::CreateSimtraceEvent()  // static
{
    LOG(LEVEL) << "[" ; 

    SEvt* prior = SEvt::Get(0);  
    if(prior == nullptr ) return nullptr ; 

    sframe& pfr = prior->frame ;   
    pfr.set_hostside_simtrace();

    if( pfr.ce.w == 0.f )
    {
        pfr.ce.w = 200.f ;  
        LOG(LEVEL) 
            << " kludging frame extent, this happens with U4SimulateTest "
            << " as the CSGFoundry geometry is not available causing the "
            << " SEvt sframe to be default "
            ;
    }

    // set_hostside_simtrace into frame which 
    // influences the action of SEvt::setFrame  
    // especially SEvt::setFrame_HostsideSimtrace which 
    // generates simtrace photons
        
    SEventConfig::SetRGModeSimtrace();
    LOG(LEVEL) << " SWITCH : SEventConfig::SetRGModeSimtrace " ; 
    SEvt* ste = new SEvt ;
    ste->setFrame(pfr);   

    LOG(LEVEL) << "] ste.simtrace.size " << ste->simtrace.size() ; 

    return ste ; 
}


/**
SEvt::MakeInputPhotonGenstep
-----------------------------

Now called from SEvt::addFrameGenstep (formerly from SEvt::setFrame)
Note that the only thing taken from the *input_photon* is the 
number of photons so this can work with either local or 
transformed *input_photon*. 

The m2w transform from the frame is copied into the genstep.  
HMM: is that actually used ? Because the frame is also persisted. 

**/

quad6 SEvt::MakeInputPhotonGenstep(const NP* input_photon, const sframe& fr )
{
    LOG(LEVEL) << " input_photon " << NP::Brief(input_photon) ;  

    quad6 ipgs ; 
    ipgs.zero(); 
    ipgs.set_gentype( OpticksGenstep_INPUT_PHOTON ); 
    ipgs.set_numphoton(  input_photon->shape[0]  ); 
    fr.m2w.write(ipgs); // copy fr.m2w into ipgs.q2,q3,q4,q5 
    return ipgs ; 
}


/**
SEvt::setCompProvider
----------------------

This is called for device side running only from QEvent::init

**/

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

/**
SEvt::gatherDomain
--------------------

Create (2,4,4) NP array and populate with quad4 from::

    sevent::get_domain 
    sevent::get_config


NB an alternative route for metadata is the 
SEvt::meta OR QEvent::meta that gets adopted as
the NPFold meta in SEvt::gather_components

**/

NP* SEvt::gatherDomain() const
{
    quad4 dom[2] ;
    evt->get_domain(dom[0]);
    evt->get_config(dom[1]);  // maxima, counts 
    NP* domain = NP::Make<float>( 2, 4, 4 );
    domain->read2<float>( (float*)&dom[0] );
    // actually it makes more sense to place metadata on domain than hits 
    // as domain will always be available
    domain->set_meta<unsigned>("hitmask", selector->hitmask );
    domain->set_meta<std::string>("creator", "SEvt::gatherDomain" );
    domain->set_meta<int>("index", index); 
    domain->set_meta<int>("instance", instance); 

    return domain ;
}




int SEvt::Count()  // static
{
    int count = 0 ; 
    if(Exists(0)) count += 1 ; 
    if(Exists(1)) count += 1 ; 
    return count ; 
}  


SEvt* SEvt::Get_EGPU(){ return SEvt::Get(EGPU) ; }
SEvt* SEvt::Get_ECPU(){ return SEvt::Get(ECPU) ; }

SEvt* SEvt::Get(int idx)  // static
{  
    assert( idx == 0 || idx == 1 );    
    return INSTANCES[idx] ; 
}
void SEvt::Set(int idx, SEvt* inst) // static
{
    assert( idx == 0 || idx == 1 );    
    INSTANCES[idx] = inst ;
    LOG(LEVEL) << " idx " << idx  << " " << DescINSTANCE()  ; 
}

/**
SEvt::Create
-------------

Q: Does all SEvt creation use this ?


**/

SEvt* SEvt::Create(int idx)  // static
{ 
    assert( idx == 0 || idx == 1); 
    SEvt* evt = new SEvt ; 
    evt->setInstance(idx) ; 
    INSTANCES[idx] = evt  ; 
    assert( Get(idx) == evt ); 
    LOG(LEVEL) << " idx " << idx  << " " << DescINSTANCE()  ; 
    return evt  ; 
}

bool SEvt::Exists(int idx)  // static 
{
    return Get(idx) != nullptr ; 
} 
SEvt* SEvt::CreateOrReuse(int idx) 
{  
    SEvt* evt = Exists(idx) ? Get(idx) : Create(idx) ; 
    LOG(LEVEL) << " idx " << idx  << " " << DescINSTANCE()  ; 
    return evt ; 
}
SEvt* SEvt::HighLevelCreateOrReuse(int idx)
{
    SEvt* evt = Exists(idx) ? Get(idx) : HighLevelCreate(idx) ; 
    LOG(LEVEL) << " idx " << idx  << " " << DescINSTANCE()  ; 
    return evt ; 
}


/**
SEvt::CreateOrReuse
---------------------

Creates 0, 1 OR 2 SEvt depending on SEventConfig::IntegrationMode()::

    OPTICKS_INTEGRATION_MODE (aka opticksMode)

+-----------------+----------+----------+--------------------------------------------+
|  opticksMode    | num SEvt | SEvt idx | notes                                      |
+=================+==========+==========+============================================+
|             0   |    0     |    -     |                                            |
+-----------------+----------+----------+--------------------------------------------+
|             1   |    1     |    0     |  GPU optical simulation only               |
+-----------------+----------+----------+--------------------------------------------+
|             2   |    1     |    1     |  CPU optical simulation only               |
+-----------------+----------+----------+--------------------------------------------+
|             3   |    2     |   0,1    |  both GPU and CPU optical simulations      |
+-----------------+----------+----------+--------------------------------------------+

**/


void SEvt::CreateOrReuse()
{
    int integrationMode = SEventConfig::IntegrationMode() ; 
    LOG(LEVEL) << " integrationMode " << integrationMode  ; 

    if( integrationMode == 0 )
    {
        CreateOrReuse(ECPU);          // HMM: is this needed, for metadata recording ? 
    }
    else if( integrationMode == 1 )   // GPU optical simulation only
    {
        CreateOrReuse(EGPU); 
    }
    else if( integrationMode == 2 )   // CPU optical simulation only 
    {
        CreateOrReuse(ECPU); 
    }
    else if( integrationMode == 3 )  // both CPU and GPU optical simulation
    {
        CreateOrReuse(EGPU); 
        CreateOrReuse(ECPU); 
    }
    else
    {
        LOG(fatal) << "unexpected integrationMode " << integrationMode ; 
        //assert(0); 
    }
    LOG(LEVEL) << DescINSTANCE()  ; 
}



void SEvt::SetFrame(const sframe& fr )
{
    if(Exists(0)) Get(0)->setFrame(fr); 
    if(Exists(1)) Get(1)->setFrame(fr); 
}


/**
SEvt::HighLevelCreate
----------------------

Create with bells and whistles needed by some tests such as u4/tests/U4SimulateTest
which is now invoked from U4Recorder instanciation.

1. photon rerun config by persisting G4 random states
2. setting of reldir 

HMM: perhaps reldir should be static, above the individual SEvt instance level ? 

**/

SEvt* SEvt::HighLevelCreate(int idx) // static
{
    SEvt* evt = nullptr ; 

    int g4state_rerun_id = SEventConfig::G4StateRerun(); 
    bool rerun = g4state_rerun_id > -1 ;

    const char* alldir = ssys::replace_envvar_token("ALL${VERSION}") ; 
    const char* alldir0 = "ALL0" ; 
    const char* seldir = ssys::replace_envvar_token("SEL${VERSION}") ; 

    LOG(info) 
        << " g4state_rerun_id " << g4state_rerun_id 
        << " alldir " << alldir 
        << " alldir0 " << alldir0 
        << " seldir " << seldir 
        << " rerun " << rerun
        ;   

    // this runs early, at U4Recorder instanciation, which is before logging is setup it seems 
    std::cout
        << "SEvt::HighLevelCreate"
        << " g4state_rerun_id " << g4state_rerun_id 
        << " alldir " << alldir 
        << " alldir0 " << alldir0 
        << " seldir " << seldir 
        << " rerun " << rerun
        << std::endl 
        ;   


    if(rerun == false)
    {   
        SEvt::SetReldir(alldir); 
        evt = SEvt::Create(idx);    
    }   
    else
    {   
        SEvt::SetReldir(seldir);   // HUH: LoadRelative calls this static anyhow ?
        evt = SEvt::LoadRelative(alldir0) ;
        evt->clear_except("g4state");  // clear loaded evt but keep g4state 
        // when rerunning have to load states from alldir0 and then change reldir to save into seldir
    }
    // HMM: note how reldir at object rather then static level is a bit problematic for loading 

    return evt ; 
}

void SEvt::Check(int idx)
{
    SEvt* sev = Get(idx) ; 
    LOG_IF(fatal, !sev) << "must instanciate SEvt before using most SEvt methods" ; 
    assert(sev); 
}


/**
SEvt::AddTag
----------------

Tags are used when recording all randoms consumed by simulation  

NB some statics make sense to broadcast to all INSTANCES but
this is not one of them 

**/

void SEvt::AddTag(int idx, unsigned stack, float u )
{ 
    if(Exists(idx)) Get(idx)->addTag(stack,u);  
} 
int  SEvt::GetTagSlot(int idx)
{ 
    return Exists(idx) ? Get(idx)->getTagSlot() : -1 ; 
}

sgs SEvt::AddGenstep(const quad6& q)
{ 
    sgs label = {} ; 
    if(Exists(0)) label = Get(0)->addGenstep(q) ;  
    if(Exists(1)) label = Get(1)->addGenstep(q) ;  
    return label ; 
}
sgs SEvt::AddGenstep(const NP* a)
{ 
    sgs label = {} ; 
    if(Exists(0)) label = Get(0)->addGenstep(a) ;  
    if(Exists(1)) label = Get(1)->addGenstep(a) ;  
    return label ; 
}
void SEvt::AddCarrierGenstep(){ AddGenstep(SEvent::MakeCarrierGensteps()); }
void SEvt::AddTorchGenstep(){   AddGenstep(SEvent::MakeTorchGensteps());   }



SEvt* SEvt::LoadAbsolute(const char* dir_) // static
{
    const char* dir = spath::Resolve(dir_); 
    SEvt* evt = new SEvt ; 
    int rc = evt->loadfold(dir) ; 
    if(rc != 0) evt->is_loadfail = true ; 
    return evt  ; 
}


/**
SEvt::LoadRelative (formerly Load)
------------------------------------

Q: which instance slot ? 
A: are persisting instance in domain metadata and recovering in SEvt::onload

Q: should the INSTANCE array slot be filled onload ? 
A: Guess so, Loading should regain the state before the save


**/

SEvt* SEvt::LoadRelative(const char* rel)  // static 
{
    LOG(LEVEL) << "[" ; 

    if(rel != nullptr) SetReldir(rel); 

    SEvt* evt = new SEvt ; 
    int rc = evt->load() ; 
    if(rc != 0) evt->is_loadfail = true ; 

    LOG(LEVEL) << "]" ; 
    return evt ; 
}


/**
SEvt::Clear
-------------

SEvt::Clear is invoked in two situations:

1. from QEvent::setGenstep after SEvt::GatherGenstep to prepare 
   for further genstep collection, this is done immediately prior to 
   simulation launches

2. from SEvt::EndOfEvent immediately after SEvt::Save

**/

void SEvt::Clear()
{
    if(Exists(0)) Get(0)->clear();  
    if(Exists(1)) Get(1)->clear();  
}
void SEvt::Save()
{ 
    if(Exists(0)) Get(0)->save();  
    if(Exists(1)) Get(1)->save();  
}

/*
void SEvt::SaveExtra( const char* name, const NP* a)
{  
    if(Exists(0)) Get(0)->saveExtra(name, a); 
    if(Exists(1)) Get(1)->saveExtra(name, a); 
}
*/


bool SEvt::HaveDistinctOutputDirs() // static 
{
    if(Count() < 2) return true ;  
    assert( Count() == 2 ); 
    SEvt* i0 = Get(0); 
    SEvt* i1 = Get(1); 
    return i0->index != i1->index ; 
}


void SEvt::Save(const char* dir)
{  
    assert( HaveDistinctOutputDirs() );  // check will not overwrite 
    if(Exists(0)) Get(0)->save(dir) ; 
    if(Exists(1)) Get(1)->save(dir) ; 
}

void SEvt::Save(const char* dir, const char* rel)
{  
    assert( HaveDistinctOutputDirs() );  // check will not overwrite 
    if(Exists(0)) Get(0)->save(dir, rel) ; 
    if(Exists(1)) Get(1)->save(dir, rel) ; 
}

void SEvt::SaveGenstepLabels(const char* dir, const char* name)
{ 
    assert( HaveDistinctOutputDirs() );  // check will not overwrite 
    if(Exists(0)) Get(0)->saveGenstepLabels(dir, name ); 
    if(Exists(1)) Get(1)->saveGenstepLabels(dir, name ); 
}




uint64_t SEvt::T_BeginOfRun = 0 ; 
uint64_t SEvt::T_EndOfRun = 0 ; 
void SEvt::BeginOfRun(){ T_BeginOfRun = stamp::Now(); } // static 
void SEvt::EndOfRun()  { T_EndOfRun   = stamp::Now(); } // static 


template<typename T>
void SEvt::SetRunMeta(const char* k, T v )
{
    RUN_META->set_meta<T>(k, v ); 
}

template void SEvt::SetRunMeta<int>(      const char*, int  );
template void SEvt::SetRunMeta<uint64_t>( const char*, uint64_t  );
template void SEvt::SetRunMeta<unsigned>( const char*, unsigned  );
template void SEvt::SetRunMeta<float>(    const char*, float  );
template void SEvt::SetRunMeta<double>(   const char*, double  );


/**
SEvt::SaveRunMeta
-------------------

May be called for example from U4Recorder::EndOfRunAction

**/

void SEvt::SaveRunMeta(const char* base)
{
    SetRunMeta<uint64_t>("T_BeginOfRun", T_BeginOfRun ); 
    SetRunMeta<uint64_t>("T_EndOfRun",   T_EndOfRun );  
    const char* dir = RunDir(base); 
    RUN_META->save(dir, "run.npy") ; 
}






/**
SEvt::beginOfEvent  (former static SEvt::BeginOfEvent is removed)
-------------------------------------------------------------------

Called for example from U4Recorder::BeginOfEventAction
Note that eventID from Geant4 is zero based but the 
index used for SEvt::SetIndex is 1-based to allow (+ve,-ve) pairs. 

TODO: avoid that complication by just basing 
the output dir index prefix "p" or "n" depending on SEvt::instance 
which is either 0 or 1 (SEvt::EGPU or SEvt::ECPU)

**/

void SEvt::beginOfEvent(int eventID)
{
    int index_ = 1+eventID ;  
    LOG(LEVEL) << " index_ " << index_ ; 
    setIndex(index_); 
    addFrameGenstep();     // needed for simtrace and input photon running
}


/**
SEvt::endOfEvent (former static SEvt::EndOfEvent is removed)
--------------------------------------------------------------

Called for example from U4Recorder::EndOfEventAction

Only SEventConfig::SaveComp OPTICKS_SAVE_COMP are actually saved, 
so can switch off all saving wuth that config. 

**/

void SEvt::endOfEvent(int eventID)
{
    int index_ = 1+eventID ;    
    endIndex(index_); 
    save();             
    clear_except("hit"); 
}


bool SEvt::IndexPermitted(int index_) // static
{
    return index_ >= 1 ;  // no longer allowing -ve index 
    //return std::abs(index_) >= 1 ;   
    // for simplicity dont depend on Count, always dis-allow index 0 
}

/*
void SEvt::SetIndex(int index_)
{ 
    if(Exists(0)) Get(0)->setIndex(index_); 
    if(Exists(1)) Get(1)->setIndex(-index_); 
}

void SEvt::EndIndex(int index)
{
    if(Exists(0)) Get(0)->endIndex(index); 
    if(Exists(1)) Get(1)->endIndex(-index); 
}

void SEvt::IncrementIndex()
{   
    if(Exists(0)) Get(0)->incrementIndex() ; 
    if(Exists(1)) Get(1)->incrementIndex() ; 
}
void SEvt::UnsetIndex()
{  
    if(Exists(0)) Get(0)->unsetIndex() ; 
    if(Exists(1)) Get(1)->unsetIndex() ; 
}
*/




int SEvt::GetIndex(int idx)
{   
    return Exists(idx) ? Get(idx)->getIndex() : 0 ;   
}
S4RandomArray* SEvt::GetRandomArray(int idx)
{ 
    return Exists(idx) ? Get(idx)->random_array : nullptr ; 
}


// SetReldir can be used with the default SEvt::save() changing the last directory element before the index if present

const char* SEvt::DEFAULT_RELDIR = "ALL" ;   
const char* SEvt::RELDIR = nullptr ; 
void        SEvt::SetReldir(const char* reldir_){ RELDIR = reldir_ ? strdup(reldir_) : nullptr ; }
const char* SEvt::GetReldir(){ return RELDIR ? RELDIR : DEFAULT_RELDIR ; }


int SEvt::GetNumPhotonCollected(int idx){    return Exists(idx) ? Get(idx)->getNumPhotonCollected() : UNDEF ; }
int SEvt::GetNumPhotonGenstepMax(int idx){   return Exists(idx) ? Get(idx)->getNumPhotonGenstepMax() : UNDEF ; }
int SEvt::GetNumPhotonFromGenstep(int idx){  return Exists(idx) ? Get(idx)->getNumPhotonFromGenstep() : UNDEF ; }
int SEvt::GetNumGenstepFromGenstep(int idx){ return Exists(idx) ? Get(idx)->getNumGenstepFromGenstep() : UNDEF ; }
int SEvt::GetNumHit(int idx){                return Exists(idx) ? Get(idx)->getNumHit() : UNDEF ; }
int SEvt::GetNumHit_EGPU(){  return GetNumHit(EGPU) ; }
int SEvt::GetNumHit_ECPU(){  return GetNumHit(ECPU) ; }




NP* SEvt::GatherGenstep(int idx) {   return Exists(idx) ? Get(idx)->gatherGenstep() : nullptr ; }
NP* SEvt::GetInputPhoton(int idx) {  return Exists(idx) ? Get(idx)->getInputPhoton() : nullptr ; }

void SEvt::SetInputPhoton(NP* p) 
{  
    if(Exists(0)) Get(0)->setInputPhoton(p) ;
    if(Exists(1)) Get(1)->setInputPhoton(p) ;
}
bool SEvt::HasInputPhoton(int idx)
{  
    return Exists(idx) ? Get(idx)->hasInputPhoton() : false ; 
}
bool SEvt::HasInputPhoton()
{
    return HasInputPhoton(EGPU) || HasInputPhoton(ECPU) ; 
}

NP* SEvt::GetInputPhoton() // static 
{
    NP* ip = nullptr ; 
    if(ip == nullptr && HasInputPhoton(EGPU)) ip = GetInputPhoton(EGPU) ; 
    if(ip == nullptr && HasInputPhoton(ECPU)) ip = GetInputPhoton(ECPU) ; 
    return ip ; 
}


std::string SEvt::DescHasInputPhoton()  // static
{
    std::stringstream ss ; 
    ss 
       <<  "SEvt::DescHasInputPhoton()  " 
       << " SEventConfig::IntegrationMode " << SEventConfig::IntegrationMode()
       << " SEvt::HasInputPhoton(EGPU) " << HasInputPhoton(EGPU) 
       << " SEvt::HasInputPhoton(ECPU) " << HasInputPhoton(ECPU) 
       << std::endl 
       << "SEvt::Brief" 
       << std::endl 
       << SEvt::Brief() 
       << std::endl 
       << "SEvt::DescInputPhoton(EGPU)"
       << SEvt::DescInputPhoton(EGPU)
       << "SEvt::DescInputPhoton(ECPU)"
       << SEvt::DescInputPhoton(ECPU)
       << std::endl 
       ;
    std::string str = ss.str(); 
    return str ; 
} 




/**
SEvt::clear_vectors
--------------------

Set the photon counts to zero and clear the vectors. 
Note that most of the vectors are only used with hostside running.

**/

void SEvt::clear_vectors()
{
    numgenstep_collected = 0u ; 
    numphoton_collected = 0u ; 
    numphoton_genstep_max = 0u ; 

    clear_count += 1 ; 

    if(DEBUG_CLEAR > 0)
    {
        LOG(info) 
           << " DEBUG_CLEAR " << DEBUG_CLEAR
           << " clear_count " << clear_count 
           ; 
        std::raise(SIGINT);  
    }


    setNumPhoton(0); 

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
    aux.clear(); 
    sup.clear(); 

    gather_done = false ;  
    g4state = nullptr ;   // avoiding stale (g4state is special, as only used for 1st event) 
}

/**
SEvt::clear
-------------

Clear vectors and the fold.

Note this is called by QEvent::setGenstep 
so when adding input arrays make sure to do so 
after the genstep setup. 

**/

void SEvt::clear()
{
    LOG(info) << "SEvt::clear" ; 

    LOG(LEVEL) << "[" ;
 
    clear_vectors(); 
    if(fold) fold->clear(); 

    LOG(LEVEL) << "]" ; 
}


/**
SEvt::clear_except
--------------------

The comma delimited keeplist need not contain .npy on its keys

**/

void SEvt::clear_except(const char* keep)
{
    LOG(info) << "SEvt::clear_except" ; 

    LOG(LEVEL) << "[" ; 
    clear_vectors(); 

    bool copy = false ; 
    char delim = ',' ; 
    if(fold) fold->clear_except(keep, copy, delim); 

    LOG(LEVEL) << "]" ; 
}


void SEvt::setIndex(int index_)
{ 
    bool index_permitted = IndexPermitted(index_); 
    LOG_IF(fatal, !index_permitted) << " index_ " << index_ << " count " << Count() ; 
    assert( index_permitted ); 

    index = index_ ; 
    t_BeginOfEvent = stamp::Now();  // moved here from the static 
}
void SEvt::endIndex(int index_)
{ 
    assert( std::abs(index) == std::abs(index_) );  
    t_EndOfEvent = stamp::Now();  
}

int SEvt::getIndex() const 
{ 
    return index ; 
}
void SEvt::incrementIndex()
{
    int base_index = index == MISSING_INDEX ? 0 : index ; 
    int new_index = base_index < 0 ? base_index - 1 : base_index + 1 ; 
    setIndex(new_index); 
}
void SEvt::unsetIndex()
{  
    index = MISSING_INDEX ; 
}



void SEvt::setInstance(int instance_)
{
    instance = instance_ ;  
}
int SEvt::getInstance() const 
{
    return instance ; 
}




/**
SEvt::getNumGenstepFromGenstep
--------------------------------

Size of the genstep vector ...

caution this vector is cleared by QEvent::setGenstep 
after which must get the count from the genstep array 

**/

unsigned SEvt::getNumGenstepFromGenstep() const 
{
    assert( genstep.size() == gs.size() ); 
    return genstep.size() ; 
}

/**
SEvt::getNumPhotonFromGenstep
----------------------------------

Total photons by summation over all collected genstep. 
When collecting very large numbers of gensteps the alternative 
SEvt::getNumPhotonCollected is faster. 

**/

unsigned SEvt::getNumPhotonFromGenstep() const 
{
    unsigned tot = 0 ; 
    for(unsigned i=0 ; i < genstep.size() ; i++) tot += genstep[i].numphoton() ; 
    return tot ; 
}

unsigned SEvt::getNumGenstepCollected() const 
{
    return numgenstep_collected ;  // updated by addGenstep
}
unsigned SEvt::getNumPhotonCollected() const 
{
    return numphoton_collected ;   // updated by addGenstep 
}
unsigned SEvt::getNumPhotonGenstepMax() const 
{
    return numphoton_genstep_max ; 
}



/**
SEvt::addGenstep
------------------

The sgs summary struct of the last genstep added is returned. 

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
    LOG(LEVEL) << " index " << index << " instance " << instance ; 

    unsigned gentype = q_.gentype(); 
    unsigned matline_ = q_.matline(); 


    bool input_photon_with_normal_genstep = input_photon && OpticksGenstep_::IsInputPhoton(gentype) == false  ; 
    LOG_IF(fatal, input_photon_with_normal_genstep)
        << "input_photon_with_normal_genstep " << input_photon_with_normal_genstep
        << " MIXING input photons with other gensteps is not allowed "
        << " for example avoid defining OPTICKS_INPUT_PHOTON when doing simtrace"
        ; 
    assert( input_photon_with_normal_genstep == false ); 


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


#ifdef SEVT_NUMPHOTON_FROM_GENSTEP_CHECK
    unsigned numphoton_from_genstep = getNumPhotonFromGenstep() ; // sum numphotons from all previously collected gensteps (since last clear)
    assert( numphoton_from_genstep == numphoton_collected );   
    // DONE: Zike reports this assert succeeds, so have removed the slower getNumPhotonFromGenstep
#endif

    unsigned q_numphoton = q.numphoton() ;          // numphoton in this genstep 
    if(q_numphoton > numphoton_genstep_max) numphoton_genstep_max = q_numphoton ; 


    sgs s = {} ;                      // genstep summary struct 

    s.index = genstep.size() ;        // 0-based genstep index since last clear  
    s.photons = q_numphoton ;         // numphoton in this genstep 
    s.offset = numphoton_collected ;  // sum numphotons from all previously collected gensteps (since last clear)
    s.gentype = q.gentype() ; 

    gs.push_back(s) ;      // summary labels 
    genstep.push_back(q) ; // actual genstep params

    numgenstep_collected += 1 ; 
    numphoton_collected += q_numphoton ;  // keep running total for all gensteps collected, since last clear


    int tot_photon = s.offset+s.photons ; 

    LOG_IF(debug, enabled) << " s.desc " << s.desc() << " gidx " << gidx << " enabled " << enabled << " tot_photon " << tot_photon ; 

    bool num_photon_changed = tot_photon != evt->num_photon ; 

    LOG(debug) 
        << " tot_photon " << tot_photon
        << " evt.num_photon " << evt->num_photon
        << " num_photon_changed " << num_photon_changed
        << " gs.size " << gs.size() 
        << " genstep.size " << genstep.size()
        << " numgenstep_collected " << numgenstep_collected
        << " numphoton_collected " << numphoton_collected
        << " tot_photon " << tot_photon
        << " s.index " << s.index
        << " s.photons " << s.photons
        << " s.offset " << s.offset
        << " s.gentype " << s.gentype
        << " s.desc " << s.desc()
        ;

    setNumPhoton(tot_photon); // still call when no change for reset hostside_running_resize_done:false
    return s ; 
}



/**
SEvt::setNumPhoton
----------------------

This is called from SEvt::addGenstep, updating evt.num_photon 
according to the additional genstep collected and evt.num_seq/tag/flat/record/rec/prd
depending on the configured max which when zero will keep the counts zero.  

Also called by QEvent::setNumPhoton prior to device side allocations.

*hostside_running_resize_done:false*
    signals the following SEvt::beginPhoton to call SEvt::hostside_running_resize, 
    so allocation for hostside running happens on reaching the first photon track

Note that SEvt::beginPhoton is used for hostside running only (eg U4Recorder/U4SimulateTest)  
so as gensteps are added with SEvt::addGenstep from the U4 scintillation 
and Cerenkov collection SEvt::setNumPhoton increments the totals in sevent.h:evt 
and sets hostside_running_resize_done:false such that at the next SEvt::beginPhoton
which happens from the BeginOfTrackAction the std::vector grow as
needed to accommodate the photons from the last genstep collected.   

**/

void SEvt::setNumPhoton(unsigned num_photon)
{
    bool num_photon_allowed = int(num_photon) <= evt->max_photon ; 
    LOG_IF(fatal, !num_photon_allowed) << " num_photon " << num_photon << " evt.max_photon " << evt->max_photon ;
    assert( num_photon_allowed );

    evt->num_photon = num_photon ; 
    evt->num_seq    = evt->max_seq   > 0 ? evt->num_photon : 0 ;
    evt->num_tag    = evt->max_tag  == 1 ? evt->num_photon : 0 ;
    evt->num_flat   = evt->max_flat == 1 ? evt->num_photon : 0 ;
    evt->num_sup    = evt->max_sup   > 0 ? evt->num_photon : 0 ;

    evt->num_record = evt->max_record * evt->num_photon ;
    evt->num_rec    = evt->max_rec    * evt->num_photon ;
    evt->num_aux    = evt->max_aux    * evt->num_photon ;
    evt->num_prd    = evt->max_prd    * evt->num_photon ;

    LOG(LEVEL)
        << " evt->num_photon " << evt->num_photon
        << " evt->num_tag " << evt->num_tag
        << " evt->num_flat " << evt->num_flat
        ;

    hostside_running_resize_done = false ;    
}

/**
SEvt::setNumSimtrace
---------------------

**/

void SEvt::setNumSimtrace(unsigned num_simtrace)
{
    bool num_simtrace_allowed = int(num_simtrace) <= evt->max_simtrace ;
    LOG_IF(fatal, !num_simtrace_allowed) << " num_simtrace " << num_simtrace << " evt.max_simtrace " << evt->max_simtrace ;
    assert( num_simtrace_allowed );
    LOG(LEVEL) << " num_simtrace " << num_simtrace ;  

    evt->num_simtrace = num_simtrace ;
}


/**
SEvt::hostside_running_resize
-------------------------------

Canonically called from SEvt::beginPhoton  (also SEvt::setFrame_HostsideSimtrace)

**/

void SEvt::hostside_running_resize()
{
    bool is_self_provider = isSelfProvider() ; 
    LOG_IF(fatal, is_self_provider == false ) << " NOT-is_self_provider " << descProvider() ;   
    LOG(LEVEL) 
        << " is_self_provider " << is_self_provider 
        << " hostside_running_resize_done " << hostside_running_resize_done
        ;

    assert( hostside_running_resize_done == false ); 
    assert( is_self_provider ); 

    hostside_running_resize_done = true ; 
    hostside_running_resize_(); 

    LOG(LEVEL) 
        << " is_self_provider " << is_self_provider 
        << std::endl 
        << evt->desc() 
        ; 

}

/**
SEvt::hostside_running_resize_
--------------------------------

Resize the hostside std::vectors according to the sizes from sevent.h:evt
and update evt array pointers to follow around the std::vectors as they get reallocated.

This makes the hostside sevent.h:evt environment mimic the deviceside environment 
even though deviceside uses device buffers and hostside uses std::vectors. 

Notice how the same sevent.h struct that holds deviceside pointers 
is being using to hold the hostside vector data pointers. 


+-----------+-----------------+
| vector    |   evt->         |
+===========+=================+
| pho       |   num_photon    |
+-----------+-----------------+
| slot      |   num_photon    |
+-----------+-----------------+
| photon    |   num_photon    |
+-----------+-----------------+
| record    |   num_record    |
+-----------+-----------------+
| rec       |   num_rec       |
+-----------+-----------------+
| aux       |   num_aux       |
+-----------+-----------------+
| seq       |   num_seq       |
+-----------+-----------------+
| prd       |   num_prd       |
+-----------+-----------------+
| tag       |   num_tag       |
+-----------+-----------------+
| flat      |   num_flat      |
+-----------+-----------------+
| simtrace  |   num_simtrace  |
+-----------+-----------------+

Vectors are disabled by some of the above *num* 
being configured to zero via SEventConfig. 

**/

void SEvt::hostside_running_resize_()
{
    LOG(info) << "resizing photon " << photon.size() << " to evt.num_photon " << evt->num_photon  ; 

    if(evt->num_photon > 0) 
    { 
         pho.resize(  evt->num_photon );  
         // no device side equivalent array 
    }
    if(evt->num_photon > 0) 
    {
        slot.resize( evt->num_photon ); 
         // no device side equivalent array 
    }
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
    if(evt->num_aux > 0) 
    {
        aux.resize(evt->num_aux); 
        evt->aux = aux.data() ; 
    }
    if(evt->num_sup > 0) 
    {
        sup.resize(evt->num_sup); 
        evt->sup = sup.data() ; 
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

/**
SEvt::get_genflag
-------------------

This is called for example from SEvt::beginPhoton
to become the "TO" "SI" "CK" at the start of photon histories. 

1. lookup sgs genstep label corresponding to the spho photon label 
2. convert the sgs gentype into the corresponding photon genflag which must be 
   one of CERENKOV, SCINTILLATION or TORCH

   * What about input photons ? Presumably they are TORCH ? 
   
**/

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

Canonically invoked from tail of U4Recorder::PreUserTrackingAction_Optical

0. for hostside_running_resize_done:false calls hostside_running_resize which resizes vectors 
   and updates evt pointers : via the toggle this only gets done when reaching first photon of the event 
1. determine genflag SI/CK/TO, via lookups in the sgs corresponding to the spho label
2. zeros current_ctx and slot[label.id] (the "recording head" )
3. start filling current_ctx.p sphoton with set_idx and set_flag  

**/
void SEvt::beginPhoton(const spho& label)
{
    if(!hostside_running_resize_done) hostside_running_resize(); 

    dbg->beginPhoton++ ; 
    LOG(LEVEL) ; 
    LOG(LEVEL) << label.desc() ; 

    unsigned idx = label.id ; 

    bool in_range = idx < pho.size() ; 
    LOG_IF(error, !in_range) 
        << " not in_range " 
        << " idx " << idx 
        << " pho.size  " << pho.size() 
        << " label " << label.desc() 
        ;  

    if(!in_range) std::cerr
        << "SEvt::beginPhoton FATAL not in_range" 
        << " idx " << idx 
        << " pho.size  " << pho.size() 
        << " label " << label.desc() 
        << std::endl 
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

#ifdef WITH_SUP
    quadx6& xsup = (quadx6&)ctx.sup ;  
    xsup.q0.w.x = stamp::Now(); 
#endif

    ctx.idx = idx ;  
    ctx.evt = evt ; 
    ctx.prd = &current_prd ;   // current_prd is populated by InstrumentedG4OpBoundaryProcess::PostStepDoIt 

    ctx.p.set_idx(idx); 
    ctx.p.set_flag(genflag); 

    bool flagmask_one_bit = ctx.p.flagmask_count() == 1  ;
    LOG_IF(error, !flagmask_one_bit ) 
        << " not flagmask_one_bit "
        << " : should only be a single bit in the flagmask at this juncture "
        << " label " << label.desc() 
         ;

    assert( flagmask_one_bit ); 
}

unsigned SEvt::getCurrentPhotonIdx() const { return current_pho.id ; }


/**
SEvt::resumePhoton
---------------------

Canonically called from tail of U4Recorder::PreUserTrackingAction_Optical
following a FastSim->SlowSim transition. 

Transitions between Geant4 "SlowSim" and FastSim result in fSuspend track status
and the history of a single photon being split across multiple calls to 
U4RecorderTest::PreUserTrackingAction (U4Recorder::PreUserTrackingAction_Optical)
   
Need to join up the histories, a bit like rjoin does for reemission. 
But with FastSim/SlowSim the G4Track are the same pointers, unlike
with reemission where the G4Track pointers differ.  

BUT reemission photons will also go thru FastSim/SlowSim transitions, 
so need separate approach than the label.gn (photon generation) handling of reemission ?

**/

void SEvt::resumePhoton(const spho& label)
{
    dbg->resumePhoton++ ; 
    LOG(LEVEL); 
    LOG(LEVEL) << label.desc() ; 

    unsigned idx = label.id ; 
    assert( idx < pho.size() );  
}


/**
SEvt::rjoin_resumePhoton
-------------------------

Invoked from U4Recorder::PreUserTrackingAction_Optical for 
reemission photons that subsequently transition from FastSim 
back to SlowSim. 

BUT as the FastSim/SlowSim transition happens inside PMT 
will never need to do real rjoin history rewriting  ?

**/

void SEvt::rjoin_resumePhoton(const spho& label)
{
    dbg->rjoin_resumePhoton++ ; 
    LOG(LEVEL); 
    LOG(LEVEL) << label.desc() ; 

    unsigned idx = label.id ; 
    assert( idx < pho.size() );  
}


/**
SEvt::rjoinPhoton : only used for hostside running
---------------------------------------------------

Called from tail of U4Recorder::PreUserTrackingAction_Optical for G4Track with 
spho label indicating a reemission generation greater than zero.

HUH: BUT THAT MEANS THIS WILL BE CALLED FOR EVERY SUBSEQUENT 
STEP OF THE REEMISSION PHOTON NOT JUST THE FIRST THAT NEEDS REJOINING ?

* SUSPECT THAT WILL GET REPEATED "RE RE RE RE" ? 
* MAYBE NEED ANOTHER MARK TO SAY THE REJOIN WAS DONE AND DOENST NEED RE-rjoin ?

Note that this will mostly be called for photons that originate from 
scintillation gensteps BUT it will also happen for Cerenkov (and Torch) genstep 
generated photons within a scintillator due to reemission. 

HMM: could directly change photon[idx] via ref ? 
But are here taking a copy to current_photon
and relying on copyback at SEvt::finalPhoton

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
    assert( label.gen() == parent_label.gen() + 1 ); 

    // every G4Track has its own label but for reemission the lineage 
    // through the generations is reflected by multiple such track labels 
    // all having the same lineage (label.gs,label.ix,label.id) but different 
    // reemission generations


    const sgs& _gs = get_gs(label);  
    bool expected_gentype = OpticksGenstep_::IsExpected(_gs.gentype);  // SI/CK/TO 
    assert(expected_gentype);  
    // NB: within scintillator, photons of any gentype may undergo reemission  

    const sphoton& parent_photon = photon[idx] ; 
    unsigned parent_idx = parent_photon.idx() ; 
    assert( parent_idx == idx ); 


    // replace pho[idx] and current_pho with the new generation label 
    pho[idx] = label ;   
    current_pho = label ; 

    const int& bounce = slot[idx] ; assert( bounce > 0 );   
    int prior = bounce - 1 ; 

    // RE-WRITE HISTORY : CHANGING BULK_ABSORB INTO BULK_REEMIT

    if( evt->photon )
    {
        current_ctx.p = photon[idx] ; 
        sphoton& current_photon = current_ctx.p ; 

        rjoinPhotonCheck(current_photon); 
        current_photon.flagmask &= ~BULK_ABSORB  ; // scrub BULK_ABSORB from flagmask
        current_photon.set_flag(BULK_REEMIT) ;     // gets OR-ed into flagmask 
    }

    // at truncation point and beyond cannot compare or do rejoin fixup
    if( evt->seq && prior < evt->max_seq )
    {
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

    // NOT: rec  (compressed record) are not handled
    //      but no longer using that as decided full recording 
    //      is a debugging only activity so no point in going 
    //      to great lengths squeezing memory usage for something that 
    //      is just used while debugging
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
    LOG_IF(fatal, !expect)
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

    LOG_IF(fatal, !flag_xor) << " flag_xor FAIL " << OpticksPhoton::Abbrev(seq_flag) << std::endl ;  
    assert( flag_xor ); 
}

/**
SEvt::pointPhoton : only used for hostside running
---------------------------------------------------

Invoked from U4Recorder::UserSteppingAction_Optical to cause the 
current photon to be recorded into record vector. 

The pointPhoton and finalPhoton methods need to do the hostside equivalent of 
what CSGOptiX/CSGOptiX7.cu:simulate does device side, so have setup the environment to match:

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

#ifndef PRODUCTION
    t_PenultimatePoint = t_LastPoint ; 
    t_LastPoint = stamp::Now() ;  
    quad4& aux = current_ctx.aux ;
    quadx4& auxx = (quadx4&)aux ;  
    auxx.q3.w.x = t_LastPoint ;  // shoe-horn uint64_t time stamp into aux 
#endif
    assert( ctx.idx == idx ); 
    int& bounce = slot[idx] ; 

    bool first_point = bounce == 0 ; 
    bool first_flag = ctx.p.flagmask_count() == 1 ; 
    bool fake_first = first_flag == true && first_point == false ;   
    LOG_IF(LEVEL, fake_first) 
        << " fake_first detected, bounce: " << bounce 
        << " EARLY EXIT pointPhoton" 
        << " this happens when the POST of the first step is fake "
        << " resulting in GENFLAG being repeated as 2nd step first_flag still true "
        ;   
    if(fake_first) return ; 


    if(first_point == false) ctx.trace(bounce); 
    ctx.point(bounce); 


    sseq& seq = ctx.seq ; 
    LOG(LEVEL) 
        << "(" << std::setw(5) << label.id
        << "," << std::setw(2) << bounce
        << ") "
        << std::setw(2) << OpticksPhoton::Abbrev(ctx.p.flag()) 
        << seq.desc_seqhis()
        ;   


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

    bounce += 1 ;   // increments slot array, by reference
}

/**
SEvt::addTag
-------------

HMM: this needs to be called following every random consumption ... see U4Random::flat 

**/

bool SEvt::PIDX_ENABLED = ssys::getenvbool("SEvt__PIDX_ENABLED") ; 

void SEvt::addTag(unsigned tag, float flat)
{
    if(evt->tag == nullptr) return  ; 
    stagr& tagr = current_ctx.tagr ; 
    unsigned idx = current_ctx.idx ; 

    LOG_IF(info, PIDX_ENABLED && int(idx) == PIDX ) 
        << " idx " << idx
        << " PIDX " << PIDX
        << " tag " << tag 
        << " flat " << flat 
        << " evt.tag " << evt->tag 
        << " tagr.slot " << tagr.slot
        ; 

    tagr.add(tag,flat)  ; 


    if(random)
    {
        int flat_cursor = random->getFlatCursor(); 
        assert( flat_cursor > -1 ); 
        double flat_prior = random->getFlatPrior(); 
        bool cursor_slot_match = unsigned(flat_cursor) == tagr.slot ; 
        LOG_IF(error, !cursor_slot_match)
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

1. asserts label is same lineage as current_pho
2. calls sctx::end on ctx, copying ctx.seq into evt->seq[idx]
3. copies ctx.p into evt->photon[idx]  

**/

void SEvt::finalPhoton(const spho& label)
{
    dbg->finalPhoton++ ; 
    LOG(LEVEL) << label.desc() ; 
    assert( label.isSameLineage(current_pho) ); 

    unsigned idx = label.id ; 
    sctx& ctx = current_ctx ; 
    assert( ctx.idx == idx ); 

#ifdef WITH_SUP
    quadx6& xsup = (quadx6&)ctx.sup ;  
    xsup.q0.w.y = stamp::Now();   
    xsup.q1.w.x = t_PenultimatePoint ; 
    xsup.q1.w.y = t_LastPoint ; 
#endif

    ctx.end();   // copies {seq,sup} into evt->{seq,sup}[idx] (and tag, flat when DEBUG_TAG)
    evt->photon[idx] = ctx.p ;   // HUH: why not do this in ctx.end ?
}



/**
SEvt::AddProcessHitsStamp
---------------------------

As ProcessHits may be called multiple
times for each photon this records the 
timestamp range of those calls and the count. 

Note that this relies on being zeroed for each photon. 

Also note that the only thing specific to "ProcessHits"
is the convention of where to store the stamp range. It 
is up to the user to call this from the right place. 

**/

void SEvt::AddProcessHitsStamp(int idx, int p) // static
{ 
    if(Exists(idx)) Get(idx)->addProcessHitsStamp(p) ; 
}
/**
SEvt::addProcessHitsStamp
---------------------------

See squadx.h and search for WITH_SUP to find where sup is populated.  Also see sevt.py for usage. 

+--------------------------------------+----------------------------------------------------------------------------------+
| sctx::sup fields                     |                                                                                  |
+==================+===================+==================================================================================+
|  q0.w.x          |  q0.w.y           | q0.w.x SEvt::beginPhoton q0.w.y SEvt::finalPhoton                                |
+------------------+-------------------+----------------------------------------------------------------------------------+
|  q1.w.x          |  q1.w.y           | q1.w.x SEvt::finalPhoton t_PenultimatePoint q1.w.y SEvt::finalPhoton t_LastPoint |
+------------------+-------------------+----------------------------------------------------------------------------------+
|  q2.w.x          |  q2.w.y           | time range from SEvt::addProcessHitsStamp(0)                                     |
+---------+--------+--------+----------+----------------------------------------------------------------------------------+
|  q3.u.x | q3.u.y | q3.u.x | q3.u.w   | .x call count from SEvt::addProcessHitsStamp(0)                                  |   
+---------+--------+--------+----------+----------------------------------------------------------------------------------+
|  q4.w.x          |  q4.w.y           | time range from SEvt::addProcessHitsStamp(1)                                     |
+---------+--------+-------------------+----------------------------------------------------------------------------------+
|  q5.u.x | q5.u.y | q5.u.z | q5.u.w   | .x call count frmo SEvt::addProcessHitsStamp(1)                                  |  
+---------+--------+--------+----------+----------------------------------------------------------------------------------+

Use BP=SEvt::addProcessHitsStamp to find where time stamps are coming. Currently none. 

**/
void SEvt::addProcessHitsStamp(int p)
{
    assert( p > -1 ); 
#ifdef WITH_SUP
    uint64_t now = stamp::Now();

    quad6&  sup  = current_ctx.sup ;  
    quadx6& xsup = (quadx6&)current_ctx.sup ;  

    uint64_t* h0 = nullptr ; 
    uint64_t* h1 = nullptr ; 
    unsigned* hc = nullptr ; 

    switch(p)
    {
       case 0: { h0 = &xsup.q2.w.x ; h1 = &xsup.q2.w.y ; hc = &sup.q3.u.x ;} ; break ; 
       case 1: { h0 = &xsup.q4.w.x ; h1 = &xsup.q4.w.y ; hc = &sup.q5.u.x ;} ; break ; 
    } 
    assert( hc && h0 && h1 ); 

    *hc += 1 ; 

    if(*h0 == 0) 
    {
        *h0 = now ; 
    }
    else if(*h1 == 0)
    {
        *h1 = now ;  
    }
    else if( *h0 > 0 && *h1 > 0 )
    {
        *h1 = now ;  
    }

    /*
    std::cout 
        << "SEvt::addProcessHitsStamp" 
        << " p " << p 
        << " now " << now 
        << " h0 " << *h0 
        << " h1 " << *h1 
        << " hc " << *hc
        << std::endl 
        ; 
    */
#endif
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


NP* SEvt::gatherPho0() const { return NPX::Make<int>( (int*)pho0.data(), int(pho0.size()), 4 ); }
NP* SEvt::gatherPho() const {  return NPX::Make<int>( (int*)pho.data(), int(pho.size()), 4 ); }
NP* SEvt::gatherGS() const {   return NPX::Make<int>( (int*)gs.data(),  int(gs.size()), 4 );  }

NP* SEvt::gatherGenstep() const { return NPX::Make<float>( (float*)genstep.data(), int(genstep.size()), 6, 4 ) ; }



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
NP* SEvt::gatherAux() const 
{ 
    if( evt->aux == nullptr ) return nullptr ; 
    NP* r = makeAux(); 
    r->read2( (float*)evt->aux ); 
    return r ; 
} 
NP* SEvt::gatherSup() const 
{ 
    if( evt->sup == nullptr ) return nullptr ; 
    NP* p = makeSup(); 
    p->read2( (float*)evt->sup ); 
    return p ; 
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
NP* SEvt::gatherSeed() const   // COULD BE IMPLEMENTED : IF NEEDED TO DEBUG  SLOT->GS ASSOCIATION 
{ 
    LOG(fatal) << " not implemented for hostside running : getting this error indicates CompMask mixup " ; 
    assert(0); 
    return nullptr ; 
}

/**
SEvt::gatherHit
-------------------------------------------

Does CPU side equivalent of QEvent::gatherHit_ 
using the photon array and the sphoton_selector 

**/

NP* SEvt::gatherHit() const 
{ 
    const NP* p = getPhoton(); 
    NP* h = p ? p->copy_if<float, sphoton>(*selector) : nullptr ;  
    return h ; 
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
    NP* p = NP::Make<float>( evt->num_photon, 4, 4 ); 

    p->set_meta<uint64_t>("t_BeginOfEvent", t_BeginOfEvent ); 
    p->set_meta<uint64_t>("t_EndOfEvent",   t_EndOfEvent ); 
    p->set_meta<uint64_t>("T_BeginOfRun",   T_BeginOfRun ); 
    // T_EndOfRun cannot be saved here, it might be saved if SEvt::SaveRunMeta is called

    // AddEnvMeta(p, false) ; 
    // smeta::Collect(p->meta, "SEvt::makePhoton"); 
    // decided env meta is better placed on the fold, not the photon

    return p ; 
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
NP* SEvt::makeAux() const 
{ 
    NP* r = NP::Make<float>( evt->num_photon, evt->max_aux, 4, 4 ); 
    return r ; 
}
NP* SEvt::makeSup() const 
{ 
    NP* p = NP::Make<float>( evt->num_photon, 6, 4 ); 
    return p ; 
}

NP* SEvt::makeSeq() const 
{
    return NP::Make<unsigned long long>( evt->num_seq, 2, sseq::NSEQ );  
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


/**
SEvt::getMeta (an SCompProvider method)
-----------------------------------------

NB as metadata is collected in SEvt::gather_components 
from the SCompProvider this is NOT the source of metadata
for GPU running, that comes from QEvent::getMeta. 
This is only the source for CPU U4Recorder running (aka B evts).  

**/

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

NP* SEvt::gatherComponent(unsigned cmp) const 
{
    unsigned gather_mask = SEventConfig::GatherComp(); 
    return gather_mask & cmp ? gatherComponent_(cmp) : nullptr ; 
}

NP* SEvt::gatherComponent_(unsigned cmp) const 
{
    NP* a = nullptr ; 
    switch(cmp)
    {   
        case SCOMP_INPHOTON:  a = gatherInputPhoton() ; break ;   
        case SCOMP_G4STATE:   a = gatherG4State()     ; break ;   

        case SCOMP_GENSTEP:   a = gatherGenstep()  ; break ;   
        case SCOMP_DOMAIN:    a = gatherDomain()   ; break ;   
        case SCOMP_PHOTON:    a = gatherPhoton()   ; break ;   
        case SCOMP_RECORD:    a = gatherRecord()   ; break ;   
        case SCOMP_REC:       a = gatherRec()      ; break ;   
        case SCOMP_AUX:       a = gatherAux()      ; break ;   
        case SCOMP_SUP:       a = gatherSup()      ; break ;   
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


/**
SEvt::saveGenstep
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
void SEvt::saveGenstepLabels(const char* dir, const char* name) const 
{
    NP::Write<int>(dir, name, (int*)gs.data(), gs.size(), 4 ); 
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
       << " is_loaded " << ( is_loaded ? "YES" : "NO" )
       << std::endl
       << " is_loadfail " << ( is_loadfail ? "YES" : "NO" )
       << std::endl
       ;
    std::string s = ss.str(); 
    return s ; 
}

std::string SEvt::descFold() const 
{
    return fold->desc(); 
}
std::string SEvt::Brief() // static
{
    std::stringstream ss ; 
    ss << "SEvt::Brief " 
       << " SEvt::Exists(0) " << ( Exists(0) ? "Y" : "N" )
       << " SEvt::Exists(1) " << ( Exists(1) ? "Y" : "N" )
       << std::endl 
       << " SEvt::Get(0)->brief() " << ( Exists(0) ? Get(0)->brief() : "-" ) 
       << std::endl 
       << " SEvt::Get(1)->brief() " << ( Exists(0) ? Get(0)->brief() : "-" ) 
       << std::endl 
       ;
    std::string str = ss.str(); 
    return str ; 
} 
std::string SEvt::brief() const 
{
    std::stringstream ss ; 
    ss << "SEvt::brief " 
       << " getIndex " << getIndex()  
       << " hasInputPhoton " << ( hasInputPhoton() ? "Y" : "N" ) 
       << " hasInputPhotonTransformed " << ( hasInputPhotonTransformed() ? "Y" : "N" ) 
       ; 
    std::string str = ss.str(); 
    return str ; 
}

std::string SEvt::desc() const 
{
    std::stringstream ss ; 
    ss << evt->desc()
       << std::endl
       << descDir()
       << std::endl
       << dbg->desc()
       << std::endl
       << " g4state " << ( g4state ? g4state->sstr() : "-" )
       << std::endl
       << " SEventConfig::Initialize_COUNT " << SEventConfig::Initialize_COUNT 
       << std::endl
       ; 
    std::string str = ss.str(); 
    return str ; 
}

std::string SEvt::descDbg() const 
{
    std::stringstream ss ; 
    ss << dbg->desc() << std::endl ; 
    std::string str = ss.str(); 
    return str ; 
}

/**
SEvt::gather_components
------------------------

NB the provider is either this SEvt OR a QEvent instance held by QSim 

Note thet QEvent::setGenstep invoked SEvt::clear so the genstep vectors 
are clear when this gets called. So must rely on the contents of the 
fold to get the stats. 

**/

void SEvt::gather_components()   // *GATHER*
{
    int num_genstep = -1 ; 
    int num_photon  = -1 ; 
    int num_hit     = -1 ; 

    int num_comp = gather_comp.size() ; 

    for(int i=0 ; i < num_comp ; i++)
    {
        unsigned cmp = gather_comp[i] ;   
        const char* k = SComp::Name(cmp);    
        NP* a = provider->gatherComponent(cmp); 
        LOG(LEVEL) << " k " << std::setw(15) << k << " a " << ( a ? a->brief() : "-" ) ; 
        if(a == nullptr) continue ;  
        fold->add(k, a); 

        int num = a->shape[0] ;  
        if(     SComp::IsGenstep(cmp)) num_genstep = num ; 
        else if(SComp::IsPhoton(cmp))  num_photon = num ; 
        else if(SComp::IsHit(cmp))     num_hit = num ; 
    }


    assert( num_genstep > -1 ); 

    if( SEventConfig::IsRGModeSimulate())
    {
        assert( num_photon > -1 ); 
        LOG_IF(fatal, num_hit == -1 ) << " SKIP ASSERT : SHOULD NOW ALWAYS HAVE HIT ARRAY (EVEN IF EMPTY?)  AS HAVE SEvt::gatherHit  " ; 
        //assert( num_hit > -1 ); 
    }

    gather_total += 1 ;

    if(num_genstep > -1) genstep_total += num_genstep ;
    if(num_photon > -1)  photon_total += num_photon ;
    if(num_hit > -1)     hit_total += num_hit ; 

    LOG(LEVEL) 
        << " num_comp " << num_comp
        << " num_genstep " << num_genstep
        << " num_photon " << num_photon
        << " num_hit " << num_hit
        << " gather_total " << gather_total 
        << " genstep_total " << genstep_total 
        << " photon_total " << photon_total 
        << " hit_total " << hit_total 
        ;
}

void SEvt::gather_metadata()
{
    std::string provmeta = provider->getMeta(); 
    LOG(LEVEL) << " provmeta ["<< provmeta << "]" ; 
    fold->meta = provmeta ; 
}


/**
SEvt::gather
-------------

Collects the components configured by SEventConfig::CompMask
into NPFold from the SCompProvider which can either be:

* this SEvt instance for hostside running, eg U4RecorderTest, X4SimtraceTest
* the qudarap/QEvent instance for deviceside running, eg G4CXSimulateTest

**/

void SEvt::gather() 
{
    LOG_IF(fatal, gather_done) << " gather_done ALREADY : SKIPPING " ; 
    if(gather_done) return ; 
    gather_done = true ;   // SEvt::setNumPhoton which gets called by adding gensteps resets this to false

    gather_components(); 
    gather_metadata(); 
}




void SEvt::add_array( const char* k, const NP* a )
{
    fold->add(k, a);  
}

void SEvt::addEventConfigArray() 
{
    fold->add(SEventConfig::NAME, SEventConfig::Serialize() ); 
}

/**
SEvt::save
--------------

The component arrays are gathered by SEvt::gather_components
into the NPFold and then saved. Which components to gather and save 
are configured via SEventConfig::GatherComp and SEventConfig::SaveComp 
using the SComp enumeration. 

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


**Override with TMP envvar rather than OPTICKS_OUT_FOLD to still have auto-bookkeeping**

Note that when needing to override the default output directory it is usually 
preferable to use TMP envvar as most of the automatic bookkeeping will still be done in that case.

The below examples are with GEOM envvar set to "Pasta" and "FewPMT" with different executables:

+--------------------------------------------+-----------------------------------------------------------+
|   TMP envvar                               |  SEvt saveDir                                             | 
+============================================+===========================================================+
|    undefined                               |   /tmp/blyth/opticks/GEOM/Pasta/SEvtTest/ALL              |
+--------------------------------------------+-----------------------------------------------------------+
|   /tmp/$USER/opticks                       |   /tmp/blyth/opticks/GEOM/Pasta/SEvtTest/ALL              | 
+--------------------------------------------+-----------------------------------------------------------+
|   undefined                                |   /tmp/blyth/opticks/GEOM/FewPMT/U4SimulateTest/ALL0/000  |
+--------------------------------------------+-----------------------------------------------------------+
 
Only when more control of the output is needed is it appropriate to use OPTICKS_OUT_FOLD envvar.  

+--------------------------------------------+-----------------------------------------------------------+
|  OPTICKS_OUT_FOLD envvar                   |  SEvt saveDir                                             | 
+============================================+===========================================================+
|   undefined                                |   /tmp/blyth/opticks/GEOM/Pasta/SEvtTest/ALL              |
+--------------------------------------------+-----------------------------------------------------------+
|   /tmp/$USER/opticks                       |   /tmp/blyth/opticks/ALL                                  |
+--------------------------------------------+-----------------------------------------------------------+
|   /tmp/$USER/opticks/GEOM/$GEOM/SEvtTest   |   /tmp/blyth/opticks/GEOM/Pasta/SEvtTest/ALL              |
+--------------------------------------------+-----------------------------------------------------------+

* see tests/SEvtTest_saveDir.sh

**/

void SEvt::save() 
{
    const char* dir = DefaultDir(); 
    save(dir); 
}
void SEvt::saveExtra( const char* name, const NP* a  ) const 
{
    const char* dir = DefaultDir(); 
    saveExtra( dir, name , a );  
}


int SEvt::load()
{
    const char* dir = DefaultDir(); 
    int rc = load(dir); 
    LOG(LEVEL) << "SGeo::DefaultDir " << dir << " rc " << rc ;
    return rc ;  
}

void SEvt::save(const char* bas, const char* rel ) 
{
    const char* dir = SPath::Resolve(bas, rel, DIRPATH); 
    save(dir); 
}
void SEvt::save(const char* bas, const char* rel1, const char* rel2 ) 
{
    const char* dir = SPath::Resolve(bas, rel1, rel2,  DIRPATH); 
    save(dir); 
}


bool SEvt::hasIndex() const 
{
    return index != MISSING_INDEX ;  
}

bool SEvt::hasInstance() const 
{
    return instance != MISSING_INSTANCE ;  
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
    const char* reldir = GetReldir() ; 
    const char* dir = nullptr ; 
    if(hasIndex()) 
    { 
        assert( index > 0 ); 
        int u_index = 0 ; 

        if(SEventConfig::IsRGModeSimulate())
        {
            assert( instance == SEvt::EGPU || instance == SEvt::ECPU ); 
            u_index = instance == SEvt::EGPU ? index : -index ; 
        }
        else if(SEventConfig::IsRGModeSimtrace())
        {
            //assert( instance == MISSING_INSTANCE );  // not with U4SimtraceTest.sh 
            u_index = index ;  ; 
        }

        dir = SPath::Resolve(base, reldir, u_index, DIRPATH ) ; 
    }
    else
    {
        dir = SPath::Resolve(base, reldir,  DIRPATH) ; 
    }
    return dir ;  
}









/**
SEvt::RunDir
--------------

Directory without event index, used for run level metadata. 

**/

const char* SEvt::RunDir( const char* base_ )  // static
{
    const char* base = base_ ? base_ : SGeo::DefaultDir() ; 
    const char* reldir = GetReldir() ; 
    const char* dir = SPath::Resolve(base, reldir, DIRPATH ); 
    return dir ; 
}



const char* SEvt::DefaultDir() // static
{
    return SGeo::DefaultDir() ; 
}




std::string SEvt::descSaveDir(const char* dir_) const 
{
    const char* dir = getOutputDir(dir_); 
    const char* reldir = GetReldir() ; 
    bool with_index = index != MISSING_INDEX ;  
    std::stringstream ss ; 
    ss << "SEvt::descSaveDir"
       << " dir_ " << ( dir_ ? dir_ : "-" )
       << " dir  " << ( dir  ? dir  : "-" )
       << " reldir " << ( reldir ? reldir : "-" )
       << " with_index " << ( with_index ? "Y" : "N" )
       << " index " << ( with_index ? index : -1 ) 
       << " this " << std::hex << this << std::dec
       << std::endl
       ;
    std::string str = ss.str(); 
    return str ;  
} 

int SEvt::load(const char* dir_) 
{
    const char* dir = getOutputDir(dir_); 
    LOG(LEVEL) << " dir " << dir ; 
    LOG_IF(fatal, dir == nullptr) << " null dir : probably missing environment : run script, not executable directly " ;   
    assert(dir); 
    int rc = loadfold(dir); 
    return rc ; 
}

int SEvt::loadfold( const char* dir )
{
    LOG(LEVEL) << "[ fold.load " << dir ; 
    int rc = fold->load(dir); 
    LOG(LEVEL) << "] fold.load " << dir ; 
    is_loaded = true ; 
    onload(); 
    return rc ; 
}


void SEvt::onload()
{
    const NP* domain = fold->get(SComp::Name(SCOMP_DOMAIN)) ; 
    if(!domain) return ; 

    index = domain->get_meta<int>("index");  
    instance = domain->get_meta<int>("instance");  

    if(hasInstance()) // ie not MISSING_INSTANCE
    {
        Set(instance, this); 
    }

    LOG(LEVEL) 
        << " from domain " 
        << " index " << index
        << " instance " << instance
        ;

}




/**
SEvt::save
------------

If an index has been set with SEvt::setIndex SEvt::SetIndex 
and not unset with SEvt::UnsetIndex SEvt::unsetIndex
then the directory is suffixed with the index::

    /some/directory/001
    /some/directory/002
    /some/directory/003
   
HMM: what about a save following a gather ? does the download happen twice ?


Note that now that have *instance* do not actually need to fiddle
around with paired index.

**/

void SEvt::save(const char* dir_) 
{
    gather(); 

    LOG(LEVEL) << descComponent() ; 
    LOG(LEVEL) << descFold() ; 

    bool shallow = true ; 
    std::string save_comp = SEventConfig::SaveCompLabel() ; 
    NPFold* save_fold = fold->copy(save_comp.c_str(), shallow) ; 

    LOG_IF(LEVEL, save_fold == nullptr) << " NOTHING TO SAVE SEventConfig::SaveCompLabel/OPTICKS_SAVE_COMP  " << save_comp ; 
    if(save_fold == nullptr) return ;  

    const char* dir = getOutputDir(dir_); 
    LOG(info) << " dir " << dir <<  " index " << index << " instance " << instance  << " OPTICKS_SAVE_COMP  " << save_comp ; 
    LOG(LEVEL) << descSaveDir(dir_) ; 


    LOG(LEVEL) << "[ save_fold.save " << dir ; 
    save_fold->save(dir); 
    LOG(LEVEL) << "] save_fold.save " << dir ; 

    saveLabels(dir);   
    saveFrame(dir);   
    // could add these to the fold ?  
}

void SEvt::saveExtra(const char* dir_, const char* name, const NP* a ) const
{
    const char* dir = getOutputDir(dir_); 
    a->save(dir, name );  
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


std::string SEvt::descComponent() const 
{
    const NP* genstep  = fold->get(SComp::Name(SCOMP_GENSTEP)) ; 
    const NP* seed     = fold->get(SComp::Name(SCOMP_SEED)) ;  
    const NP* photon   = fold->get(SComp::Name(SCOMP_PHOTON)) ; 
    const NP* hit      = fold->get(SComp::Name(SCOMP_HIT)) ; 
    const NP* record   = fold->get(SComp::Name(SCOMP_RECORD)) ; 
    const NP* rec      = fold->get(SComp::Name(SCOMP_REC)) ;  
    const NP* aux      = fold->get(SComp::Name(SCOMP_REC)) ;  
    const NP* sup      = fold->get(SComp::Name(SCOMP_SUP)) ;  
    const NP* seq      = fold->get(SComp::Name(SCOMP_SEQ)) ; 
    const NP* domain   = fold->get(SComp::Name(SCOMP_DOMAIN)) ; 
    const NP* simtrace = fold->get(SComp::Name(SCOMP_SIMTRACE)) ; 
    const NP* g4state  = fold->get(SComp::Name(SCOMP_G4STATE)) ; 

    std::stringstream ss ; 
    ss << "SEvt::descComponent" 
       << std::endl 
       << std::setw(20) << " SEventConfig::GatherCompLabel " << SEventConfig::GatherCompLabel() << std::endl  
       << std::endl 
       << std::setw(20) << " SEventConfig::SaveCompLabel " << SEventConfig::SaveCompLabel() << std::endl  
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
       << std::setw(20) << "aux" << " " 
       << std::setw(20) << ( aux ? aux->sstr() : "-" ) 
       << " " 
       << std::setw(30) << "SEventConfig::MaxAux"
       << std::setw(20) << SEventConfig::MaxAux()
       << std::endl
       << std::setw(20) << "sup" << " " 
       << std::setw(20) << ( sup ? sup->sstr() : "-" ) 
       << " " 
       << std::setw(30) << "SEventConfig::MaxSup"
       << std::setw(20) << SEventConfig::MaxSup()
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
       << std::setw(20) << "g4state" << " " 
       << std::setw(20) << ( g4state ? g4state->sstr() : "-" ) 
       << " "
       << std::endl
       ;
    std::string s = ss.str(); 
    return s ; 
}
std::string SEvt::descComp() const 
{
    std::stringstream ss ; 
    ss << "SEvt::descComp " 
       << " gather_comp.size " << gather_comp.size() 
       << " SComp::Desc(gather_comp) " << SComp::Desc(gather_comp)
       << std::endl 
       << " save_comp.size "   << save_comp.size() 
       << " SComp::Desc(save_comp) " << SComp::Desc(save_comp)
       << std::endl 
       ; 
    std::string s = ss.str(); 
    return s ; 
}

std::string SEvt::descVec() const 
{
    std::stringstream ss ; 
    ss << "SEvt::descVec " 
       << " gather_comp " << gather_comp.size()  
       << " save_comp " << save_comp.size()  
       << " genstep " << genstep.size()  
       << " gs " << gs.size()  
       << " pho0 " << pho0.size()  
       << " pho " << pho.size()  
       << " slot " << slot.size()
       << " photon " << photon.size()  
       << " record " << record.size()  
       << " rec " << rec.size()  
       << " seq " << seq.size()  
       << " prd " << prd.size()  
       << " tag " << tag.size()  
       << " flat " << flat.size()  
       << " simtrace " << simtrace.size()  
       << " aux " << aux.size()  
       << " sup " << sup.size()  
       ; 
    std::string s = ss.str(); 
    return s ; 
}



const NP* SEvt::getPhoton() const { return fold->get(SComp::PHOTON_) ; }
const NP* SEvt::getHit() const {    return fold->get(SComp::HIT_) ; }
const NP* SEvt::getAux() const {    return fold->get(SComp::AUX_) ; }
const NP* SEvt::getSup() const {    return fold->get(SComp::SUP_) ; }

unsigned SEvt::getNumPhoton() const { return fold->get_num(SComp::PHOTON_) ; }
unsigned SEvt::getNumHit() const    
{ 
    int num = fold->get_num(SComp::HIT_) ;  // number of items in array 
    return num == NPFold::UNDEF ? 0 : num ;   // avoid returning -1 when no hits
}

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
    getPhotonFrame(fr, lp);   // HMM: this is just using lp.iindex
    fr.transform_w2m(lp); 
}

/**
SEvt::getLocalHit
------------------

Canonical usage from U4HitGet::FromEvt

1. copy *idx* hit from NP array into sphoton& lp struct 
2. uses lp.iindex (instance index) to lookup the frame from the SGeo* cf geometry  

   * TODO: check sensor_identifier, it should now be done GPU side already ? 

**/

void SEvt::getLocalHit(sphit& ht, sphoton& lp, unsigned idx) const 
{
    getHit(lp, idx);   // copy *idx* hit from NP array into sphoton& lp struct 

    sframe fr ; 
    getPhotonFrame(fr, lp); 
    fr.transform_w2m(lp); 

    ht.iindex = fr.inst() ; 
    ht.sensor_identifier = fr.sensor_identifier(); 
    ht.sensor_index = fr.sensor_index(); 
}

/**
SEvt::getPhotonFrame
---------------------

Note that this relies on the photon iindex which 
may not be set for photons ending in some places. 
It should always be set for photons ending on PMTs
assuming properly instanced geometry. 

**/

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
       << " getNumPhotonCollected "     <<  getNumPhotonCollected() 
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

std::string SEvt::descInputPhoton() const 
{
    const char* ip = SEventConfig::InputPhoton() ; 
    const char* ipf = SEventConfig::InputPhotonFrame() ; 
    int c1 = 35 ; 

    const char* div = " : " ; 
    std::stringstream ss ; 
    ss << "SEvt::descInputPhoton" << std::endl ;
    ss << std::setw(c1) << " SEventConfig::IntegrationMode "  << div << SEventConfig::IntegrationMode() << std::endl ; 
    ss << std::setw(c1) << " SEventConfig::InputPhoton "      << div << ( ip  ? ip  : "-" ) << std::endl ; 
    ss << std::setw(c1) << " SEventConfig::InputPhotonFrame " << div << ( ipf ? ipf : "-" ) << std::endl ; 
    ss << std::setw(c1) << " hasInputPhoton " << div << ( hasInputPhoton() ? "YES" : "NO " ) << std::endl ;  
    ss << std::setw(c1) << " input_photon "   << div << ( input_photon ? input_photon->sstr() : "-" )     << std::endl ;  
    ss << std::setw(c1) << " input_photon.lpath " << div << ( input_photon ? input_photon->get_lpath() : "--" ) << std::endl ; 
    ss << std::setw(c1) << " hasInputPhotonTransformed " << div << ( hasInputPhotonTransformed() ? "YES" : "NO " ) ;  
    std::string s = ss.str(); 
    return s ; 
}
std::string SEvt::DescInputPhoton(int idx) // static
{
    return Exists(idx) ? Get(idx)->descInputPhoton() : "-" ; 
}



std::string SEvt::descFull(unsigned max_print) const
{
    std::stringstream ss ; 
    ss << "[ SEvt::descFull "  << std::endl ; 
    ss << ( cf ? cf->descBase() : "no-cf" ) << std::endl ; 
    ss << descDir() << std::endl ;  
    ss << descNum() << std::endl ; 
    ss << descComponent() << std::endl ; 
    ss << descInputPhoton() << std::endl ; 

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

frame set by SEvt::setFrame is used to transform the photon into local "model" frame 

**/

void SEvt::getFramePhoton(sphoton& lp, unsigned idx) const 
{
    getPhoton(lp, idx); 
    applyFrameTransform(lp); 
}
void SEvt::getFrameHit(sphoton& lp, unsigned idx) const 
{
    getHit(lp, idx); 
    applyFrameTransform(lp); 
}
void SEvt::applyFrameTransform(sphoton& lp) const 
{
    bool zero_frame = frame.is_zero() ; 
    LOG_IF(fatal, zero_frame) << " must setFrame before can applyFrameTransform " ; 
    assert(!zero_frame);  
    frame.transform_w2m(lp); 
}

