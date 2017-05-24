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

//
// GenericCollector
//////////////////

StromaV_BUCKET_INFO_COLLECTOR_DEFINE_MCONF( GenericCollector, "Generic" ) {
    goo::dict::Dictionary genericCfg( "own_GenericBucketInfo", 
        "Generic bucket supp. info collector that implements common fropping "
        "criteria: dropping by size limits (# event or/and #kbytes of raw data). "
        "This class also intoduces the SHA256 digest of raw (uncompressed) buckets. "
        "SHA256 checksum may further be used for bucket identification as well "
        "as for integrity checks. The sum is intended to uncompressed "
        "serialized buckets, may cause a significant performance impact (due "
        "to complexity of SHA256), and has a dynamic cast operation that "
        "may yield a deficiency of overall performance for small buckets.");
    genericCfg.insertion_proxy()
        .p<size_t>("maxBucketSize_kB",
                        "Maximum bucket capacity (in kilobytes). 0 disables"
                        "criterion.",
                    500)
        .p<size_t>("maxBucketSize_events",
                        "Maximum bucket capacity (number of enents). 0 disables "
                        "criterion. For compressed bucket dispatching the best "
                        "choice will become to set it for few thousands since "
                        "most of compression algorithms provides reasonable "
                        "ratio for few megabytes of data.",
                    0)
        ;
    goo::dict::DictionaryInjectionMap injM;
        injM( "maxBucketSize_kB",       "buckets.generic.maxBucketSize_kB" )
            ( "maxBucketSize_events",   "buckets.generic.maxBucketSize_events" )
            ;
    return std::make_pair( genericCfg, injM );
}

GenericCollector::GenericCollector( events::CommonBucketDescriptor * rmsgPtr,
                                    size_t nMaxEvents, size_t nMaxBytes ) :
                Parent(rmsgPtr),
                _rawDataSize(0), _nEvents(0),
                _maxEvents(nMaxEvents), _maxSizeInBytes(nMaxBytes) {
    clear();
}

GenericCollector::GenericCollector(
            const goo::dict::Dictionary & dct ) : GenericCollector(
                google::protobuf::Arena::CreateMessage<events::CommonBucketDescriptor>(
                        sV::mixins::PBEventApp::arena_ptr()
                    ),
                dct["maxBucketSize_events"].as<size_t>(),
                dct["maxBucketSize_kB"].as<size_t>()*1024
            ) {}

bool
GenericCollector::_V_consider_event( const events::Event &,
                                     const iBundlingDispatcher & ibdsp ) {
    ++_nEvents;
    _rawDataSize = ibdsp.bucket().ByteSize();
    if(  (_maxEvents && (_maxEvents < _nEvents))
      || (_maxSizeInBytes && (_maxSizeInBytes < _rawDataSize)) ) {
        // Conditions triggered.
        return true;
    }
    // Keep going.
    return false;
}

void
GenericCollector::_V_clear() {
    bzero( _hash, SHA256_DIGEST_LENGTH );
    _nEvents = _rawDataSize = 0;
    bzero( _hash, sizeof(_hash) );
}

void
GenericCollector::_V_pack_suppinfo(
                ::google::protobuf::Any* miMsgRef,
                const iBundlingDispatcher & ibdsp ) {
    _my_supp_info_ptr()->set_nevents( _nEvents );
    const IPlainBufferDispatcher & ipbdsp =
                        dynamic_cast<const IPlainBufferDispatcher &>(ibdsp);
    // Compute and write SHA256 hash:
    SHA256( ipbdsp.raw_buffer_data(), ipbdsp.raw_buffer_length(), _hash );
    _my_supp_info_ptr()->set_sha256hash( _hash, SHA256_DIGEST_LENGTH );

    Parent::_V_pack_suppinfo( miMsgRef, ibdsp );
}

//
// iBundlingDispatcher
/////////////////////

iBundlingDispatcher::iBundlingDispatcher(   events::BucketInfo * biEntriesPtr,
                                            bool doPackSuppInfo ) :
            iDispatcher(),
            _doPackSuppInfo(doPackSuppInfo),
            _biEntriesPtr(biEntriesPtr) { }

bool
iBundlingDispatcher::_V_is_full() const {
    if( are_suppinfo_collectors_set() ) {
        for( auto cp : suppinfo_collectors() ) {
            if( cp.second->do_drop() ) {
                return true;
            }
        }
    }
    return false;
}

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
    iDispatcher::push_event( eve );
    // Consider event with supp. info collectors:
    bool doDrop = false;
    if( are_suppinfo_collectors_set() ) {
        for( auto cp : suppinfo_collectors() ) {
            doDrop |= cp.second->consider_event( eve, *this );
        }
    }
    if( doDrop ) {
        drop_bucket();
    }
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


