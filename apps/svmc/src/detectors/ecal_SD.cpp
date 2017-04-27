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

# ifndef H_SVMC_DETECTORS_ECAL_SENSITIVE_DETECTOR_H
# define H_SVMC_DETECTORS_ECAL_SENSITIVE_DETECTOR_H

# include "afNA64_config.h"
# include "nsp_scorer.tcc"

# include "app/app.h"
# include <G4VSensitiveDetector.hh>
# include <G4LogicalVolumeStore.hh>
# include "ext.gdml/SensDetDict.hpp"
//# include "g4_DC.hh"
# include "app_g4simulation.hpp"
# include "detector_ids.h"

# include <TH2F.h>
# include <TFile.h>
# include <TTree.h>

# ifdef StromaV_RPC_PROTOCOLS
# include "na64_uevent.hpp"
# include "app/mixins/protobuf.hpp"
# endif  // StromaV_RPC_PROTOCOLS

/**@file ecal_SD.cpp
 * @brief A generic-purpose ECAL information writer.
 *
 * Collects an entire energy deposition in ECAL during  a run. As a direct
 * summation causes rounding error, we should accumulate depositions
 * by themselves.
 */

namespace sV {

//
// Sensitive detector descendant
//

class ECAL_cell : public G4VSensitiveDetector {
public:
    typedef aux::NotSoPrimitiveScorer<> Scorer;
    typedef uint32_t DetectorUniqueID;

    enum FeatureFlags : uint16_t {
        noFeatures          = 0x0,
        segmentated         = 0x1,
        treeStoresEdep      = 0x2,
        treeStoresResponse  = 0x4,
        treeStoresTime      = 0x8,
        // ...
    };

    /// DetectorID is composed from replica number stack.
    //typedef uint64_t DetectorID;
    union DetectorID {
        struct {
            uint16_t det_No;
            uint8_t cell_No,
                    layerX_No,
                    layer_No
                    //ECAL_No = 0,
                    //entireEcal_No = 0
                    ;
        } byFields;
        DetectorUniqueID wholenum;
        DetectorID() { bzero(this, sizeof(DetectorID)); }
        DetectorID( DetectorUniqueID v ) : wholenum(v) {}
        operator DetectorUniqueID() const { return wholenum; }

        /// op-r to use in map (not in unordered_map!)
        bool operator<(const DetectorID & right) const {
            return (wholenum < right.wholenum);
        }
    };
    struct PMTStatistics {  // represents an event
        Scorer * _response,
               * _edep
               // ... other scorers ?
                ;
        //TH2F * _edepVStime;
        // ... histograms, etc
    };
private:
    bool _boundsSet;  // After all geometry is created...
    uint16_t _features;
    uint64_t _nCalls,
             _nEvents;
    std::unordered_map<DetectorUniqueID, PMTStatistics *> _pmtStatsPerEvent;
    G4LogicalVolume * _ecalLVPtr,
                    * _preshowerLVPtr
                    ;

    # ifdef StromaV_RPC_PROTOCOLS
    na64::events::SimulatedEvent_Payload _lastEvent_Payload;
    # endif  // StromaV_RPC_PROTOCOLS

    void _allocate_data_tree_for_segmentated_version();
protected:
    DECLTYPE(_pmtStatsPerEvent)::iterator _get_stats_it( DetectorUniqueID );
    static PMTStatistics * _new_pmt_stats( uint32_t, uint32_t, uint32_t );
public:
    ECAL_cell( const std::string & );
    virtual ~ECAL_cell() {}
public:
    // G4 interface override
    virtual void Initialize( G4HCofThisEvent * ) override;
    virtual G4bool ProcessHits( G4Step * aStep,
                                G4TouchableHistory * ROhist ) override;
    virtual void EndOfEvent(G4HCofThisEvent *) override;
    virtual void clear() override;

    // Saves data whenever it need and invokes clear.
    virtual void fill_event();
};  // class ECAL_cell



//
// IMPLEMENTATION
////////////////

ECAL_cell::ECAL_cell( const std::string & name ) :
        G4VSensitiveDetector( name ),
        _boundsSet(false),
        _features( treeStoresEdep | treeStoresTime | treeStoresResponse )  // TODO: configurable?
        /*_tree(nullptr)*/ {
    _nEvents = _nCalls = 0;

    std::string detID = name.substr( name.rfind( "/" ) + 1, std::string::npos );
    if( "ECAL_segm" == detID ) {
        _allocate_data_tree_for_segmentated_version();
        _features |= segmentated;
        sV_log3( ESC_CLRCYAN "ECAL_cell" ESC_CLRCLEAR
                     " %p:" ESC_CLRGREEN "%s" ESC_CLRCLEAR
                     " sensDet constructed in segmentated layer variant.\n",
                     this, name.c_str() );
    } else if( "ECAL_nosegm" == detID ) {
        _TODO_  // TODO
        sV_log3( ESC_CLRCYAN "ECAL_cell" ESC_CLRCLEAR
                     " %p:" ESC_CLRGREEN "%s" ESC_CLRCLEAR
                     " constructed in monolithic layer variant.\n",
                     this, name.c_str() );
    } else {
        emraise( malformedArguments,
            "Please, specify \"ECAL_segm\" or \"ECAL_nosegm\" sensDet for ECAL's cell." );
    }
}

ECAL_cell::PMTStatistics *
ECAL_cell::_new_pmt_stats( uint32_t detNo, uint32_t layerXNo, uint32_t cellNo) {
    auto res = new PMTStatistics;
    uint32_t poolNNodes = goo::app<sV::AbstractApplication>().cfg_option<int32_t>("g4-SD-ECAL_cell.scorerPool-NCells");
    res->_response = new Scorer( poolNNodes );
    res->_edep = new Scorer( poolNNodes );
    if( goo::app<sV::AbstractApplication>().cfg_option<bool>("g4-SD-ECAL_cell.timeVSedepHisto")) {
        char nameBF[64], labelBF[128];
        snprintf( nameBF, sizeof(nameBF), "eVSt:%x%x%x;singleEvent",
                    detNo, layerXNo, cellNo );
        snprintf( labelBF, sizeof(labelBF), "E/t at %x/%x/%x;singleEvent",
                    detNo, layerXNo, cellNo );
        //res->_edepVStime = new TH2F(
        //        nameBF, labelBF,
        //        goo::app<sV::AbstractApplication>().cfg_option<int>("g4-SD-ECAL_cell.timeVSedep-edepNBins"),
        //        0., goo::app<sV::AbstractApplication>().cfg_option<double>("g4-SD-ECAL_cell.timeVSedepMaxEDep-MeV"),
        //        goo::app<sV::AbstractApplication>().cfg_option<int>("g4-SD-ECAL_cell.timeVSedep-timeNBins"),
        //        0., goo::app<sV::AbstractApplication>().cfg_option<double>("g4-SD-ECAL_cell.timeVSedepMaxTime-ns")
        //    );
    } else {
        //res->_edepVStime = nullptr;
    }
    return res;
}

//
// Interface
//

void
ECAL_cell::Initialize(G4HCofThisEvent* /*HCE*/) {
    if( !_boundsSet ) {
        // obtain the ECAL and preshower volumes ptr
        {
            G4LogicalVolumeStore & store = *G4LogicalVolumeStore::GetInstance();
            _ecalLVPtr = store.GetVolume( "ECAL" ),
            _preshowerLVPtr = store.GetVolume( "preshower" );
            if( !_ecalLVPtr || !_preshowerLVPtr ){
                emraise( notFound,
                         "Using of ECAL_cell sensDet requires there should be "
                         "\"preshower\" and \"ECAL\" logical volumes defined on scene." );
            }
        }
        sV_log2( "ECAL_cell %p initialized. ECAL volume: %p, preshower: %p.\n",
                this, _ecalLVPtr, _preshowerLVPtr );
        # if 0
        if( !gFile ) {
            _tree = nullptr;
        } else {
            if( _features & segmentated ) {
                _allocate_data_tree_for_segmentated_version();
            } else {
                _TODO_  // TODO
            }
        }
        # endif
        _boundsSet = true;
    }
}

G4bool
ECAL_cell::ProcessHits( G4Step * aStep,
                        G4TouchableHistory * /*ROhist*/ ) {
    _nCalls++;
    G4double hitEdep = aStep->GetTotalEnergyDeposit();
    if( !hitEdep ) {
        return false;  // Note: ignore hits without actual energy deposition!
    }
    G4StepPoint* preStep = aStep->GetPreStepPoint();
    const G4VTouchable * touchable = preStep->GetTouchable();

    DetectorID dID;
    {
        G4LogicalVolume * volptr = touchable->GetVolume(4)->GetLogicalVolume();
        if( volptr == _ecalLVPtr ) {
            dID.byFields.det_No = EnumScope::d_ECAL1;
        } else if( volptr == _preshowerLVPtr ) {
            dID.byFields.det_No = EnumScope::d_ECAL0;
        } else {
            sV_logw( "Expected ECAL/preshower volume; got %s at %p instead on 4 replica number depth.\n",
                         touchable->GetVolume(4)->GetName().c_str(), volptr );
        }
    }
    dID.byFields.cell_No = touchable->GetReplicaNumber(1);
    dID.byFields.layerX_No = touchable->GetReplicaNumber(2);
    //dID.byFields.layer_No = touchable->GetReplicaNumber(3);  // exclude cell #
    assert( !(touchable->GetReplicaNumber(0) ||  // assure, it's really segmentated ECAL!
              touchable->GetReplicaNumber(4) ||
              touchable->GetReplicaNumber(5)) );

    auto it = _pmtStatsPerEvent.find( dID.wholenum );
    if( _pmtStatsPerEvent.end() == it ) {
        auto rp = _pmtStatsPerEvent.emplace( dID.wholenum, _new_pmt_stats(
                    (int) dID.byFields.det_No,
                    (int) dID.byFields.layerX_No,
                    (int) dID.byFields.cell_No
            ) );
        it = rp.first;
        if( !rp.second ) {
            emraise( badArchitect, "Stats insertion failed." );
        } else {
            sV_log3( "Allocated stats for %u (%x/%x/%x) PMT.\n",
                        dID.wholenum,
                        (int) dID.byFields.det_No,
                        (int) dID.byFields.layerX_No,
                        (int) dID.byFields.cell_No
                    );
        }
    }

    assert( it->second );
    PMTStatistics & cStats = *(it->second);
    {
        G4double stepLength = aStep->GetStepLength(),
                 charge = aStep->GetTrack()->GetDefinition()->GetPDGCharge();
        cStats._edep->push_value( hitEdep );
        //if( cStats._edepVStime ) {
        //    cStats._edepVStime->Fill( hitEdep, preStep->GetGlobalTime() );
        //}
        # if 1
        G4double birk1 = 0.126*CLHEP::mm/CLHEP::MeV;  // ref: MK
        # else
        G4double birk1 = aStep->GetTrack()
                        ->GetMaterial()
                        ->GetIonisation()
                        ->GetBirksConstant();
        //0.126*mm/MeV --- todo: why it returns zero?
        # endif
        if( birk1*hitEdep*stepLength*charge ) {
            G4double hitResp = hitEdep/(1. + birk1*hitEdep/stepLength);
            cStats._response->push_value( hitResp );
        }
        // track->GetKineticEnergy() ?
    }



    return true;
}

void
ECAL_cell::EndOfEvent(G4HCofThisEvent *) {
    # if 0
    for( auto it  = _pmtStatsPerEvent.begin();
              it != _pmtStatsPerEvent.end(); ++it) {
        std::cout << "PMT #" << std::hex << (int) DetectorID(it->first).byFields.det_No << "/"
                                         << (int) DetectorID(it->first).byFields.cell_No << "/"
                                         << (int) DetectorID(it->first).byFields.layerX_No << "/"
                                         << (int) DetectorID(it->first).byFields.layer_No << ": ";
        std::cout << it->second->_response->sum() << " / "
                  << it->second->_edep->sum() << " = "
                  << it->second->_response->sum() / it->second->_edep->sum()
                  << std::endl;
        std::cout << "Detector id: " << std::hex << compose_cell_identifier(
                (AFR_DetMjNo) DetectorID(it->first).byFields.det_No,
                (uint8_t) DetectorID(it->first).byFields.layerX_No,
                (uint8_t) DetectorID(it->first).byFields.cell_No ) << std::endl;
    }
    # endif
    //cStats._edepVStime->Write(); //

    fill_event();
    sV_log3( "ECAL_cell hits processing invoked %lu times on event #%lu.\n", _nCalls, _nEvents );
    ++_nEvents;
    clear();
}

void
ECAL_cell::clear() {
    for( auto it  = _pmtStatsPerEvent.begin();
              it != _pmtStatsPerEvent.end(); ++it) {
        it->second->_response->reset();
        it->second->_edep->reset();
    }
    //if( _features & segmentated ) {
    //    bzero( _lastEvent.edep, sizeof(_lastEvent.edep) );
    //    bzero( _lastEvent.edep, sizeof(_lastEvent.resp) );
    //} else {
    //    _TODO_  // TODO
    //}
    _lastEvent_Payload.Clear();
    _nCalls = 0;
}

//
// Own subroutines
//

void
ECAL_cell::_allocate_data_tree_for_segmentated_version() {
    //_tree = new TTree( "ECALStats",
    //                   "ECAL statistics" );
    //_edepBranch = _features & treeStoresEdep     ?
    //        _tree->Branch( "edep", _lastEvent.edep, "edep[72]/D" ) : nullptr;
    //_respBranch = _features & treeStoresResponse ?
    //        _tree->Branch( "resp", _lastEvent.resp, "resp[72]/D" ) : nullptr;
    //sV_logw( "Branch addresses set.\n" );
    // ...
}

void
ECAL_cell::fill_event() {
# if 0
    if( _features & segmentated ) {
        for( auto it  = _pmtStatsPerEvent.begin();
                  it != _pmtStatsPerEvent.end(); ++it) {
            DetectorID dID(it->first);
            if( _features & (treeStoresEdep | treeStoresResponse) ) {
                //uint8_t idx = ( g4sim::EnumScope::ECAL_preshowerPart == dID.byFields.det_No ? 0 : 32) +
                //              dID.byFields.cell_No*6 + dID.byFields.layerX_No
                //              ;
                //TODO:
                //auto pmtStatsPtr = ::g4sim::Application::c_event().add_pmt_stats();
                //pmtStatsPtr->set_detector_id( dID.wholenum );
                //pmtStatsPtr->set_edep( it->second->_edep->sum() );
                //if( _features & treeStoresResponse ) {
                //    pmtStatsPtr->set_resp( it->second->_response->sum() );
                //}
            }
        }
    } else {
        _TODO_  // TODO
    }
# endif
    for( auto it  = _pmtStatsPerEvent.begin();
                  it != _pmtStatsPerEvent.end(); ++it) {
        # ifdef StromaV_RPC_PROTOCOLS
        na64::events::PMTStatistics* gpbPMTStatistics =
                                     _lastEvent_Payload.add_pmt_stats();
        gpbPMTStatistics->set_resp( it->second->_response->sum() );
        gpbPMTStatistics->set_detector_id( compose_cell_identifier(
                    (AFR_DetMjNo) DetectorID(it->first).byFields.det_No,
                    (uint8_t) DetectorID(it->first).byFields.layerX_No,
                    (uint8_t) DetectorID(it->first).byFields.cell_No ) );
        gpbPMTStatistics->set_edep( it->second->_edep->sum() );
        // XXX
        // std::cout << gpbPMTStatistics->DebugString() << std::endl;
        # endif  // StromaV_RPC_PROTOCOL
    }
    # ifdef StromaV_RPC_PROTOCOLS
    sV::mixins::PBEventApp::c_event()
                        .mutable_simulated()
                        ->mutable_payload()
                        ->PackFrom(_lastEvent_Payload);
    # endif  // StromaV_RPC_PROTOCOL
}
//
// REGISTER SENSITIVE DETECTOR
//

extGDML_G4_REGISTER_SD( ECAL_cell )

}  // namespace sV

# endif  // H_SVMC_DETECTORS_ECAL_SENSITIVE_DETECTOR_H

