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

#include <sstream>

#include <boost/lexical_cast.hpp>
#include <boost/math/constants/constants.hpp>

#include "SLOG.hh"

#include "NGLM.hpp"
#include "GLMFormat.hpp"
#include "Animator.hh"
#include "OrbitalView.hh"


const char* OrbitalView::PREFIX = "orbitalview" ;
const char* OrbitalView::getPrefix()
{
    return PREFIX ; 
}

OrbitalView::OrbitalView(View* basis, unsigned int period, bool verbose) 
     : 
     View(ORBITAL),
     m_basis(basis),
     m_count(0),
     m_period(period),
     m_fraction(0.f),
     m_animator(NULL),
     m_verbose(verbose)
{
    init();
}

Animator* OrbitalView::getAnimator()
{
    return m_animator ; 
}

void OrbitalView::setFraction(float fraction)
{
    m_fraction = fraction ; 
}

void OrbitalView::init()
{
    m_animator = new Animator(&m_fraction, m_period, 0.f, 1.f, "OrbitalView"); 
    //m_animator->setModeRestrict(Animator::NORM);  // only OFF,SLOW,NORM,FAST, 
    if(m_verbose) m_animator->Summary("OrbitalView::init");
    m_animator->setMode(Animator::SLOW8);
}

bool OrbitalView::hasChanged()
{
    return m_count > 0 && m_animator->isActive() ;  
}

void OrbitalView::nextMode(unsigned int modifiers)
{
    m_animator->nextMode(modifiers);
}

bool OrbitalView::isActive()
{
    return m_animator->isActive();
}


void OrbitalView::tick()
{
    m_count++ ; 

    bool bump(false);

    m_animator->step(bump);

    //LOG(info) << description("IV::tick") << " : " << m_animator->description() ;

    if(bump)
    {
        LOG(info) << description("OrbitalView::tick BUMP ") ; 
    }

    update();
}


std::string OrbitalView::description(const char* msg)
{
    std::stringstream ss ; 
    ss << msg 
       << " fr " << m_fraction 
       << " cn " << m_count
       ;
    return ss.str();
}


/**
OrbitalView::update
---------------------

Uses basis view eye position (x,y) to define the 
radius and initial phase (phi angle) of the orbit.
The gaze direction is tangential to the path 

**/

void OrbitalView::update()
{
    glm::vec4 base_m = m_basis->getEye() ;
    glm::vec3 tmp(base_m);
    tmp.z = 0.f ;  // hmm should dot product with up direction ?
    float r = glm::length(tmp) ; 
    float phase = atan2( tmp.y, tmp.x ) ;  // hmm, tis tangential view anyhow so a jump is inevitable
    float pi = boost::math::constants::pi<float>() ;
    float phi = phase + m_fraction*pi*2.0f  ;
    float sinphi = sin(phi);
    float cosphi = cos(phi);

    glm::vec4 hub = glm::vec4( 0.f , 0.f, base_m.z, 1.f );
    glm::vec4 gaze = glm::vec4( -r*sinphi, r*cosphi, 0.f, 0.f ) ;

    m_orb_eye  = glm::vec4( r*cosphi, r*sinphi, base_m.z, 1.f ) ;
    m_orb_look = m_orb_eye + gaze ; 
    m_orb_up = hub - m_orb_eye ; 

    LOG(debug) << "OrbitalView::update"
              << " base_m " << gformat(base_m)
              << " m_orb_eye " << gformat(m_orb_eye)
              << " m_orb_look " << gformat(m_orb_look)
              << " m_orb_up " << gformat(m_orb_up)
              << " r " << r 
              << " phi " << phi 
              << " fraction " << m_fraction
              ;
}


glm::vec4 OrbitalView::getEye(const glm::mat4& m2w) 
{ 
    if(m_count == 0) update();
    glm::vec4 eye_w = m2w * m_orb_eye ; 
    return eye_w ;  
} 

glm::vec4 OrbitalView::getLook(const glm::mat4& m2w) 
{ 
    if(m_count == 0) update();
    glm::vec4 look_w = m2w * m_orb_look ; 
    return look_w ;  
} 

glm::vec4 OrbitalView::getUp(const glm::mat4& m2w) 
{ 
    if(m_count == 0) update();
    glm::vec4 up_w = m2w * m_orb_up ; 
    return up_w ;  
} 

glm::vec4 OrbitalView::getGaze(const glm::mat4& m2w, bool )
{
    glm::vec4 eye = getEye(m2w);
    glm::vec4 look = getLook(m2w);
    glm::vec4 gaze = look - eye ; 
    return gaze ;                // w=0. OK as direction
}

void OrbitalView::Summary(const char* msg)
{
    LOG(info) << msg ;
    m_basis->Summary(msg); 
}


