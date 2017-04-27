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
# include "actions/TrackingAction.hpp"

# include <G4String.hh>
# include <G4VProcess.hh>
# include <iostream>

namespace svmc {

TrackingAction::TrackingAction() {
    aprimeVertex.posX=0;
    aprimeVertex.posY=0;
    aprimeVertex.posZ=0;
    aprimeVertex.momentumX=0;
    aprimeVertex.momentumY=0;
    aprimeVertex.momentumZ=0;
    aprimeVertex.totalEnergy=0;
    aprimeVertex.kineticEnergy=0;
    //  aprimeVertex.trackStatus=fAlive;

    aprimeInfo->Branch("aprimeVertex", &aprimeVertex,
            "posX/D:posY/D:posZ/D:momentumX/D:momentumY/D:momentumZ/D:totalEnergy/D:kineticEnergy");
}

TrackingAction::~TrackingAction() {
    aprimeInfo->Write();
    //aprimeInfo->Print();
}

//void TrackingAction::PreUserTrackingAction(const G4Track * track) {
    /*
    if ( track->GetTrackID() == 1) {
        double globalTime = track->GetGlobalTime();
        double totalEnergy = track->GetTotalEnergy();
        G4ThreeVector momentum = track->GetMomentumDirection();
        G4TrackStatus trackStatus = track->GetTrackStatus();
        std::cout << "=========================================="
            << std::endl
            << "PreUser stage. ID of the track is 1" << std::endl
            << "Global time ----- " << globalTime << std::endl
            << "Total energy ----"  << totalEnergy << std::endl
            << "momentum --------"  << momentum[0] << ", "
                                    << momentum[1] << ", "
                                    << momentum[2] << ", " << std::endl
            << "track status: " << trackStatus << std::endl
            << "=========================================="
            << std::endl;
    }
    */
//}

void TrackingAction::PostUserTrackingAction(const G4Track * track) {
    if (track->GetCreatorProcess() &&
        track->GetCreatorProcess()->GetProcessName() == "A'-e.m.mixing" ) {
        G4ThreeVector vertexMomentumDirection = track->GetVertexMomentumDirection();
        G4ThreeVector vertexPosition = track->GetVertexPosition();
        aprimeVertex.kineticEnergy = track->GetVertexKineticEnergy();
        aprimeVertex.totalEnergy = track->GetTotalEnergy();

        aprimeVertex.posX = vertexPosition[0];
        aprimeVertex.posY = vertexPosition[1];
        aprimeVertex.posZ = vertexPosition[2];
        aprimeVertex.momentumX = vertexMomentumDirection[0];
        aprimeVertex.momentumY = vertexMomentumDirection[1];
        aprimeVertex.momentumZ = vertexMomentumDirection[2];

        aprimeInfo->Fill();
    }
    /*
     * if (track->GetCreatorProcess() &&
             track->GetCreatorProcess()->GetProcessName() != "annihil" &&
             track->GetCreatorProcess()->GetProcessName() != "phot" &&
             track->GetCreatorProcess()->GetProcessName() != "eBrem" &&
             track->GetCreatorProcess()->GetProcessName() != "compt" &&
             track->GetCreatorProcess()->GetProcessName() != "conv" &&
             track->GetCreatorProcess()->GetProcessName() != "eIoni"
        ) {
        G4String creatorProcessName = track->GetCreatorProcess()->GetProcessName();
        std::cout << "creator process name " << creatorProcessName << std::endl;
    }
    */
    /*if ( track->GetTrackID() == 1) {
        double globalTime = track->GetGlobalTime();
        double totalEnergy = track->GetTotalEnergy();
        G4ThreeVector momentum = track->GetMomentumDirection();
        G4TrackStatus trackStatus = track->GetTrackStatus();
        std::cout << "=========================================="
            << std::endl
            << "PostUser stage. ID of the track is 1" << std::endl
            << "Global time ----- " << globalTime << std::endl
            << "Total energy ----"  << totalEnergy << std::endl
            << "momentum --------"  << momentum[0] << ", "
                                    << momentum[1] << ", "
                                    << momentum[2] << ", " << std::endl
            << "track status: " << trackStatus << std::endl
            << "=========================================="
            << std::endl;
    }*/
    /*
    if (track->GetCreatorProcess() &&
        track->GetCreatorProcess()->GetProcessName() == "A'-e.m.mixing" ) {
        G4String creatorProcessName = track->GetCreatorProcess()->GetProcessName();
        G4ThreeVector vertexMomentumDirection = track->GetVertexMomentumDirection();
        G4ThreeVector vertexPosition = track->GetVertexPosition();
        double vertexEnergy = track->GetVertexKineticEnergy();
        double vertexTotEnergy = track->GetTotalEnergy();
        G4TrackStatus trackStatus = track->GetTrackStatus();

        std::cout << "creator process name " << creatorProcessName << std::endl;
        std::cout << "vertex momentum direction: " << vertexMomentumDirection[0] << ", "
            << vertexMomentumDirection[1] << ", "
            << vertexMomentumDirection[2] << std::endl <<
            "track status: " << trackStatus << std::endl <<
            "vertexPosition: " << vertexPosition[0] << ", "
            << vertexPosition[1] << ", "
            << vertexPosition[2] << std::endl <<
            "vertex kinetic energy: " << vertexEnergy << std::endl
            << "total energy: " << vertexTotEnergy << std::endl;
        //if (creatorProcessName!="")
    }
    */
}

}  // namespace svmc

