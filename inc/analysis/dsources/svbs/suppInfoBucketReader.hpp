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

# ifndef H_STROMA_V_SVBP_READER_BUCKET_SUPP_INFO_READER_H
# define H_STROMA_V_SVBP_READER_BUCKET_SUPP_INFO_READER_H

# include "sV_config.h"

# ifdef RPC_PROTOCOLS

# include "analysis/dsources/svbs/bucketReader.hpp"
# include "buckets/iBundlingDispatcher.hpp"
# include "logging.hpp"

namespace sV {
namespace buckets {

/**@brief A bucket reader with supplementary information acq. shortcuts.
 * @class SuppInfoBucketReader
 *
 * This class introduces access to buckets supplementary info based on
 * supp. info collector classes implementation.
 *
 * The bucket itself does not provide any data except the sequence of events.
 * However, for many practical application user code usually involves some
 * preliminary (supplementary) information about these events: number of
 * events, size, various excerpts, hash sums, etc. Thus, immediately after the
 * basic sequence management it does become crucial to introduce support for
 * such supplementary data.
 *
 * At the level of bucket serialization mechanism the supplementary data is
 * defined via the collectors classes (see iAbstractInfoCollector descendants).
 * Hence we will use the same definitions for unpacking supplementary
 * information.
 *
 * This class inherits the basic BucketReader class and additionally contains
 * the internal cache of the related supplementary information. One has to
 * note that for the sake of efficiency this cache won't be unpacked
 * immediately all at once. Instead, each cache entry will be unpacked by
 * demand. Standard getter is the supp_info_entry() template method.
 *
 * The couple of abstract methods has to define interaction with some external
 * source of information about available supp. info collector classes.
 *
 * Since managing buckets with supplementary information may become a
 * complicated thing, for development and debugging purposes it becomes
 * convinient to introduce logging. Operating with buckets within handling
 * pipeline usually a prolonged routine, so putting these logs into a dedicated
 * stream may also be convinient.
 *
 * @ingroup analysis
 * @ingroup buckets
 */
class SuppInfoBucketReader : public BucketReader,
                             public sV::logging::Logger {
public:
    /// Single supplementary information cache entry.
    struct MetaInfoCache {
        std::string name;
        buckets::iAbstractInfoCollector * collectorPtr;
        uint16_t positionInMetaInfo;  // set to USHRT_MAX when invalid.
    };
private:
    /// Ptr to supp info message-container for current bucket.
    events::BucketInfo * _bucketInfo;
    /// Indicates whether _miCache is valid for current bucket.
    mutable bool _cacheValid;
    /// Keeps RTTI mappings for suppInfo.
    mutable std::unordered_map<std::type_index, MetaInfoCache> _miCache;
protected:
    /// Shall return bucket's metainfo hdr. May be overriden by descendants in
    /// case the metainfo shall not be obtained from bucket itself.
    virtual const events::BucketInfoEntry & _supp_info( uint16_t ) const;
    /// Invalidates supp. info caches
    void invalidate_supp_info_caches() const;
    /// Will use _supp_info() method to set up internal caches ready for various
    /// supp_info_entries() acquizition.
    virtual void _recache_supp_info() const;
    /// Has to return C++ RTTI type hash for given supp info data type.
    virtual std::type_index _V_get_collector_type_hash( const std::string & ) const = 0;
    /// Has to allocate new cache entry with set name and collector ptr fields.
    virtual MetaInfoCache _V_new_cache_entry( const std::string & ) const = 0;
    /// Returns mutable reference to supplementary info container of current
    /// bucket. Usually used as a setter. Doesn't cause recaching.
    events::BucketInfo & mutable_supp_info_entries();
public:
    /// Ctr. Accepts pointers to protobuf messages that will be used as
    /// reentrant buffers. For performance, one probably would prefer to
    /// allocate them using protobuf's arena.
    SuppInfoBucketReader( events::Bucket * bucketPtr,
                          events::BucketInfo * bucketInfoPtr );
    /// Returns immutable reference to supplementary info container of current
    /// bucket.
    const events::BucketInfo & supp_info_entries() const;
    /// Returns true when supp info caches are valid (related to the current
    /// bucket).
    bool is_supp_info_caches_valid() const { return _cacheValid; }
    /// Writes the content of supp info of target type to destination by
    /// reference and returns true. Returns false if no supp info of this type
    /// available in current bucket.
    template<typename T> bool supp_info_entry( typename T::CollectingType & dest ) const {
        if( !is_supp_info_caches_valid() ) {
            _recache_supp_info();
        }
        auto it = _miCache.find( std::type_index(typeid(T)) );
        if( _miCache.end() == it
         || it->second.positionInMetaInfo == USHRT_MAX ) {
            return false;
        }
        it->second.collectorPtr->unpack_suppinfo(
                    _supp_info( it->second.positionInMetaInfo ).suppinfo() );
        dest.CopyFrom( dynamic_cast<T*>(it->second.collectorPtr)->own_supp_info_ref() );
        return true;
    }
};  // class SuppInfoBucketReader

}  // namespace ::sV::buckets
}  // namespace sV

# endif  // RPC_PROTOCOLS

# endif  // H_STROMA_V_SVBP_READER_BUCKET_SUPP_INFO_READER_H

