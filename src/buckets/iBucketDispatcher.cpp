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
# include "buckets/iBucketDispatcher.hpp"

# ifdef RPC_PROTOCOLS

# include "app/mixins/protobuf.hpp"

# include "event.pb.h"

# include <iostream>
// # include <fstream>

namespace sV {

iBucketDispatcher::iBucketDispatcher( size_t nMaxKB, size_t nMaxEvents, bool doPackMetaInfo ) :
                            _nBytesMax(nMaxKB*1024), _nMaxEvents(nMaxEvents), _doPackMetaInfo(doPackMetaInfo),
                            _rawBucketPtr(
                                google::protobuf::Arena::CreateMessage<events::Bucket>(
                                            sV::mixins::PBEventApp::arena_ptr()) ),
                            _miCollectorPtr(nullptr) {}

iBucketDispatcher::~iBucketDispatcher() {
    if( !is_bucket_empty() ) {
        drop_bucket();
    }
}

size_t iBucketDispatcher::drop_bucket() {
    if( iBucketDispatcher::do_pack_metainfo() && is_metainfo_collector_set() ) {
        metainfo_collector().pack_metainfo( *bucket().mutable_metainfo() );
    }
    size_t ret = _V_drop_bucket();
    clear_bucket();
    if( is_metainfo_collector_set() ) {
        metainfo_collector().clear();
    }
    return ret;
}

void iBucketDispatcher::clear_bucket() {
    _rawBucketPtr->Clear();
}

bool iBucketDispatcher::is_bucket_full() {
    return ( (n_max_bytes()  != 0 && n_bytes()  >= n_max_bytes() )
          || (n_max_events() != 0 && n_events() >= n_max_events()) );
}

bool iBucketDispatcher::is_bucket_empty() {
    return ( n_bytes() ?  false : true );
}

void iBucketDispatcher::push_event(const events::Event & eve) {
    events::Event* event = bucket().add_events();
    event->CopyFrom( eve );
    if( is_metainfo_collector_set() ) {
        metainfo_collector().consider_event( eve );
    }
    if ( is_bucket_full() ) {
        drop_bucket();
    }
}

void
iBucketDispatcher::metainfo_collector( iAbstractBucketMetaInfoCollector & micPtr ) {
    if( _miCollectorPtr ) {
        _miCollectorPtr->clear();
    }
    _miCollectorPtr = &micPtr;
}

const iAbstractBucketMetaInfoCollector &
iBucketDispatcher::metainfo_collector() const {
    if( !_miCollectorPtr ) {
        emraise( uninitialized, "Metainfo collector is not set for bucket "
            "dispatcher %p while instance was required.", this );
    }
    return *_miCollectorPtr;
}

iAbstractBucketMetaInfoCollector &
iBucketDispatcher::metainfo_collector() {
    const iBucketDispatcher * cthis = this;
    const iAbstractBucketMetaInfoCollector & cref = cthis->metainfo_collector();
    return const_cast<iAbstractBucketMetaInfoCollector &>(cref);
}

}  //  namespace sV

# endif  //  RPC_PROTOCOLS

