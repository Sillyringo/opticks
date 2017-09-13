
#include "SLog.hh"

#include "Opticks.hh"  // okc-
#include "OpticksHub.hh" // okg-
#include "OpticksSwitches.h" 

// opop-
#include "OpEngine.hh"
#include "OpIndexer.hh"
#include "OpSeeder.hh"
#include "OpZeroer.hh"

// optixrap-
#include "OConfig.hh"
#include "OContext.hh"
#include "OEvent.hh"
#include "OPropagator.hh"
#include "OScene.hh"

#include "PLOG.hh"


unsigned OpEngine::getOptiXVersion()
{
   return OConfig::OptiXVersion();
}

OContext* OpEngine::getOContext()
{
    return m_scene->getOContext(); 
}

OPropagator* OpEngine::getOPropagator()
{
    return m_propagator ; 
}

OpEngine::OpEngine(OpticksHub* hub) 
     : 
      m_log(new SLog("OpEngine::OpEngine")),
      m_hub(hub),
      m_ok(m_hub->getOpticks()),
      m_scene(new OScene(m_hub)),
      m_ocontext(m_scene->getOContext()),
      m_entry(NULL),
      m_oevt(NULL),
      m_propagator(NULL),
      m_seeder(NULL),
      m_zeroer(NULL),
      m_indexer(NULL)
{
   init();
   (*m_log)("DONE");
}
void OpEngine::init()
{
   m_ok->setOptiXVersion(OConfig::OptiXVersion()); 
   if(m_ok->isLoad())
   {
       LOG(warning) << "OpEngine::init skip initPropagation as just loading pre-cooked event " ;
   }
   else if(m_ok->isTracer())
   {
       LOG(warning) << "OpEngine::init skip initPropagation as tracer mode is active  " ; 
   }
   else
   {
       LOG(warning) << "OpEngine::init initPropagation START" ;
       initPropagation(); 
       LOG(warning) << "OpEngine::init initPropagation DONE" ;

   }
}

void OpEngine::initPropagation()
{
    m_entry = m_ocontext->addEntry(m_ok->getEntryCode()) ;
    m_oevt = new OEvent(m_hub, m_ocontext);
    m_propagator = new OPropagator(m_hub, m_oevt, m_entry);
    m_seeder = new OpSeeder(m_hub, m_oevt) ;
    m_zeroer = new OpZeroer(m_hub, m_oevt) ;
    m_indexer = new OpIndexer(m_hub, m_oevt) ;
}



 

unsigned OpEngine::uploadEvent()
{
    return m_oevt->upload();                   // creates OptiX buffers, uploads gensteps
}

void OpEngine::propagate()
{
    m_seeder->seedPhotonsFromGensteps();  // distributes genstep indices into the photons buffer OR seed buffer

    m_oevt->markDirty();                   // inform OptiX that must sync with the CUDA modified photon/seed depending on WITH_SEED_BUFFER 

    //m_zeroer->zeroRecords();              // zeros on GPU record buffer via OptiX or OpenGL  (not working OptiX 4 in interop)

    m_propagator->launch();               // perform OptiX GPU propagation : write the photon, record and sequence buffers

    indexEvent();
}


void OpEngine::indexEvent()
{
    if(m_ok->isProduction()) return ; 

#ifdef WITH_RECORD
    m_indexer->indexSequence();
#endif
    m_indexer->indexBoundaries();
}


unsigned OpEngine::downloadEvent()
{
    return m_oevt->download();
}


void OpEngine::cleanup()
{
    m_scene->cleanup();
}

void OpEngine::Summary(const char* msg)
{
    LOG(info) << msg ; 
}


void OpEngine::downloadPhotonData()  // was used for debugging of seeding (buffer overwrite in interop mode on Linux)
{
     if(m_ok->isCompute()) m_oevt->downloadPhotonData(); 
}

