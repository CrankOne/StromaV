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

# include "app/mixins/protobuf.hpp"

# ifdef RPC_PROTOCOLS

# include "event.pb.h"

namespace sV {
namespace mixins {

PBEventApp * PBEventApp::_self_PBEventAppPtr = nullptr;

::sV::events::Event &
PBEventApp::c_event() {
    if( !PBEventApp::_self_PBEventAppPtr ) {
        emraise( badState, "No PBEventApp singleton-instance initialized still." );
    }
    PBEventApp & appInst = *PBEventApp::_self_PBEventAppPtr;
    if( !appInst._cEvent ) { appInst._cEvent = new ::sV::events::Event(); }
    return *appInst._cEvent;
}

PBEventApp::PBEventApp( AbstractApplication::Config * c ) :
        AbstractApplication(c),
        _cEvent(nullptr) {
    PBEventApp::_self_PBEventAppPtr = this;
}

}  // namespace mixins
}  // namespace sV

# endif  // RPC_PROTOCOLS

