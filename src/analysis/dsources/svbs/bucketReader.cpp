/*
 * Copyright (c) 2017 Renat R. Dusaev <crank@qcrypt.org>
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

# ifdef RPC_PROTOCOLS

# include "analysis/dsources/svbs/bucketReader.hpp"

namespace sV {
namespace buckets {

// BucketReader
//////////////

BucketReader::BucketReader( events::Bucket * reentrantBucketPtr ) :
            iEventSequence( 0x0 ),
            _cBucket( reentrantBucketPtr ),
            _it( reentrantBucketPtr ) {}

const events::Bucket &
BucketReader::_V_bucket() const {
    if( !_cBucket ) {
        emraise( badState, "Bucket pointer was not set for reading handle %p.",
            this );
    }
    return *_cBucket;
}

bool
BucketReader::_V_is_good() {
    return _it.sym().nEvent < BucketReader::n_events();
}

void
BucketReader::_V_next_event( Event *& ePtr ) {
    ePtr = &(::sV::mixins::PBEventApp::c_event());
    ePtr->Clear();
    if( _it.sym().nEvent >= (size_t) bucket().events_size() ) {
        emraise( overflow, "Required reading %zu-th event from bucket of %d "
            "events.", _it.sym().nEvent, bucket().events_size() );
    }
    ePtr->CopyFrom( bucket().events( _it.sym().nEvent ) );
    ++_it;
}

BucketReader::Event *
BucketReader::_V_initialize_reading() {
    Event * eventPtr;
    reset_bucket_iterator();
    BucketReader::_V_next_event( eventPtr );
    return eventPtr;
}

void
BucketReader::_V_finalize_reading() {
    _it.sym().nEvent = (size_t) bucket().events_size();
}

size_t
BucketReader::n_events() const {
    if( !is_bucket_set() ) {
        return 0;
    }
    return bucket().events_size();
}

void
BucketReader::reset_bucket_iterator() const {
    _it.sym() = 0;
}

}  // namespace ::sV::buckets
}  // namespace sV

# endif  // RPC_PROTOCOLS


