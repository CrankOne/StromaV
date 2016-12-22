/*
 * Copyright (c) 2016 Renat R. Dusaev <crank@qcrypt.org>
 * Author: Renat R. Dusaev <crank@qcrypt.org>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

# ifndef H_STROMA_V_SYNCHROTRON_RADIATION_IN_MAT_PHYSICS_H
# define H_STROMA_V_SYNCHROTRON_RADIATION_IN_MAT_PHYSICS_H

# include "../../config.h"

# ifdef GEANT4_MC_MODEL
# ifdef GEANT4_DYNAMIC_PHYSICS

# include <Geant4/G4VPhysicsConstructor.hh>
# include "g4extras/PhysList.hpp"

/**@file SynchrotronRadiationPhysics.hpp
 *
 * Complementary physics for ModularPhysList 
 * contains SynchrotronRadiationInMat.
 *
 * It is required that to G4EmStandardPhysics physics module to be
 * included.
 */

namespace sV {

class SynchrotronRadiationPhysics : public G4VPhysicsConstructor {
private:
    bool _inMaterials;
    int _verbosityLevel;
public:
    SynchrotronRadiationPhysics( int verbosity=-1, bool inMaterials=true );
    virtual ~SynchrotronRadiationPhysics();

    virtual void ConstructParticle() override;
    virtual void ConstructProcess() override;
    //  goo::app<AbstractApplication>().cfg_option<bool>("g4.UseSR")  TODO
    bool is_in_materials() const { return _inMaterials; }
};  // class SynchrotronRadiationPhysics

/// Custom constructor --- extracts parameters from application config.
template<> SynchrotronRadiationPhysics *
ModularPhysicsList::construct_physics<SynchrotronRadiationPhysics>();

}  // namespace sV

# endif  // GEANT4_DYNAMIC_PHYSICS
# endif  // GEANT4_MC_MODEL

# endif  // H_STROMA_V_SYNCHROTRON_RADIATION_IN_MAT_PHYSICS_H

