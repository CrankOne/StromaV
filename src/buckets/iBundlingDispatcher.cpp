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

# include "buckets/iTPlainBufferDispatcher.tcc"

namespace sV {
namespace buckets {

StromaV_BUCKET_INFO_COLLECTOR_DEFINE( ChecksumsCollector, "SHA256" ) {
    return goo::dict::Dictionary( NULL, 
        "This class intoduces the SHA256 digest of raw (uncompressed) buckets. "
        "SHA256 checksum may further be used for bucket identification as well "
        "as for integrity checks. The sum is intended to uncompressed "
        "serialized buckets, may cause a significant performance impact (due "
        "to complexity of SHA256), and has a dynamic cast operation that "
        "may yield a deficiency of overall performance for small buckets.");
}

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
    const IPlainBufferDispatcher & ipbdsp = dynamic_cast<const IPlainBufferDispatcher &>(ibdsp);

    SHA256( ipbdsp.raw_buffer_data(), ipbdsp.raw_buffer_length(), _hash );

    _my_supp_info_ptr()->set_sha256hash( _hash, SHA256_DIGEST_LENGTH );
    Parent::_V_pack_suppinfo( miMsgRef, ibdsp );
}

//
//
//

//

iBundlingDispatcher::iBundlingDispatcher(   size_t nMaxKB,
                                            size_t nMaxEvents,
                                            events::BucketInfo * biEntriesPtr,
                                            bool doPackSuppInfo ) :
            iDispatcher( nMaxKB, nMaxEvents ),
            _doPackSuppInfo(doPackSuppInfo),
            _biEntriesPtr(biEntriesPtr) { }

void
iBundlingDispatcher::_append_suppinfo() {
    for( auto cp : suppinfo_collectors() ) {
        ::sV::events::BucketInfoEntry * entryPtr = supp_info().add_entries();
        entryPtr->set_infotype( cp.first );
        cp.second->pack_suppinfo( entryPtr->mutable_suppinfo(), *this );
        cp.second->clear();
    }
}

size_t
iBundlingDispatcher::drop_bucket() {
    if( iBundlingDispatcher::do_pack_suppinfo()
     && are_suppinfo_collectors_set() ) {
        _append_suppinfo();
    }
    return iDispatcher::drop_bucket();
}

void
iBundlingDispatcher::push_event(const events::Event & eve) {
    if( are_suppinfo_collectors_set() ) {
        for( auto cp : suppinfo_collectors() ) {
            cp.second->consider_event( eve );
        }
    }
    iDispatcher::push_event( eve );
}

void
iBundlingDispatcher::suppinfo_collectors( CollectorsMap & mics ) {
    for( auto cp : suppinfo_collectors() ) {
        cp.second->clear();
    }
    suppinfo_collectors().clear();
    _miCollectors = mics;
}

const iBundlingDispatcher::CollectorsMap &
iBundlingDispatcher::suppinfo_collectors() const {
    return _miCollectors;
}

iBundlingDispatcher::CollectorsMap &
iBundlingDispatcher::suppinfo_collectors() {
    const iBundlingDispatcher * cthis = this;
    const CollectorsMap & cref = cthis->suppinfo_collectors();
    return const_cast<CollectorsMap &>(cref);
}

events::BucketInfo &
iBundlingDispatcher::supp_info() {
    const iBundlingDispatcher * cthis = this;
    return const_cast<events::BucketInfo &>( cthis->supp_info() );
}

const events::BucketInfo &
iBundlingDispatcher::supp_info() const {
    if( !_biEntriesPtr ) {
        emraise( badArchitect,
            "Supp info msg ptr is not set for bundling dispatcher instance %p.",
            this );
    }
    return *_biEntriesPtr;
}

void
iBundlingDispatcher::add_collector( const std::string & name,
                                    iAbstractInfoCollector & collector ) {
    auto ir = suppinfo_collectors().emplace( name, &collector );
    if( !ir.second ) {
        sV_logw( "Duplicating insertion of an info collector \"%s\" denied. "
            "Offered instance ptr: %p, existing: %p.\n",
            name.c_str(), &collector, ir.first->second );
    }
}

}  // namespace ::sV::buckets
}  //  namespace sV

# endif  //  RPC_PROTOCOLS


