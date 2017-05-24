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
# include "buckets/iDispatcher.hpp"

# ifdef RPC_PROTOCOLS

# include "app/mixins/protobuf.hpp"

# include "event.pb.h"

namespace sV {
namespace buckets {

iDispatcher::iDispatcher() :
                            _rawBucketPtr(
                                google::protobuf::Arena::CreateMessage<events::Bucket>(
                                            sV::mixins::PBEventApp::arena_ptr()) ) {}

iDispatcher::~iDispatcher() {
    if( !is_empty() ) {
        drop_bucket();
    }
}

size_t
iDispatcher::drop_bucket() {
    size_t ret = _V_drop_bucket();
    clear_bucket();
    return ret;
}

void
iDispatcher::clear_bucket() {
    _rawBucketPtr->Clear();
}

bool
iDispatcher::is_empty() {
    return !bucket().events_size();
}

void
iDispatcher::push_event(const events::Event & eve) {
    events::Event* event = bucket().add_events();
    event->CopyFrom( eve );
    if ( is_full() ) {
        drop_bucket();
    }
}

}  // namespace ::sV::buckets
}  // namespace sV

# endif  //  RPC_PROTOCOLS

