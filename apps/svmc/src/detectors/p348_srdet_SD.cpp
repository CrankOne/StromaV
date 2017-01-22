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

# ifndef p348_srdet_SD_h
# define p348_srdet_SD_h

# include "p348g4_config.h"
# include "nsp_scorer.tcc"
# include "g4extras/SensDetDict.hpp"

// # pragma GCC diagnostic push
// # pragma GCC diagnostic ignored "-Wdeprecated-register"  // "register" is depr-d
# include <G4VSensitiveDetector.hh>
# include <G4LogicalVolumeStore.hh>
// # pragma GCC diagnostic pop

# include <TH2F.h>
# include <TFile.h>
# include <TTree.h>

# if 0
# ifdef RPC_PROTOCOLS
# include "p348g4_uevent.hpp"
# endif  // RPC_PROTOCOLS
# endif


namespace p348 {

class test01_SD : public G4VSensitiveDetector {

    public :
        test01_SD( const std::string &);
        virtual ~test01_SD();

        // G4 interface override
        virtual void Initialize(G4HCofThisEvent*) override;
        virtual G4bool ProcessHits( G4Step * aStep,
                                    G4TouchableHistory * ROhist) override;
        virtual void EndOfEvent(G4HCofThisEvent*) override;
        virtual void clear() override;

        // Saves data whenever it need and invokes clear.
        // virtual void fill_event();

        typedef aux::NotSoPrimitiveScorer<> Scorer;
        // TODO typedef uint32_t DetectorUniqueID;

        struct PMTScorers {  //represents an event
            Scorer* _resp,
                  * _edep;
            // TH1F  * _resp;
            // TH1F  * _edep;

        } *_Stats;
        // TODO
        # if 0
        union DetectorID {
            struct {
                uint8_t det_No,
                        layerX_No,
                        layerY_No;
                        // another uint8_t?
            } byFields;
            DetectorUniqueID wholenum;
        }
        # endif
        TH1F * _total_edep;
        TH1F * _total_resp;

        private :
        bool _bound_set;
        G4LogicalVolume * _srdetLVPtr;
        // TODO std::unordered_map<DetectorUniqueID, PMTStatistics*> _pmtStatsPerDetector;

        // TODO void _allocate_data_tree_for_segmentated_version();
};  // class test01_SD

test01_SD::test01_SD( const std::string & name) :
                        G4VSensitiveDetector( name ),
                        _bound_set(false) {

    _total_edep       = new TH1F("total_edep", "Total Energy Deposition in SRdet", 100, 0, 100000);
    _total_resp       = new TH1F("total_resp", "Total Response in SRdet",          100, 0, 100000);
    _Stats            = new PMTScorers();
    _Stats->_resp     = new Scorer(10000);
    _Stats->_edep     = new Scorer(10000);

}

test01_SD::~test01_SD() {
    //_total_edep->Write();  // Already exists somewhere
    //_total_resp->Write();

    // TODO  Write files?
    // _total_edep->Close();
    // _total_resp->Close();
}

void test01_SD::Initialize(G4HCofThisEvent*) {
    if (!_bound_set) {
        G4LogicalVolumeStore & store = *G4LogicalVolumeStore::GetInstance();
        _srdetLVPtr = store.GetVolume("entire_srdet");  // TODO  ???
        _bound_set = true;
    }
    // PMTStatistics* Stats = new PMTStatistics;
}

G4bool test01_SD::ProcessHits( G4Step * aStep,
                               G4TouchableHistory*) {
    G4double hitEdep = aStep->GetTotalEnergyDeposit();
    if (!hitEdep) return false;

    // G4StepPoint* preStep = aStep->GetPreStepPoint();
    // const G4VTouchable * touchable = preStep->GetTouchable();

    _Stats->_edep->push_value( hitEdep );

    G4double stepLength = aStep->GetStepLength(),
             charge = aStep->GetTrack()->GetDefinition()->GetPDGCharge();
    G4double birk1 = 0.126*CLHEP::mm/CLHEP::MeV;  // ref: MK
    if( birk1*hitEdep*stepLength*charge) {
        G4double hitResp = hitEdep/(1. + birk1*hitEdep/stepLength);
        _Stats->_resp->push_value(hitResp);
        //_total_resp->Fill(hitResp);

    }

    //_total_edep->Fill(hitEdep);

    return true;
}

void test01_SD::EndOfEvent(G4HCofThisEvent *) {
    std::cout <<_Stats->_edep->sum()<< std::endl << _Stats->_resp->sum()<<std::endl;
    _total_edep->Fill(_Stats->_edep->sum());
    _total_resp->Fill(_Stats->_resp->sum());
    // fill_event();
    //_total_edep->Write();
    //_total_resp->Write();
    clear();
}

void test01_SD::clear() {
    _Stats->_edep->reset();
    _Stats->_resp->reset();
}

// Register SD
P348_G4_REGISTER_SD( test01_SD )

}        // namespace p348
# endif  // p348_srdet_SD_h

