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

CommonBucketDescription::CommonBucketDescription( events::CommonBucketDescriptor * rmsgPtr ) :
                Parent(rmsgPtr) {
    clear();
}

CommonBucketDescription::CommonBucketDescription(
            const goo::dict::Dictionary & ) : CommonBucketDescription(
                google::protobuf::Arena::CreateMessage<events::CommonBucketDescriptor>(
                        sV::mixins::PBEventApp::arena_ptr()
                    )
                ) {}

void
CommonBucketDescription::_V_consider_event( const events::Event & eve ) {
    ++_nEvents;
}

void
CommonBucketDescription::_V_clear() {
    bzero( _hash, SHA256_DIGEST_LENGTH );
    _nEvents = 0;
}

void
CommonBucketDescription::_V_pack_suppinfo( ::google::protobuf::Any* miMsgRef ) {
    _my_supp_info_ptr()->set_nevents( _nEvents );
    _TODO_  // TODO: need entire bucket data to calculate hash
    _my_supp_info_ptr()->set_sha256hash( _hash, SHA256_DIGEST_LENGTH );
    Parent::_V_pack_suppinfo( miMsgRef );
}

//
// iBucketDispatcher
///////////////////

iBucketDispatcher::iBucketDispatcher( size_t nMaxKB, size_t nMaxEvents, bool doPackMetaInfo ) :
                            _nBytesMax(nMaxKB*1024), _nMaxEvents(nMaxEvents), _doPackMetaInfo(doPackMetaInfo),
                            _rawBucketPtr(
                                google::protobuf::Arena::CreateMessage<events::Bucket>(
                                            sV::mixins::PBEventApp::arena_ptr()) ) {}

iBucketDispatcher::~iBucketDispatcher() {
    if( !is_bucket_empty() ) {
        drop_bucket();
    }
}

void
iBucketDispatcher::_append_suppinfo_if_need() {
    if( iBucketDispatcher::do_pack_metainfo() && are_metainfo_collectors_set() ) {
        for( auto cp : metainfo_collectors() ) {
            ::sV::events::BucketMetaInfo * miPtr = bucket().add_metainfo();
            miPtr->set_metainfotype( cp.first );
            cp.second->pack_suppinfo( miPtr->mutable_suppinfo() );
            cp.second->clear();
        }
    }
}

size_t
iBucketDispatcher::drop_bucket() {
    iBucketDispatcher::_append_suppinfo_if_need();
    size_t ret = _V_drop_bucket();
    clear_bucket();
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
    if( are_metainfo_collectors_set() ) {
        for( auto cp : metainfo_collectors() ) {
            cp.second->consider_event( eve );
        }
    }
    if ( is_bucket_full() ) {
        drop_bucket();
    }
}

void
iBucketDispatcher::metainfo_collectors( iBucketDispatcher::CollectorsMap & mics ) {
    for( auto cp : metainfo_collectors() ) {
        cp.second->clear();
    }
    metainfo_collectors().clear();
    _miCollectors = mics;
}

const iBucketDispatcher::CollectorsMap &
iBucketDispatcher::metainfo_collectors() const {
    return _miCollectors;
}

iBucketDispatcher::CollectorsMap &
iBucketDispatcher::metainfo_collectors() {
    const iBucketDispatcher * cthis = this;
    const CollectorsMap & cref = cthis->metainfo_collectors();
    return const_cast<CollectorsMap &>(cref);
}

}  //  namespace sV

# endif  //  RPC_PROTOCOLS

