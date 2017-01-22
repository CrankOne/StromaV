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
# include "actions/EventAction.hpp"
# include "app/mixins/protobuf.hpp"

namespace svmc {

EventAction::EventAction( std::ostream & fileRef) :
    PlainStreamBucketDispatcher(
        fileRef,
        (uint8_t)goo::app<sV::AbstractApplication>().cfg_option<int>
        ("b-dispatcher.maxBucketSize.KB"),
        (uint8_t)goo::app<sV::AbstractApplication>().cfg_option<int>
        ("b-dispatcher.maxBucketSize.events")
        ) {

    //_fileRef.open(goo::app<sV::AbstractApplication>().cfg_option<std::string>
    //    ("b-dispatcher.outFile"), std::ios::out | std::ios::binary |
    //                              std::ios::app );
    //_streamRef(_fileRef);
}

EventAction::~EventAction() {
    //_fileRef.close();
}

void EventAction::EventAction::BeginOfEventAction(const G4Event* ) {
    // sV::mixins::PBEventApp::c_event().mutable_simulated();
}

//  manage Buckets with UEvents
void EventAction::EndOfEventAction(const G4Event* ) {
    # ifdef RPC_PROTOCOLS
    push_event(sV::mixins::PBEventApp::c_event());
    sV::mixins::PBEventApp::c_event().mutable_simulated()->Clear();
    # endif
}

}  // namespace svmc

