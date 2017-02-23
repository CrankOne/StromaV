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
# include "config.h"

# ifdef GEANT4_MC_MODEL
# ifdef GEANT4_DYNAMIC_PHYSICS

# include "app/abstract.hpp"

# include "g4extras/physics/SynchrotronRadiation.hpp"

# include <Geant4/G4Electron.hh>
# include <Geant4/G4Positron.hh>
# include <Geant4/G4MuonPlus.hh>
# include <Geant4/G4MuonMinus.hh>
# include <Geant4/G4Proton.hh>
# include <Geant4/G4SynchrotronRadiationInMat.hh>
# include <Geant4/G4SynchrotronRadiation.hh>
# include <Geant4/G4StepLimiter.hh>
# include <Geant4/G4Version.hh>
# if G4VERSION_NUMBER > 999
    # include <Geant4/G4AutoDelete.hh>  // introduced since Geant 4.10
# endif
# include <Geant4/G4PhysicsListHelper.hh>
# include <Geant4/G4ProcessType.hh>

# if 1
// factory
# include "G4PhysicsConstructorFactory.hh"
namespace sV{
    G4_DECLARE_PHYSCONSTR_FACTORY(SynchrotronRadiationPhysics);
}
# endif

# if G4VERSION_NUMBER > 999
# if defined( __clang__ ) && ! defined( G4MULTITHREADED )
// When using aParticleIterator without multithreading support, an in-template
// static field is used for .offset attr. This causes Clang to complain about
// actual attribute instantiation. The problem is, besides of this reasonable
// notion, Clang may generate multiple instances for such attributes when
// they're operating in different threads. To suppress this annoying warning we
// define it here in hope it won't lead to dangerous consequencies.
template<> G4VPCData * G4VUPLSplitter<G4VPCData>::offset;
# endif  // G4_TLS
# endif

// 1) `G4SynchrotronRadiation` -- stable process, which included in
//      G4EmExtraPhysics builder as an option. Was validated and published by
//      Geant4 team.
//   `G4SynchrotronRadiationInMat` -- experimental process. It has correction for
//      low energy suppresion of the spectrum(Ter-Mikaelian effect). It influence on
//      finite mean free path of the process and etc.
//      http://hypernews.slac.stanford.edu/HyperNews/geant4/get/emprocess/997.html?inline=1
//
// 2) `procMan->AddProcess( SomeProcess,     -1, -1, 5 );`
//      "The general interface to a physics process allows 3 stage of interactions:
//      AtRest, AlongTheStep, PostStep. A process may be active not at all stages.
//      There are limitations on the possible order of calls to processes at each
//      of the stage. For example, trasportation action AlongStep should be before
//      any action of any process - first one need to define the step and than apply
//      other processes. Each of 3 integer numbers shows the order of the process.
//      Negative number means that the process is inactive at the give stage.
//
//      In the current version of Geant4 the optimal EM physics list is discussed
//      and shown in the EM web page:
//      https://twiki.cern.ch/twiki/bin/view/Geant4/ElectromagneticPhysics#PhysicsLists"
//           Vladimir Ivanchenko.
//      http://hypernews.slac.stanford.edu/HyperNews/geant4/get/phys-list/516/1.html
//    Now we use G4PhysicsListHelper, one should just register process via it.
//    Though, there is question about order of adding processes. How the user can
//    control this order with G4PhysicsListHelper?
//
namespace sV {

template<> SynchrotronRadiationPhysics *
ModularPhysicsList::construct_physics<SynchrotronRadiationPhysics>() {
    //std::cout << " === #0 === " << std::endl;  // XXX
    return new SynchrotronRadiationPhysics(
                ModularPhysicsList::physicsVerbosity,
                goo::app<sV::AbstractApplication>().co()["extraPhysics.physicsSR.considerMaterials"].as<bool>()
            );
}

SynchrotronRadiationPhysics::SynchrotronRadiationPhysics( int verbosity, bool inMaterials )
    : G4VPhysicsConstructor( inMaterials ? "SynchrotronRadiationInMat" : "SynchrotronRadiation" ),
     _inMaterials( inMaterials ),
     _verbosityLevel( verbosity > 0 ? verbosity :
                      goo::app<sV::AbstractApplication>().co()["verbosity"].as<int>() ) {
    // Possible options. It affects on process order in stage:
    // fNotDefined,
    // fTransportation,
    // fElectromagnetic,
    // fOptical,             
    // fHadronic,
    // fPhotolepton_hadron,
    // fDecay,
    // fGeneral,
    // fParameterisation,
    // fUserDefined 
    SetPhysicsType(fUserDefined);
    // XXX std::cout << "SR Constructor has been invoked" << std::endl;
}

SynchrotronRadiationPhysics::~SynchrotronRadiationPhysics() {
}

void SynchrotronRadiationPhysics::ConstructParticle() {
    //std::cout << " === #1 === " << std::endl;  // XXX
    G4Electron::Electron();
    G4Positron::Positron();
    G4MuonPlus::MuonPlus();
    G4MuonMinus::MuonMinus();
    G4Proton::Proton();
}

void SynchrotronRadiationPhysics::ConstructProcess() {
    G4PhysicsListHelper* ph = G4PhysicsListHelper::GetPhysicsListHelper();
    # if G4VERSION_NUMBER > 999
    // This line causes clang to doubt about unavailable definition. To supress
    // his warnings, we use another form of the same thing:
    # if 1
    for( aParticleIterator->reset(); (*aParticleIterator)() ; )
    # else
    auto localParticleIterator =
            (G4VPhysicsConstructor::GetSubInstanceManager()
                        .offset[g4vpcInstanceID])._aParticleIterator;
    for( localParticleIterator->reset();
         (*localParticleIterator)() ; )
    # endif
    # else
    for( theParticleIterator->reset(); (*theParticleIterator)() ; )
    # endif
    {
        G4ParticleDefinition* particle =
        # if G4VERSION_NUMBER > 999
        aParticleIterator->value();
        # else
        theParticleIterator->value();
        # endif
        G4String particleName = particle->GetParticleName();

        G4VDiscreteProcess * SRProcPtr = ( _inMaterials ?
                dynamic_cast<G4VDiscreteProcess *>(new G4SynchrotronRadiationInMat()) :
                dynamic_cast<G4VDiscreteProcess *>(new G4SynchrotronRadiation()) );
        # if G4VERSION_NUMBER > 999
        G4AutoDelete::Register( SRProcPtr );  // introduced since Geant 4.10
        # endif

        if (particleName=="e-") {
            ph->RegisterProcess(SRProcPtr,          particle );
            ph->RegisterProcess(new G4StepLimiter,  particle );
        } else if (particleName=="e+" || particleName=="mu+"
                                      || particleName=="mu-"
                                      || particleName=="proton") {
            // XXX Previous way 0.1: procMan->AddProcess(fSR,               -1, -1, 5 );
            ph->RegisterProcess(SRProcPtr,          particle );
            ph->RegisterProcess(new G4StepLimiter,  particle );
        } else if (particle->GetPDGCharge() != 0.0 &&
                 !particle->IsShortLived()) {
            ph->RegisterProcess(SRProcPtr,          particle );
        }
        // XXX std::cout << "SR process has been registered for " << particleName << std::endl;
    }
}

REGISTER_PHYSICS_MODULE( SynchrotronRadiationPhysics );

}   //  namespace sV

# endif  // GEANT4_DYNAMIC_PHYSICS
# endif  // GEANT4_MC_MODEL

