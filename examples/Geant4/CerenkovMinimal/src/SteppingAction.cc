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

#include "SteppingAction.hh"
#include "G4Track.hh"
#include "G4Step.hh"
#include "Ctx.hh"

#ifdef WITH_OPTICKS
#include "G4OpticksRecorder.hh"
#endif


SteppingAction::SteppingAction(Ctx* ctx_)
    :
    ctx(ctx_)
{
}

void SteppingAction::UserSteppingAction(const G4Step* step)
{
#ifdef WITH_OPTICKS
    if(ctx->_recorder)
    {
        ctx->_recorder->UserSteppingAction(step); 
    }
#endif
    ctx->setStep(step); 
}

