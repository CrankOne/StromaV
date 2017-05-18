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
# include "buckets/iBundlingDispatcher.hpp"

# ifdef RPC_PROTOCOLS

# include "app/mixins/protobuf.hpp"

namespace sV {
namespace buckets {

ChecksumsCollector::ChecksumsCollector( events::CommonBucketDescriptor * rmsgPtr ) :
                Parent(rmsgPtr) {
    clear();
}

ChecksumsCollector::ChecksumsCollector(
            const goo::dict::Dictionary & ) : ChecksumsCollector(
                google::protobuf::Arena::CreateMessage<events::CommonBucketDescriptor>(
                        sV::mixins::PBEventApp::arena_ptr()
                    )
                ) {}

void
ChecksumsCollector::_V_consider_event( const events::Event & ) {
    ++_nEvents;
}

void
ChecksumsCollector::_V_clear() {
    bzero( _hash, SHA256_DIGEST_LENGTH );
    _nEvents = 0;
}

void
ChecksumsCollector::_V_pack_suppinfo(
                ::google::protobuf::Any* miMsgRef,
                const iBundlingDispatcher & ibdsp ) {
    _my_supp_info_ptr()->set_nevents( _nEvents );
    _TODO_  // TODO: need entire bucket data to calculate hash
    _my_supp_info_ptr()->set_sha256hash( _hash, SHA256_DIGEST_LENGTH );
    Parent::_V_pack_suppinfo( miMsgRef, ibdsp );
}

//
//
//

//

void
iBundlingDispatcher::_append_suppinfo( events::BucketInfo & bInfo ) {
    for( auto cp : metainfo_collectors() ) {
        ::sV::events::BucketInfoEntry * entryPtr = bInfo.add_entries();
        entryPtr->set_infotype( cp.first );
        cp.second->pack_suppinfo( entryPtr->mutable_suppinfo(), *this );
        cp.second->clear();
    }
}

size_t
iBundlingDispatcher::drop_bucket() {
    if( iBundlingDispatcher::do_pack_metainfo()
     && are_metainfo_collectors_set() ) {
        _append_suppinfo( *(bucket().mutable_info()) );
    }
    return iDispatcher::drop_bucket();
}

void
iBundlingDispatcher::push_event(const events::Event & eve) {
    if( are_metainfo_collectors_set() ) {
        for( auto cp : metainfo_collectors() ) {
            cp.second->consider_event( eve );
        }
    }
    iDispatcher::push_event( eve );
}

void
iBundlingDispatcher::metainfo_collectors( CollectorsMap & mics ) {
    for( auto cp : metainfo_collectors() ) {
        cp.second->clear();
    }
    metainfo_collectors().clear();
    _miCollectors = mics;
}

const iBundlingDispatcher::CollectorsMap &
iBundlingDispatcher::metainfo_collectors() const {
    return _miCollectors;
}

iBundlingDispatcher::CollectorsMap &
iBundlingDispatcher::metainfo_collectors() {
    const iBundlingDispatcher * cthis = this;
    const CollectorsMap & cref = cthis->metainfo_collectors();
    return const_cast<CollectorsMap &>(cref);
}

}  // namespace ::sV::buckets
}  //  namespace sV

# endif  //  RPC_PROTOCOLS


