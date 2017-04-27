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

# ifndef H_SV_G4_TRACKING_ACTION_H
# define H_SV_G4_TRACKING_ACTION_H

//  # include "g4extras/DetectorConstruction.hpp"

# include <G4UserTrackingAction.hh>
# include <G4Track.hh>
# include <G4TrackStatus.hh>
# include <TTree.h>

# include <iostream>

namespace svmc {

class TrackingAction :
    public G4UserTrackingAction {
    public:
        TrackingAction ();
        ~TrackingAction ();

        virtual void  PreUserTrackingAction(const G4Track*) override {}
        virtual void PostUserTrackingAction(const G4Track*) override;

    protected:
    private:
        /*  This tree store information about particle
         *  parameters in Aprime generation reaction
         */
        TTree * aprimeInfo = new TTree("aprimeInfo", "Info about A' production reaction");

        struct vertex {
            double posX,
                   posY,
                   posZ;
            double momentumX,
                   momentumY,
                   momentumZ;
            double totalEnergy;
            double kineticEnergy;
            //  G4TrackStatus trackStatus;
        }aprimeVertex;
};  // class UserTrackingAction


}  // namespace svmc

# endif  // H_SV_G4_TRACKING_ACTION_H

