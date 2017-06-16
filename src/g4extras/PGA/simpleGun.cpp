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

# include "sV_config.h"

# ifdef GEANT4_MC_MODEL

# include <G4ParticleGun.hh>
# include <G4ParticleTable.hh>

# include "g4extras/PGA.hpp"
# include "g4extras/PGA/simpleGun.hh"
//# include "utils.hpp"

# include "app/abstract.hpp"
# include "app/mixins/geant4.hpp"
# include "app/cvalidators.hpp"

namespace sV {

PrimaryGeneratorActionPG::PrimaryGeneratorActionPG(
            const std::string & particleType,
            const G4double & energyMeV,
            const G4ThreeVector & position,
            const G4ThreeVector & direction
    ) :
        G4VUserPrimaryGeneratorAction(),
        _pGunPtr(nullptr) {
    _pGunPtr = new G4ParticleGun( 1 );

    G4ParticleTable * particleTable = G4ParticleTable::GetParticleTable();
    G4ParticleDefinition * particle
        = particleTable->FindParticle( particleType );
    if( !particle ) {
        emraise( malformedArguments, "Couldn't find particle \"%s\".", particleType.c_str() );
    }

    _pGunPtr->SetParticleDefinition( particle );
    _pGunPtr->SetParticleEnergy( energyMeV*CLHEP::MeV );
    _pGunPtr->SetParticlePosition( position );
    _pGunPtr->SetParticleMomentumDirection( direction );

    sV_log1( "Simple particle gun %p created at {%.2e, %.2e, %.2e} / "
                 "{%.2e, %.2e, %.2e} for %s particles of %e MeV.\n",
              _pGunPtr,
              position.x(),     position.y(),   position.z(),
              direction.x(),    direction.y(),  direction.z(),
              particle->GetParticleName().c_str(),
              energyMeV );
}

PrimaryGeneratorActionPG::PrimaryGeneratorActionPG( const goo::dict::Dictionary & dict ) :
        PrimaryGeneratorActionPG(
            dict["particleType"].as<std::string>(),
            dict["energy-MeV"].as<double>(),
            dict["position-cm"].as<G4ThreeVector>()*CLHEP::cm,
            dict["direction"].as<G4ThreeVector>() ) {}

void
PrimaryGeneratorActionPG::GeneratePrimaries( G4Event * anEvent ) {
    _pGunPtr->GeneratePrimaryVertex( anEvent );
}

PrimaryGeneratorActionPG::~PrimaryGeneratorActionPG() {
    if( _pGunPtr ) {
        delete _pGunPtr;
    }
}

StromaV_DEFINE_GEANT4_PGA_MCONF( "simpleGun", PrimaryGeneratorActionPG ) {
    goo::dict::Dictionary lconf( "simpleGun", "Simple Geant4 particle gun "
        "primary events generator parameters." );
    lconf.insertion_proxy()
        .p<std::string>( "particleType", "Particle type to be emitted by gun." )
        .p<double>( "energy-MeV", "Initial particle energy to be emitted." )
        .p<G4ThreeVector>( "position-cm", "Initial position of particle "
                "emitted by gun.",
            G4ThreeVector(0., 0., 0.) )
        .p<G4ThreeVector>( "direction", "Direction of emitted particle.",
            G4ThreeVector(0., 0., -1.))
        ;
    goo::dict::DictionaryInjectionMap injM;
        injM( "particleType",       "Geant4.simpleGun.particleType" )
            ( "energy-MeV",         "Geant4.simpleGun.energy-MeV"   )
            ( "position-cm",        "Geant4.simpleGun.position-cm"  )
            ( "direction",          "Geant4.simpleGun.direction"    )
            ;
    return std::make_pair( lconf, injM );
}

}  // namespace sV

# endif  // GEANT4_MC_MODEL

