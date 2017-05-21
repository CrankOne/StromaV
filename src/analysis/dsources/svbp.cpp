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

# include "analysis/dsources/svbs.hpp"

# ifdef RPC_PROTOCOLS

# include "app/analysis.hpp"

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
    ePtr->CopyFrom( bucket().events( _it.sym().nEvent ) );
    ++_it;
}

BucketReader::Event *
BucketReader::_V_initialize_reading() {
    Event * eventPtr;
    BucketReader::_V_next_event( eventPtr );
    sV_log3( "BucketReader %p: initialized for reading.\n", this );
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

//
// SuppInfoBucketReader
//////////////////////

SuppInfoBucketReader::SuppInfoBucketReader(
                        events::Bucket * bucketPtr,
                        events::BucketInfo * bucketInfoPtr ) :
                                        iEventSequence( 0x0 ),
                                        BucketReader( bucketPtr ),
                                        _bucketInfo( bucketInfoPtr ) {}

void
SuppInfoBucketReader::invalidate_supp_info_caches() const {
    _cacheValid = false;
    for( auto & p : _miCache ) {
        p.second.positionInMetaInfo = USHRT_MAX;
    }
    sV_log3( "SuppInfoBucketReader %p: supp. info caches invalidated.\n", this );
}

const events::BucketInfoEntry &
SuppInfoBucketReader::_supp_info( uint16_t n ) const {
    return supp_info_entries().entries( n );
}

void
SuppInfoBucketReader::_recache_supp_info() const {
    for( int i = 0; i < _bucketInfo->entries_size(); ++i ) {
        const events::BucketInfoEntry & miRef = _bucketInfo->entries( i );
        std::type_index tIdx = _V_get_collector_type_hash( miRef.infotype() );
        auto cacheEntryIt = _miCache.find( tIdx );
        if( _miCache.end() == cacheEntryIt ) {
            cacheEntryIt = _miCache.emplace( tIdx,
                        _V_new_cache_entry( miRef.infotype() ) ).first;
        }
        MetaInfoCache & micRef = cacheEntryIt->second;
        micRef.positionInMetaInfo = i;
    }
    _cacheValid = true;
    sV_log3( "SuppInfoBucketReader %p: supp. info caches renewed.\n", this );
}

const events::BucketInfo &
SuppInfoBucketReader::supp_info_entries() const {
    if( !_bucketInfo ) {
        emraise( badArchitect, "Suplementary info container instance is not "
            "set for reader instance %p.", this );
    }
    if( !is_supp_info_caches_valid() ) {
        _recache_supp_info();
    }
    return *_bucketInfo;
}

events::BucketInfo &
SuppInfoBucketReader::supp_info_entries() {
    const SuppInfoBucketReader * cthis = this;
    return const_cast<events::BucketInfo &>( cthis->supp_info_entries() );
}

//
// CompressedBucketReader
////////////////////////

CompressedBucketReader::CompressedBucketReader(
                events::DeflatedBucket * dfltdBcktPtr,
                events::Bucket * bucketPtr,
                events::BucketInfo * bucketInfoPtr,
                const Decompressors * decompressors ) :
                        iEventSequence( 0x0 ),
                        SuppInfoBucketReader( bucketPtr, bucketInfoPtr ),
                        _dfltdBucketPtr( dfltdBcktPtr ),
                        _decompressedBucketValid( false ),
                        _decompressors( decompressors )
{}

const events::DeflatedBucket &
CompressedBucketReader::compressed_bucket() const {
    if( !_dfltdBucketPtr ) {
        emraise( badState, "Deflated bucket pointer was not set for "
            "reading handle %p.", this );
    }
    return *_dfltdBucketPtr;
}

const events::BucketInfoEntry &
CompressedBucketReader::_supp_info( uint16_t n ) const {
    return supp_info_entries().entries( n );
}

const iDecompressor *
CompressedBucketReader::_decompressor( iDecompressor::CompressionAlgo algo ) const {
    auto it = _decompressors->find( algo );
    if( _decompressors->end() == it ) {
        emraise( notFound, "Has no decompressor referenced by code %d.",
                (int) algo );
    }
    return it->second;
}

void
CompressedBucketReader::_decompress_bucket() const {
    if( ! _dfltdBucketPtr->data().compressedcontent().size() ) {
        sV_logw( "Trying to decompress bucket of zero size.\n" );
    } else {
        sV_log3( "Decompressing data of size %d with algorithm #%d.\n",
                _dfltdBucketPtr->data().compressedcontent().size(),
                _dfltdBucketPtr->data().compressionalgo() );
    }
    const iDecompressor * dcmprPtr = _decompressor(
                                    _dfltdBucketPtr->data().compressionalgo() );
    size_t decompressedLength;  // provisioned
    _dcmBuffer.resize(
            decompressedLength = dcmprPtr->decompressed_dest_buffer_len(
                (const uint8_t *) _dfltdBucketPtr->data().compressedcontent().c_str(),
                _dfltdBucketPtr->data().compressedcontent().size()
            ) );
    size_t realDecompressedLength =
        dcmprPtr->decompress_series(
            (const uint8_t *) _dfltdBucketPtr->data().compressedcontent().c_str(),
            _dfltdBucketPtr->data().compressedcontent().size(),
            _dcmBuffer.data(),
            _dcmBuffer.capacity() );
    # if 0
        const uint8_t * dt = (const uint8_t *) _dfltdBucketPtr->data().compressedcontent().c_str();
        sV_log3( "XXX: %x %x %x ... [%zu] -decompression-> %x %x %x ... [%zu]\n",
            dt[0], dt[1], dt[2], _dfltdBucketPtr->data().compressedcontent().size(),
            _dcmBuffer.data()[0],
            _dcmBuffer.data()[1],
            _dcmBuffer.data()[2], _dcmBuffer.size()
        );
    # endif
    _dcmBuffer.erase(
        _dcmBuffer.begin() + realDecompressedLength,
        _dcmBuffer.end() );
    if( decompressedLength < _dcmBuffer.size() ) {
        emraise( badState, "Decompressed length was erroneously predicted: "
                "%zu (provisioned) < %zu (real).",
                decompressedLength, _dcmBuffer.size() );
    }
    sV_log3( "CompressedBucketReader %p: bucket of size %zu -> %zu "
            "decompressed.\n", this,
            _dfltdBucketPtr->data().compressedcontent().size(),
            decompressedLength );
    if( ! const_cast<CompressedBucketReader *>(this)
                ->_mutable_bucket_ptr()
                ->ParseFromArray( _dcmBuffer.data(), _dcmBuffer.size() )
                // XXX: tests on trivial compression:
                //->ParseFromArray( _dfltdBucketPtr->data().compressedcontent().c_str(),
                //                  _dcmBuffer.size() )
        ) {
        emraise( thirdParty, "Protobuf failed to parse decompressed data of "
            "size %zu.", _dcmBuffer.size() );
    }
    _decompressedBucketValid = true;
    sV_log3( "CompressedBucketReader %p: bucket of size %zu "
            "parsed.\n", this, decompressedLength );
}

const events::Bucket &
CompressedBucketReader::_V_bucket() const  {
    if( !is_decompressed_bucket_valid() ) {
        const_cast<CompressedBucketReader *>(this)->_decompress_bucket();
    }
    return BucketReader::_V_bucket();
}

void
CompressedBucketReader::set_bucket_ptr( events::Bucket * ptr ) {
    invalidate_decompressed_bucket_cache();
    invalidate_supp_info_caches();
    SuppInfoBucketReader::set_bucket_ptr( ptr );
}

void
CompressedBucketReader::set_compressed_bucket_ptr( events::DeflatedBucket * dfltBctPtr ) {
    invalidate_decompressed_bucket_cache();
    _dfltdBucketPtr = dfltBctPtr;
}


//
// Buckets
/////////

# if 0
//void
//BucketReader::_V_print_brief_summary( std::ostream & ) const {}

BucketsFileReader::BucketsFileReader(
            const std::list<goo::filesystem::Path> & filenames,
            size_t maxEvents,
            bool enableProgressbar ) :
                        Parent( 0x0 ),  // TODO: md support
                        sV::AbstractApplication::ASCII_Entry( goo::aux::iApp::exists() ?
                                &goo::app<AbstractApplication>() : nullptr, 1 ),
                        _filenames( filenames.begin(), filenames.end() ),
                        _nEventsMax(0),
                        _pbParameters(nullptr) {
    if( enableProgressbar && maxEvents ) {
        _pbParameters = new PBarParameters;
        bzero( _pbParameters, sizeof(PBarParameters) );
        _pbParameters->mtrestrict = 250;
        _pbParameters->length = 80;
        _pbParameters->full = maxEvents;
    }
}
# endif

}  // namespace ::sV::buckets

}  // namespace sV


namespace std {

size_t
hash<::sV::aux::SHA256BucketHash>::operator()(
                        const ::sV::aux::SHA256BucketHash & sha256 ) const {
    const uint8_t * key = (const uint8_t *) sha256.hash;
    uint32_t h = 157;  // seed, actually
    const uint32_t* key_x4 = (const uint32_t*) key;
    size_t i = 32 >> 2;
    do {
        uint32_t k = *key_x4++;
        k *= 0xcc9e2d51;
        k = (k << 15) | (k >> 17);
        k *= 0x1b873593;
        h ^= k;
        h = (h << 13) | (h >> 19);
        h += (h << 2) + 0xe6546b64;
    } while (--i);
    key = (const uint8_t*) key_x4;

    h ^= 32;
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;
    return h;
}

size_t
equal_to<::sV::aux::SHA256BucketHash>::operator()(
                const ::sV::aux::SHA256BucketHash & sha256l,
                const ::sV::aux::SHA256BucketHash & sha256r ) const {
    return !memcmp( sha256l.hash, sha256r.hash, sizeof(sha256l.hash) );
}

std::ostream &
operator<<(std::ostream & os, const ::sV::aux::SHA256BucketHash & sha256h) {
    os << std::hex;
    for( uint8_t i = 0; i < 8; ++i ) {
        os << sha256h.hash[i];
    }
    os << std::dec;
    return os;
}

}  // namespace std

namespace sV {

Buckets::Buckets(
            events::DeflatedBucket * dfltdBcktPtr,
            events::Bucket * bucketPtr,
            events::BucketInfo * bInfo,
            events::CommonBucketDescriptor * particularInfo,
            const std::list<goo::filesystem::Path> & paths_,
            size_t nMaxEvents,
            bool enableProgressbar,
            uint8_t ASCII_lines ) :
                iEventSequence(0x0),
                Parent( dfltdBcktPtr, bucketPtr, bInfo, particularInfo,
                        &_decompressors, &_file ),
                ASCII_Entry( goo::aux::iApp::exists() ?
                        &goo::app<AbstractApplication>() : nullptr,
                        ASCII_lines ),
                _paths( paths_.begin(), paths_.end() ),
                _lastEvReadingWasGood(false),
                _pbParameters( nullptr ),
                _maxEventsNumber( nMaxEvents ),
                _eventsRead(0) {
    if( enableProgressbar && _maxEventsNumber ) {
        _pbParameters = new PBarParameters;
        bzero( _pbParameters, sizeof(PBarParameters) );
        _pbParameters->mtrestrict = 250;
        _pbParameters->length = 80;
        _pbParameters->full = _maxEventsNumber;
    }
    _sourcesIt = _paths.end();
    sV_log3( "Buckets %p: constructed.\n", this );
}

Buckets::Buckets( const goo::dict::Dictionary & dct ) : Buckets(
            sV_MSG_NEW( events::DeflatedBucket  ),
            sV_MSG_NEW( events::Bucket          ),
            sV_MSG_NEW( events::BucketInfo      ),
            sV_MSG_NEW( events::CommonBucketDescriptor ),
            std::list<goo::filesystem::Path>(
                goo::app<AbstractApplication>().app_options_list<goo::filesystem::Path>("input-file").begin(),
                goo::app<AbstractApplication>().app_options_list<goo::filesystem::Path>("input-file").end()
            ),
            goo::app<AbstractApplication>().app_option<size_t>("max-events-to-read"),
            dct["progressbar"].as<bool>(),
            1
        ) {}

Buckets::~Buckets() {
    for( auto p : _collectors ) {
        delete p.second;
    }
    for( auto p : _decompressors ) {
        delete p.second;
    }
    sV_log3( "Buckets %p: freed.\n", this );
}

std::type_index
Buckets::_V_get_collector_type_hash( const std::string & collectorName ) const {
    auto & sect = sV::sys::IndexOfConstructables::self()
                    .known_constructors<sV::buckets::iAbstractInfoCollector>();
    auto it = sect.find( collectorName );
    if( sect.end() == it ) {
        emraise( noSuchKey, "Unable to find RTTI registered for buckets supp "
            "info type \"%s\".", collectorName.c_str() );
    }
    return it->second->finalTypeIndex;
}

Buckets::MetaInfoCache
Buckets::_V_new_cache_entry( const std::string & collectorName ) const {
    MetaInfoCache ret;
    ret.name = collectorName;
    // Try to find existing collector:
    auto it = _collectors.find( collectorName );
    if( _collectors.end() == it ) {
        // If it is not found, construct one:
        it = _collectors.emplace(
                    collectorName,
                    sV::generic_new<sV::buckets::iAbstractInfoCollector>( collectorName )
                ).first;
    }
    ret.collectorPtr = it->second;
    ret.positionInMetaInfo = USHRT_MAX;
    return ret;
}

const iDecompressor *
Buckets::_decompressor( iDecompressor::CompressionAlgo algo ) const {
    auto it = _decompressors.find( algo );
    if( _decompressors.end() == it ) {
        std::string dcmprssrName = sV::sys::compression_algo_name( algo );
        it = _decompressors.emplace(
                    algo,
                    generic_new<iDecompressor>( dcmprssrName ) ).first;
    }
    return it->second;
}

Buckets::Event *
Buckets::_V_initialize_reading() {
    _file.open( *(_sourcesIt = _paths.begin()) );
    return Parent::_V_initialize_reading();
}

bool
Buckets::_V_is_good() {
    if( !_lastEvReadingWasGood ) {
        sV_log3( "XXX #1\n" );  // XXX
        return false;
    }
    if( _maxEventsNumber && _eventsRead > _maxEventsNumber ) {
        sV_log3( "XXX #2\n" );  // XXX
        return false;
    }
    return true;
}

bool
Buckets::_V_acquire_next_bucket( BucketReader::Event *& epr ) {
    if( _paths.end() == _sourcesIt ) {
        return _lastEvReadingWasGood = false;
    }
    if( Parent::_V_acquire_next_bucket( epr ) ) {
        sV_log3( "Next bucket in a queue acquired.\n" );
        return _lastEvReadingWasGood = true;
    }
    if( _paths.end() == ++_sourcesIt ) {
        sV_log3( "Buckets %p: has no more sources in queue of %zu entries. Done.\n",
                this, _paths.size() );
        return _lastEvReadingWasGood = false;
    }
    _file.close();
    _file.open( *_sourcesIt/*->interpolated()*/ );
    sV_log3( "Buckets %p: switched to next source in a queue: \"%s\".\n",
        this, _sourcesIt->/*interpolated().*/c_str() );
    return _lastEvReadingWasGood = Parent::_V_acquire_next_bucket( epr );
}

StromaV_EVENTS_SEQUENCE_DEFINE_MCONF( Buckets, "svb" ) {
    goo::dict::Dictionary svbfRetrieve( "svb",
        "Generic reader for compressed buckets files (.svbp)." );
    svbfRetrieve.insertion_proxy()
        .flag("progressbar",
            "Displays simple ASCII progressbar on stdout.")
        ;
    goo::dict::DictionaryInjectionMap injM;
        injM( "progressbar",        "analysis.data-sources.retreive.progressbar" )
            ;
    return std::make_pair( svbfRetrieve, injM );
}

}  // namespace sV

# endif  // RPC_PROTOCOLS

