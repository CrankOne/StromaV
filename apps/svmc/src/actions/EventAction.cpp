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

# ifdef StromaV_RPC_PROTOCOLS

# include "app/mixins/protobuf.hpp"
# include <iostream>  // XXX

namespace svmc {

EventAction::EventAction( sV::iBucketDispatcher* bucketDispatcher ) {
    _bucketDispatcher = bucketDispatcher;
}

EventAction::~EventAction() {
    delete _bucketDispatcher;
}

void EventAction::EventAction::BeginOfEventAction(const G4Event* ) {
}

//  manage Buckets with UEvents
void EventAction::EndOfEventAction(const G4Event* ) {
    _bucketDispatcher->push_event(sV::mixins::PBEventApp::c_event());
    // std::cout << sV::mixins::PBEventApp::c_event().DebugString() << std::endl;  // XXX
    sV::mixins::PBEventApp::c_event().mutable_simulated()->Clear();
}

}  // namespace svmc
# endif  // StromaV_RPC_PROTOCOLS

