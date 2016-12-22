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

# include "dummy_PG.hh"

# include <G4Event.hh>
# include <G4ParticleGun.hh>
# include <G4ParticleTable.hh>
# include <G4ParticleDefinition.hh>
# include <G4SystemOfUnits.hh>

namespace mdlv {

DummyPG::DummyPG() {
    G4int n_particle = 1;
    _particleGunPtr = new G4ParticleGun(n_particle);

    G4ParticleTable* particleTable = G4ParticleTable::GetParticleTable();
    G4String particleName;
    _particleGunPtr->SetParticleDefinition(
               particleTable->FindParticle(particleName="geantino"));
    _particleGunPtr->SetParticleEnergy(1.0*GeV);
    _particleGunPtr->SetParticlePosition(G4ThreeVector(-2.0*m, 0.1, 0.1));
}

DummyPG::~DummyPG() {
    delete _particleGunPtr;
}

void
DummyPG::GeneratePrimaries(G4Event* anEvent) {
    G4int i = anEvent->GetEventID() % 3;
    G4ThreeVector v(1.0,0.0,0.0);
    switch(i) {
        case 0: {
        } break;
        case 1: {
            v.setY(0.1);
        } break;
        case 2: {
            v.setZ(0.1);
        }
    }
    _particleGunPtr->SetParticleMomentumDirection(v);
    _particleGunPtr->GeneratePrimaryVertex(anEvent);
}

}  // namespace mdlv


