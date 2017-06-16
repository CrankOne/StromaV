/*
 * Copyright (c) 2016 Renat R. Dusaev <crank@qcrypt.org>
 * Author: Renat R. Dusaev <crank@qcrypt.org>
 * Author: Bogdan Vasilishin <togetherwithra@gmail.com>
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

# ifndef H_STROMA_V_SIMULATION_PARTICLE_GUN_H
# define H_STROMA_V_SIMULATION_PARTICLE_GUN_H

# include "sV_config.h"

# ifdef GEANT4_MC_MODEL

# include <G4VUserPrimaryGeneratorAction.hh>
# include <globals.hh>
# include <goo_dict/dict.hpp>

class G4Event;
class G4ParticleGun;

/*
 * SIMPLE GENERATORS
 */

/**@class PrimaryGeneratorActionPG
 * @brief Particle gun primary generator.
 *
 * The need of simple and naive primary event generator source
 * is a common case for initial accelerator-oriented simulation.
 *
 * The PrimaryGeneratorActionPG implements only one particle at
 * time primary event generation.
 * */

namespace sV {

class PrimaryGeneratorActionPG :
        public G4VUserPrimaryGeneratorAction {
private:
    G4ParticleGun                   * _pGunPtr;
public:
    PrimaryGeneratorActionPG(
            const std::string & particleType,
            const G4double & energyMeV,
            const G4ThreeVector & position,
            const G4ThreeVector & direction
        );
    PrimaryGeneratorActionPG( const goo::dict::Dictionary & );
    virtual ~PrimaryGeneratorActionPG();
    virtual void GeneratePrimaries(G4Event* anEvent) override;

    G4ParticleGun * get_particle_gun_ptr() { return _pGunPtr; }
    const G4ParticleGun * get_particle_gun_ptr() const { return _pGunPtr; }
};

}        // namespace sV

# endif  // GEANT4_MC_MODEL

# endif  // H_STROMA_V_SIMULATION_PARTICLE_GUN_H

