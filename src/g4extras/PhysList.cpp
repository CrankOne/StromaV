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

# include "sV_config.h"

# ifdef GEANT4_MC_MODEL

# include "app/abstract.hpp"

# include "g4extras/PhysList.hpp"
# include "g4extras/auto.out/PhysList.ihpp"

# ifdef GEANT4_DYNAMIC_PHYSICS
# include <vector>
# include <unordered_map>
// This headers only belongs to modular and dynamic physics lists:
# include "g4extras/auto.out/Particles.ihpp"
# include "g4extras/auto.out/Physics.ihpp"
# endif

# include <G4PhysListFactory.hh>
# include <G4VModularPhysicsList.hh>
# include <boost/lexical_cast.hpp>

namespace sV {

static std::list< G4VUserPhysicsList * > * _static_phlPtrsStorage = nullptr;

static void _free_allocated_phlists() __attribute__(( __destructor__(156) ));

//
// Modular physics list class declaration
//

# ifdef GEANT4_DYNAMIC_PHYSICS

# define declare_constructor_static_method( physicsName )       \
static G4VPhysicsConstructor * _static_construct_ ## physicsName ();
for_each_physics( declare_constructor_static_method )
# undef declare_constructor_static_method

std::unordered_map<std::string, G4VPhysicsConstructor * (*)()> *
ModularPhysicsList::_phListCtrsDict = nullptr;
# endif  // GEANT4_DYNAMIC_PHYSICS

//

// TODO: see examples/basic/B3 for quite simple examples.

//
//
//

G4VUserPhysicsList *
obtain_physics_list_instance(
        const std::string & name
    ) {
    // FIXME: up to now the code below permits configurations that apparently
    // has to be forbidden from Geant4 API point of view. Due to quite unclear
    // distinctions between modular/factory/pre-defined physics list we still
    // did not implement a self-consistent configuration mechanics.

    G4VUserPhysicsList * rs = nullptr;
    if( !_static_phlPtrsStorage ) {
        _static_phlPtrsStorage = new std::list< G4VUserPhysicsList * >();
    }

    std::list<std::string> ml = goo::app<sV::AbstractApplication>()
            .co()["Geant4.extraPhysics.physlist.module"]
            .as_list_of<std::string>()
            ;
    std::vector<std::string> modulesList( ml.begin(), ml.end() );

    // Note: `none' option is handled by level above as this function should
    // never return nullptr.
    if( "factory" == name ) {
        G4PhysListFactory factory;
        rs = factory.GetReferencePhysList(
                goo::app<sV::AbstractApplication>()
                .co()["Geant4.extraPhysics.productName"]
                .as<std::string>()
            );
    } else
    # ifdef GEANT4_DYNAMIC_PHYSICS
    if( "modular" == name ) {
        rs = new ModularPhysicsList( modulesList );
    } else
    # endif
    # define construct_and_return_physics_list( nm )    \
    if( # nm == name ) {                                \
        rs = new nm ();                                 \
    } else
    for_each_physlist( construct_and_return_physics_list )
    # undef construct_and_return_physics_list

    # ifdef GEANT4_DYNAMIC_PHYSICS
    if( "dynamic" == name ) {
        rs = new DynamicPhysicsList();
    } else
    # endif
    {
        emraise( noSuchKey,
            "Unknown PhysList name provided: '%s'.\n",
            name.c_str() );
    }

    if( modulesList.size()
        && "modular" != name  ) {
        G4VModularPhysicsList * modularPtr =
                                    dynamic_cast<G4VModularPhysicsList *>(rs);
        if( modularPtr ) {
            ModularPhysicsList::register_modules_at( modulesList,
                                                     modularPtr );
        } else {
            sV_logw( "Unable to add physics modules to \"%s\" Geant4 phys.list "
                "referred by %p since it does not support modular physics.\n",
                name.c_str(), rs );
        }
    }

    if( rs ) {
        _static_phlPtrsStorage->push_front( rs );
    }

    {
        std::string verbosity = goo::app<sV::AbstractApplication>()
                .co()["Geant4.extraPhysics.verbosity"]
                .as<std::string>();
        if( "application" != verbosity ) {
            try {
                rs->SetVerboseLevel( boost::lexical_cast<int>(verbosity) );
            } catch( const boost::bad_lexical_cast &) {
                emraise( malformedArguments, "Can not set `extraPhysics.verbosity' to `%s'."
                    "Only numerical value and `application' token can be interpreted.",
                    verbosity.c_str() );
            }
        }
    }

    {
        int physicsListVerbosityLevel = goo::app<sV::AbstractApplication>().verbosity();
        std::string verbosity = goo::app<sV::AbstractApplication>()
                .co()["Geant4.extraPhysics.verbosity"]
                .as<std::string>();
        if( "application" != verbosity ) {
            try {
                physicsListVerbosityLevel = boost::lexical_cast<int>(verbosity);
            } catch( const boost::bad_lexical_cast &) {
                emraise( malformedArguments, "Can not set `extraPhysics.verbosity' to `%s'."
                    "Only numerical value and `application' token can be interpreted.",
                    verbosity.c_str() );
            }
        }
        # ifdef GEANT4_DYNAMIC_PHYSICS
        ModularPhysicsList::physicsVerbosity = physicsListVerbosityLevel;
        rs->SetVerboseLevel( physicsListVerbosityLevel );
        # else
        (void)(physicsListVerbosityLevel);
        # endif
    }

    return rs;
}

/// Returns a std::vector<std::string> containing available PhysLists.
std::vector<std::string>
available_physics_lists() {
    std::vector<std::string> res;
    # define push_back_physlist_name( nm ) \
        res.push_back( # nm );
    for_each_physlist( push_back_physlist_name )
    # undef push_back_physlist_name
    return res;
}

static void
_free_allocated_phlists() {
    if( !_static_phlPtrsStorage ) return;
    for( auto it = _static_phlPtrsStorage->begin();
            _static_phlPtrsStorage->end() != it; ++it ) {
        //delete *it;  // TODO: G4RunManager deletes it by himself.
    }
    delete _static_phlPtrsStorage;
}

//
// DynamicPhysicsList (TODO)
// {{{

# ifdef GEANT4_DYNAMIC_PHYSICS
DynamicPhysicsList::DynamicPhysicsList() : G4VUserPhysicsList() {
    defaultCutValue = 1.*CLHEP::km;
    //fSRType = true; 
    //fMess = new PhysicsListMessenger(this);
}

DynamicPhysicsList::~DynamicPhysicsList() {
    //delete fMess;
}

void
DynamicPhysicsList::ConstructParticle() {
    # if 0
    for( auto it = .begin() ; it != .end(); ++it ) {
        //G4AntiNeutrinoMu::AntiNeutrinoMuDefinition();
        # define construct_particle_by_definition( particleName ) \
        if( # particleName == *it ) {
            G4 ## particleName :: particleName ## Definition();
        }
    }
    # else
    _TODO_  // TODO
    # endif
}

void
DynamicPhysicsList::ConstructProcess() {
    # if 0
    AddTransportation();
    ConstructEM();
    ConstructGeneral();
    # else
    _TODO_  // TODO
    # endif
}

void
DynamicPhysicsList::ConstructEM() {
    # if 0
    theParticleIterator->reset();
    while( (*theParticleIterator)() ){
        G4ParticleDefinition* particle = theParticleIterator->value();
        G4ProcessManager* pmanager = particle->GetProcessManager();
        G4String particleName = particle->GetParticleName();
        if( particleName == "gamma" ) {
            // gamma
            pmanager->AddDiscreteProcess( new G4PhotoElectricEffect );
            pmanager->AddDiscreteProcess( new G4ComptonScattering );
            pmanager->AddDiscreteProcess( new G4GammaConversion );
            pmanager->AddDiscreteProcess( new G4RayleighScattering );
        } else if( particleName == "e-" ) {
            //electron
            pmanager->AddProcess( new G4eMultipleScattering,       -1, 1, 1 );
            pmanager->AddProcess( new G4eIonisation,               -1, 2, 2 );
            pmanager->AddProcess( new G4eBremsstrahlung,           -1, 3, 3 );
            if( fSRType ) {
                pmanager->AddProcess( new G4SynchrotronRadiation,      -1,-1, 4 );
            } else {
                pmanager->AddProcess( new G4SynchrotronRadiationInMat, -1,-1, 4 );
            }
            pmanager->AddProcess( new G4StepLimiter,               -1,-1, 5 );
        } else if( particleName == "e+" ) {
            //positron
            pmanager->AddProcess( new G4eMultipleScattering,       -1, 1, 1 );
            pmanager->AddProcess( new G4eIonisation,               -1, 2, 2 );
            pmanager->AddProcess( new G4eBremsstrahlung,           -1, 3, 3 );
            pmanager->AddProcess( new G4eplusAnnihilation,          0,-1, 4 );
            if( fSRType ) {
                pmanager->AddProcess( new G4SynchrotronRadiation,      -1,-1, 5 );
            } else {
                pmanager->AddProcess( new G4SynchrotronRadiationInMat, -1,-1, 5 );
            }
            pmanager->AddProcess( new G4StepLimiter,               -1,-1, 6 );
        } else if( particleName == "mu+" ||
                   particleName == "mu-"    ) {
          //muon
            pmanager->AddProcess( new G4MuMultipleScattering, -1, 1, 1 );
            pmanager->AddProcess( new G4MuIonisation,         -1, 2, 2 );
            pmanager->AddProcess( new G4MuBremsstrahlung,     -1, 3, 3 );
            pmanager->AddProcess( new G4MuPairProduction,     -1, 4, 4 );
        }
    }
    # else
    _TODO_  // TODO
    # endif
}

# if 0
# include "G4Decay.hh"
# endif

void
DynamicPhysicsList::ConstructGeneral() {
    # if 0
    // Add Decay Process
    G4Decay* theDecayProcess = new G4Decay();
    theParticleIterator->reset();
    while ((*theParticleIterator)()){
        G4ParticleDefinition* particle = theParticleIterator->value();
        G4ProcessManager* pmanager = particle->GetProcessManager();
        if( theDecayProcess->IsApplicable(*particle) ) {
            pmanager ->AddProcess(theDecayProcess);
            // set ordering for PostStepDoIt and AtRestDoIt
            pmanager->SetProcessOrdering(theDecayProcess, idxPostStep);
            pmanager->SetProcessOrdering(theDecayProcess, idxAtRest);
        }
    }
    # else
    _TODO_  // TODO
    # endif
}

void
DynamicPhysicsList::SetCuts() {
    # if 0
    if( verboseLevel > 0 ){
        G4cout << "CutLength : " << G4BestUnit(defaultCutValue,"Length") << G4endl;
    }

    // set cut values for gamma at first and for e- second and next for e+,
    // because some processes for e+/e- need cut values for gamma
    SetCutValue(defaultCutValue, "gamma");
    SetCutValue(defaultCutValue, "e-");
    SetCutValue(defaultCutValue, "e+");
    if( verboseLevel > 0 ) {
        DumpCutValuesTable();
    }
    # else
    _TODO_  // TODO
    # endif
}
# endif  // GEANT4_DYNAMIC_PHYSICS }}}

//
// ModularPhysicsList
// {{{
# ifdef GEANT4_DYNAMIC_PHYSICS

int ModularPhysicsList::physicsVerbosity = 0;

# define implement_construction_callback( physicsName ) \
G4VPhysicsConstructor *                                 \
_static_construct_ ## physicsName () {      \
    return ModularPhysicsList::construct_physics<physicsName>();            \
}
for_each_physics( implement_construction_callback )
# undef implement_construction_callback

void __static_fill_physics_modules_dictionary() {
    ModularPhysicsList::_phListCtrsDict =
            new std::unordered_map<std::string, G4VPhysicsConstructor * (*)()>();
    # define add_construction_callback( physicsName ) \
    ModularPhysicsList::push_g4_physics_module( # physicsName , _static_construct_ ## physicsName );
    ModularPhysicsList::for_each_physics( add_construction_callback )
    # undef add_construction_callback
}

void
ModularPhysicsList::register_modules_at( const NamesList & names,
                                         G4VModularPhysicsList * self ) {
    if( names.empty() ) {
        emraise( badState, "register_modules_at() invoked without any "
                "names provided." );
    }
    if( !self ) {
        emraise( badState,
                 "register_modules_at() invoked against nullptr arg." );
    }
    for( auto it = names.cbegin(); names.cend() != it; ++it ) {
        auto pIt = _phListCtrsDict->find( *it );
        if( _phListCtrsDict->end() == pIt ) {
            emraise( noSuchKey, "Has no physics named \"%s\" in current "
                     "build.", (*it).c_str() )
        }
        sV_log3( "Registering physics module \""
                 ESC_CLRGREEN "%s" ESC_CLRCLEAR "\".\n", pIt->first.c_str() );
        self->RegisterPhysics( pIt->second() );
    }
}

ModularPhysicsList::ModularPhysicsList( const ModularPhysicsList::NamesList & names ) {
    register_modules_at( names, this );
}

std::vector<std::string>
ModularPhysicsList::available_physics_modules() {
    std::vector<std::string> res;
    for( auto it  = _phListCtrsDict->cbegin();
              it != _phListCtrsDict->cend(); ++it ) {
        res.push_back( it->first );
    }
    return res;
}

ModularPhysicsList::~ModularPhysicsList() {
}

void
ModularPhysicsList::push_g4_physics_module(
            const std::string & name,
            G4VPhysicsConstructor * (*f)() ) {
    auto insertionResult = _phListCtrsDict->emplace( name , f );
    if( !insertionResult.second ) {
        if( insertionResult.first->second != f ) {
            sV_loge( "Failed to register physics module "
                     "\"%s\" because this name isn't unique.\n",
                     name.c_str() );
        } else {
            sV_log3( "Omitting repeatative registering of Geant4 physics "
                     "module constructor \"%s\" (%p).", name.c_str(), f );
        }
    }
}

void
ModularPhysicsList::SetCuts() {
    G4VUserPhysicsList::SetCuts();
}

# endif  // }}} GEANT4_DYNAMIC_PHYSICS (ModularPhysicsList implementation)

}  // namespace sV

# endif  // GEANT4_MC_MODEL

