#include <sstream>
#include <iomanip>
#include "NGLM.hpp"
#include "SLOG.hh"

#include "OpticksPhotonFlags.hh"

const plog::Severity OpticksPhotonFlags::LEVEL = SLOG::EnvLevel("OpticksPhotonFlags", "DEBUG"); 


OpticksPhotonFlags::OpticksPhotonFlags( const glm::vec4& f )
    :
    boundary(Boundary(f)),
    sensorIndex(SensorIndex(f)),
    nodeIndex(NodeIndex(f)),
    photonIndex(PhotonIndex(f)),
    flagMask(FlagMask(f))
{
    LOG(LEVEL); 
}

OpticksPhotonFlags::OpticksPhotonFlags(int boundary_, unsigned sensorIndex_, unsigned nodeIndex_, unsigned photonIndex_, unsigned flagMask_ )
    :
    boundary(boundary_),
    sensorIndex(sensorIndex_),
    nodeIndex(nodeIndex_),
    photonIndex(photonIndex_),
    flagMask(flagMask_)
{
    LOG(LEVEL); 
}

bool OpticksPhotonFlags::operator==(const OpticksPhotonFlags& other) const 
{
    return 
        boundary == other.boundary  && 
        sensorIndex == other.sensorIndex  && 
        nodeIndex == other.nodeIndex  && 
        photonIndex == other.photonIndex  && 
        flagMask == other.flagMask  
        ; 
}

std::string OpticksPhotonFlags::desc() const 
{
    std::stringstream ss ; 
    ss
        << " boundary "    << std::setw(10) << boundary
        << " sensorIndex " << std::setw(10) << sensorIndex
        << " nodeIndex "   << std::setw(10) << nodeIndex
        << " photonIndex " << std::setw(10) << photonIndex 
        << " flagMask "    << std::setw(10) << std::hex << flagMask << std::dec 
        ;
    return ss.str(); 
}

std::string OpticksPhotonFlags::brief() const 
{
    std::stringstream ss ; 
    ss
        << "("
        << " b " 
        << std::setw(4) << boundary
        << " s "
        << std::setw(5) << sensorIndex
        << " n "
        << std::setw(6) << nodeIndex
        << " p "
        << std::setw(6) << photonIndex 
        << " f "
        << std::setw(10) << std::hex << flagMask << std::dec
        << ")"
        ;
    return ss.str(); 
}

/**
OpticksPhotonFlags::Boundary OpticksPhotonFlags::SensorIndex OpticksPhotonFlags::NodeIndex OpticksPhotonFlags::PhotonIndex OpticksPhotonFlags::FlagMask
--------------------------------------------------------------------------------------------------------------------------------------------------------

The somewhat curious implementation of these accessors is to allow the underlying layout and compression etc.. of the 
photon flags to be changed without changing the accessing code.  Basically the only assumption 
that cannot easily be changed and hidden in the implementation  is that the last of the four quads of the 
photon item is used for the photon flags. 
**/


int OpticksPhotonFlags::Boundary(const float& x, const float& , const float& , const float& ) // static
{
    uif_t uif ; 
    uif.f = x ; 
    unsigned hi = uif.u >> 16 ;          
    return hi <= 0x7fff  ? hi : hi - 0x10000 ;  // twos-complement see SPack::unsigned_as_int 
}
unsigned OpticksPhotonFlags::SensorIndex(const float& x, const float& , const float& , const float& ) // static
{
    uif_t uif ; 
    uif.f = x ; 
    unsigned lo = uif.u & 0xffff  ; 
    return lo ; 
}
unsigned OpticksPhotonFlags::NodeIndex(const float&, const float& y, const float& , const float& )
{
    uif_t uif ; 
    uif.f = y ; 
    return uif.u  ;          
}
unsigned OpticksPhotonFlags::PhotonIndex(const float&, const float& , const float& z, const float& )
{
    uif_t uif ; 
    uif.f = z ; 
    return uif.u  ;          
}
unsigned OpticksPhotonFlags::FlagMask(const float&, const float& , const float& z, const float& w)
{
    uif_t uif ; 
    uif.f = w ; 
    return uif.u  ;          
}

int      OpticksPhotonFlags::Boundary(     const glm::vec4& f){ return OpticksPhotonFlags::Boundary(    f.x, f.y, f.z, f.w); }
unsigned OpticksPhotonFlags::SensorIndex(  const glm::vec4& f){ return OpticksPhotonFlags::SensorIndex( f.x, f.y, f.z, f.w); }
unsigned OpticksPhotonFlags::NodeIndex(    const glm::vec4& f){ return OpticksPhotonFlags::NodeIndex(   f.x, f.y, f.z, f.w); }
unsigned OpticksPhotonFlags::PhotonIndex(  const glm::vec4& f){ return OpticksPhotonFlags::PhotonIndex( f.x, f.y, f.z, f.w); }
unsigned OpticksPhotonFlags::FlagMask(     const glm::vec4& f){ return OpticksPhotonFlags::FlagMask(    f.x, f.y, f.z, f.w); }

