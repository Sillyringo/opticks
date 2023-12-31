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

#include <cassert>
#include <sstream>
#include "SLOG.hh"

#include "OpticksConst.hh"
#include "Composition.hh"
#include "RenderStyle.hh"

const char* RenderStyle::R_PROJECTIVE_ = "R_PROJECTIVE" ;
const char* RenderStyle::R_RAYTRACED_  = "R_RAYTRACED" ; 
const char* RenderStyle::R_COMPOSITE_  = "R_COMPOSITE" ; 


RenderStyle::RenderStyle(Composition* composition) 
    :
    m_composition(composition),
    m_render_style(R_PROJECTIVE),
    m_raytrace_enabled(false)    // <-- enabled by OKGLTracer 
{
}


const char* RenderStyle::RenderStyleName(RenderStyle_t style) // static
{
    const char* s = NULL ; 
    switch(style)
    { 
       case R_PROJECTIVE: s = R_PROJECTIVE_ ; break ; 
       case R_RAYTRACED:  s = R_RAYTRACED_  ; break ; 
       case R_COMPOSITE:  s = R_COMPOSITE_  ; break ; 
       case NUM_RENDER_STYLE: s = NULL      ; break ; 
    }
    if(s == NULL)
    {
        LOG(fatal) << "invalid render style " << style ;  
    }
    assert(s);  
    return s ; 
}


const char* RenderStyle::getRenderStyleName() const 
{
    return RenderStyleName(getRenderStyle()) ; 
}

RenderStyle::RenderStyle_t RenderStyle::getRenderStyle() const 
{
    return m_render_style ; 
}

bool RenderStyle::isProjectiveRender() const 
{
   return m_render_style == R_PROJECTIVE ;
}
bool RenderStyle::isRaytracedRender() const 
{
   return m_render_style == R_RAYTRACED ;
}
bool RenderStyle::isCompositeRender() const 
{
   return m_render_style == R_COMPOSITE ;
}


std::string RenderStyle::desc() const 
{
    std::stringstream ss ; 
    ss << "RenderStyle "
       << getRenderStyleName() 
       ;
    return ss.str(); 
}


void RenderStyle::setRaytraceEnabled(bool raytrace_enabled) // set by OKGLTracer
{
    m_raytrace_enabled = raytrace_enabled ;
}

void RenderStyle::nextRenderStyle(unsigned /*modifiers*/)  // O:key cycling: Projective, Raytraced, Composite 
{
    if(!m_raytrace_enabled)
    {
        LOG(error) << "RenderStyle::nextRenderStyle is inhibited as RenderStyle::setRaytraceEnabled has not been called, see okgl.OKGLTracer " ;  
        return ; 
    }
    int next = (m_render_style + 1) % NUM_RENDER_STYLE ; 
    setRenderStyle(next); 
}


void RenderStyle::setRenderStyle( int style )
{
    m_render_style = (RenderStyle_t)style  ; 
    applyRenderStyle();
    m_composition->setChanged(true) ; // trying to avoid the need for shift-O nudging 

    LOG(info) << desc() ; 
}


void RenderStyle::command(const char* cmd) 
{ 
    LOG(info) << cmd ; 
    assert(strlen(cmd) == 2 );
    assert( cmd[0] == 'O' ); 

    int style = (int)cmd[1] - (int)'0' ;  
    setRenderStyle(style); 
}

void RenderStyle::applyRenderStyle()   
{
    // nothing to do, style is honoured by  Scene::render
}


