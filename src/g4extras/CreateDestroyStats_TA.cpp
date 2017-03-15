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
# include "g4extras/CreateDestroyStats_TA.hpp"

# include "app/app.h"
# include <goo_exception.hpp>

# include <Geant4/G4String.hh>
# include <Geant4/G4VProcess.hh>
# include <iostream>
# include <TTree.h>

namespace sV {

// ParticleStatisticsDictionary
//////////////////////////////

ParticleStatisticsDictionary * ParticleStatisticsDictionary::_self = nullptr;

ParticleStatisticsDictionary &
ParticleStatisticsDictionary::self() {
    if( !_self ) {
        _self = new ParticleStatisticsDictionary();
    }
    return *_self;
}

void
ParticleStatisticsDictionary::add_selector_ctr( const std::string & name,
                    ParticleStatisticsDictionary::SelectorConstructor ctr ) {
    auto ir = _selectors.emplace( name, ctr );
    if( !ir.second ) {
        if( ir.first->second != ctr ) {
            emraise( nonUniq, "Attempt to insert particle selector with "
                "name \"%s\" by addr %p denied because name is already "
                "reserved by selector %p. Unable to insert "
                "selector constructor.", name.c_str(), ir.first->second, ctr );
        } else {
            sV_logw( "Repeatative insertion of selector ctr \"%s\" (%p) "
                "denied.", name.c_str(), ir.first->second );
        }
    }
}

void
ParticleStatisticsDictionary::add_statistics_ctr( const std::string & name,
                    ParticleStatisticsDictionary::StatisticsConstructor ctr ) {
    auto ir = _stats.emplace( name, ctr );
    if( !ir.second ) {
        if( ir.first->second != ctr ) {
            emraise( nonUniq, "Attempt to insert particle create/destroy "
                "statistics with name \"%s\" by addr %p denied because name "
                "is already reserved by ctr %p. Unable to insert "
                "create/destroy statistics constructor.",
                name.c_str(), ir.first->second, ctr );
        } else {
            sV_logw( "Repeatative insertion of create/destroy statistics ctr "
                "\"%s\" (%p) denied.", name.c_str(), ir.first->second );
        }
    }
}

ParticleStatisticsDictionary::iTrackSelector *
ParticleStatisticsDictionary::new_selector( const std::string & name ) {
    auto it = _selectors.find(name);
    if( _selectors.end() == it ) {
        std::cerr << "HINT: available selectors ctrs:" << std::endl;
        for( auto p : _selectors ) {
            std::cerr << "HINT:    - " << p.first << std::endl;
        }
        emraise( notFound, "Unknown selector constructor name \"%s\".",
                name.c_str() );
    }
    auto iptr = it->second();
    _allocatedSelectors.insert(iptr);
    return iptr;
}

ParticleStatisticsDictionary::iTrackingStatistics *
ParticleStatisticsDictionary::new_statistics( const std::string & name ) {
    auto it = _stats.find(name);
    if( _stats.end() == it ) {
        std::cerr << "HINT: available create/destroy stats ctrs:" << std::endl;
        for( auto p : _selectors ) {
            std::cerr << "HINT:    - " << p.first << std::endl;
        }
        emraise( notFound, "Unknown create/destroy constructor name \"%s\".",
                name.c_str() );
    }
    auto iptr = it->second();
    _allocatedStatistics.insert(iptr);
    return iptr;
}

void
ParticleStatisticsDictionary::free_selector(
                ParticleStatisticsDictionary::iTrackSelector * selectorPtr ) {
    _allocatedSelectors.erase( selectorPtr );
}

void
ParticleStatisticsDictionary::free_statistics(
            ParticleStatisticsDictionary::iTrackingStatistics * statsPtr ) {
    _allocatedStatistics.erase( statsPtr );
}

void
ParticleStatisticsDictionary::finalize() {
    if( !_self )
        return;  // do nothing as dictionary wasn't even used once :(
    if( !self()._allocatedSelectors.empty() ) {
        sV_logw( "%d particle selector instances are still not freed."
            "Deleting them now:\n" );
        for( auto sPtr : self()._allocatedSelectors ) {
            sV_logw( "    - %s\n", sPtr->name().c_str() );
            delete sPtr;
        }
    }
    if( !self()._allocatedStatistics.empty() ) {
        sV_logw( "%d create/destroy statistics instances are still not freed."
            "Possible loss of data. Will delete them now.\n" );
        for( auto sPtr : self()._allocatedStatistics ) {
            delete sPtr;
        }
    }
    delete _self;
    _self = nullptr;
}

__attribute__((destructor))
static void _static_finalize_ParticleStatisticsDictionary() {
    ParticleStatisticsDictionary::finalize();
}

// CreateDestroyStats_TA
///////////////////////

CreateDestroyStats_TA::CreateDestroyStats_TA() {
}

CreateDestroyStats_TA::~CreateDestroyStats_TA() {
}

void
CreateDestroyStats_TA::_fill_if_need( StatsObjectIndex & m,
        const G4Track * track ) {
    for( auto pair : m ) {
        const ParticleStatisticsDictionary::iTrackSelector & selector = *(pair.first);
        ParticleStatisticsDictionary::iTrackingStatistics & stat = *(pair.second);
        if( selector.matches(track) ) {
            stat.fill_track( selector, track );
        }
    }
}

void
CreateDestroyStats_TA::PreUserTrackingAction(const G4Track * track) {
    // (G4 API) Action to be performed before a new track is created.
    // Purpose and Method: this method is called every time a new track is
    // created during the Geant4 event simulation.
    // Inputs: the G4Track pointer aTrack that gives access to the actual
    // created track object.
    _fill_if_need( _ssOnCreate, track );
    G4UserTrackingAction::PreUserTrackingAction( track );
}

void
CreateDestroyStats_TA::PostUserTrackingAction(const G4Track * track) {
    // (G4 API) Action to be performed after a track is dead.
    // Purpose and Method: this method is called every time a track is
    // destroied during the Geant4 event simulation.
    // Inputs: the G4Track pointer aTrack that gives access to the actual track
    // object.
    _fill_if_need( _ssOnDestroy, track );
    G4UserTrackingAction::PostUserTrackingAction( track );
}

void
CreateDestroyStats_TA::choose_tracks_on_create(
            const ParticleStatisticsDictionary::iTrackSelector * selectorPtr,
            ParticleStatisticsDictionary::iTrackingStatistics * statsPtr ) {
    _ssOnCreate.emplace( selectorPtr, statsPtr );
}

void
CreateDestroyStats_TA::choose_tracks_on_destroy(
            const ParticleStatisticsDictionary::iTrackSelector * selectorPtr,
            ParticleStatisticsDictionary::iTrackingStatistics * statsPtr ) {
    _ssOnDestroy.emplace( selectorPtr, statsPtr );
}


// ParticleBornKinematics
////////////////////////

void
ParticleBornKinematics::_V_fill_track( const sV::ParticleStatisticsDictionary::iTrackSelector & /*selector*/,
                    const G4Track * track ) {
    G4ThreeVector vertexMomentumDirection = track->GetVertexMomentumDirection();
    G4ThreeVector vertexPosition = track->GetVertexPosition();
    _vertex.kineticEnergy = track->GetVertexKineticEnergy();
    _vertex.totalEnergy = track->GetTotalEnergy();

    _vertex.posX = vertexPosition[0];
    _vertex.posY = vertexPosition[1];
    _vertex.posZ = vertexPosition[2];
    _vertex.momentumX = vertexMomentumDirection[0];
    _vertex.momentumY = vertexMomentumDirection[1];
    _vertex.momentumZ = vertexMomentumDirection[2];

    _aprimeInfo->Fill();
}

ParticleBornKinematics::ParticleBornKinematics() : _aprimeInfo(nullptr) {
    _aprimeInfo = new TTree( "aprimeInfo",
                             "Info about A' production reaction");
    _aprimeInfo->Branch( "bornVertex", &_vertex,
        "posX/D:posY/D:posZ/D:momentumX/D"
        ":momentumY/D:momentumZ/D"
        ":totalEnergy/D:kineticEnergy/D" );
}

static sV::ParticleStatisticsDictionary::iTrackingStatistics *
_static_new_APrime_TA_statistics() {
    return new ParticleBornKinematics();
}

__attribute__((constructor(156)))
static void _static_add_ParticleBornKinematics_stats() {
    ParticleStatisticsDictionary::self().add_statistics_ctr(
            "bornKinematics", _static_new_APrime_TA_statistics );
}

}  // namespace sV


