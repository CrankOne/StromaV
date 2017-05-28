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

# include "analysis/dsources/svbs/suppInfoBucketReader.hpp"

namespace sV {
namespace buckets {

//
// SuppInfoBucketReader
//////////////////////

SuppInfoBucketReader::SuppInfoBucketReader(
                        events::Bucket * bucketPtr,
                        events::BucketInfo * bucketInfoPtr,
                        std::ostream * logStream ) :
                                        iEventSequence( 0x0 ),
                                        BucketReader( bucketPtr ),
                                        logging::Logger( "buckets-reading", "SuppInfoBucketReader $(this)" ),
                                        _bucketInfo( bucketInfoPtr ) {}

void
SuppInfoBucketReader::invalidate_supp_info_caches() const {
    _cacheValid = false;
    for( auto & p : _miCache ) {
        p.second.positionInMetaInfo = USHRT_MAX;
    }
    log_msg( logging::laconic,
        "supp. info caches invalidated.\n", this );
}

const events::BucketInfoEntry &
SuppInfoBucketReader::_supp_info( uint16_t n ) const {
    return supp_info_entries().entries( n );
}

void
SuppInfoBucketReader::_recache_supp_info() const {
    size_t nRenewed = 0,
           nInserted = 0;
    for( int i = 0; i < _bucketInfo->entries_size(); ++i ) {
        const events::BucketInfoEntry & miRef = _bucketInfo->entries( i );
        std::type_index tIdx = _V_get_collector_type_hash( miRef.infotype() );
        auto cacheEntryIt = _miCache.find( tIdx );
        if( _miCache.end() == cacheEntryIt ) {
            cacheEntryIt = _miCache.emplace( tIdx,
                        _V_new_cache_entry( miRef.infotype() ) ).first;
            ++nInserted;
            log_msg( logging::loquacious, "insertion of type %s (%x)\n",
                miRef.infotype().c_str(), tIdx );
        } else {
            ++nRenewed;
            log_msg( logging::loquacious, "renewal of type %s (%x)\n",
                miRef.infotype().c_str(), tIdx );
        }
        MetaInfoCache & micRef = cacheEntryIt->second;
        if( micRef.positionInMetaInfo != USHRT_MAX ) {
            sV_logw( "Possible duplicating of bucket supp info of type %s (%x). "
                "New index in array is %u, previous: %u.\n",
                miRef.infotype().c_str(), tIdx,
                micRef.positionInMetaInfo, i );
        }
        micRef.positionInMetaInfo = i;
    }
    _cacheValid = true;
    log_msg( logging::verbose, "SuppInfoBucketReader %p: %zu supp. info caches renewed, "
        "%zu inserted from %d entries.\n", this, nRenewed, nInserted,
        _bucketInfo->entries_size() );
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
SuppInfoBucketReader::mutable_supp_info_entries() {
    return *_bucketInfo;
}

}  // namespace ::sV::buckets
}  // namespace sV

# endif  // RPC_PROTOCOLS


