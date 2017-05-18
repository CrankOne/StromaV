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

# ifndef H_STROMA_V_SVBP_READER_H
# define H_STROMA_V_SVBP_READER_H

# include "sV_config.h"

# ifdef RPC_PROTOCOLS

# include "analysis/pipeline.hpp"
# include "compression/iDecompressor.hpp"
# include "buckets/iBucketDispatcher.hpp"
# include "utils.h"

# include <goo_mixins/iterable.tcc>

# include <fstream>

namespace sV {

namespace buckets {

/**@brief Performs basic reading from events bucket.
 * @class BucketReader
 *
 * Provides basic implementation for reading events from bucket.
 *
 * @ingroup analysis
 */
class BucketReader : public virtual sV::aux::iEventSequence {
public:
    typedef sV::aux::iEventSequence Parent;
    typedef typename sV::AnalysisPipeline::Event Event;

    /// Bucket event id. Used for custom iterator class.
    template<typename BucketT>
    struct BucketEventID {
        BucketT * authorityPtr;
        size_t nEvent;

        BucketEventID( BucketT * bPtr, size_t nEve=0 ) :
                                            authorityPtr(bPtr), nEvent(nEve) {}

        BucketEventID & operator++() {
            ++nEvent;
            return *this;
        }
    };

    /// Template declaration for bucket iterator.
    template<typename EventT, typename BucketT>
    class _BucketIterator : public goo::iterators::Iterator<
                                        goo::iterators::BaseBidirIterator,
                                        goo::iterators::BaseOutputIterator,
                                        goo::iterators::DirectionComparableIterator,
                                        _BucketIterator<EventT, BucketT>,
                                        BucketEventID<BucketT>, EventT, size_t> {
    private:
        BucketEventID<BucketT> _sym;
    public:
        _BucketIterator( BucketT * bPtr, size_t nEve=0 ) :
                    _sym( bPtr, nEve ) {}

        BucketEventID<BucketT> & sym() { return _sym; }
        const BucketEventID<BucketT> & sym() const { return _sym; }
    };

    typedef _BucketIterator<const events::Event *, const events::Bucket>
                                                            ConstBucketIterator;
private:
    events::Bucket * _cBucket;
    ConstBucketIterator _it;
protected:
    virtual bool _V_is_good() override;
    virtual void _V_next_event( Event *& ) override;
    virtual Event * _V_initialize_reading() override;
    virtual void _V_finalize_reading() override;

    /// This method may be overriden by user classes that may implement some
    /// caching procedures.
    virtual const events::Bucket & _V_bucket() const;

    events::Bucket * _mutable_bucket_ptr() { return _cBucket; }
public:
    /// Ctr. Accepts a ptr to reentrant bucket message instance.
    BucketReader( events::Bucket * bucketPtr );

    /// (const) Returns reference to current bucket.
    const events::Bucket & bucket() const { return _V_bucket(); }

    /// Returns number of events in a bucket.
    virtual size_t n_events() const;

    /// Returns true when bucket pointer was associated with this handle.
    bool is_bucket_set() const { return _cBucket; }

    /// Sets bucket pointer to be treated with this handle.
    virtual void set_bucket_ptr( events::Bucket * ptr ) { _cBucket = ptr; }

    /// Returns iterator pointing to first event.
    ConstBucketIterator begin() const {
        return ConstBucketIterator( &(bucket()) );
    }

    /// Returns iterator pointing to the end of events.
    ConstBucketIterator end() const {
        return ConstBucketIterator( &(bucket()), bucket().events_size() );
    }

    /// Event getter operator.
    const events::Event & operator[]( size_t n ) {
        return bucket().events(n);
    }
};  // BucketReader


/**@brief A bucket reader with supplementary information acq. shortcuts.
 * @class SuppInfoBucketReader
 *
 * This class introduces access to buckets metadata. Keeps metadata handling
 * code.
 *
 * @ingroup analysis
 */
class SuppInfoBucketReader : public BucketReader {
public:
    struct MetaInfoCache {
        std::string name;
        iAbstractBucketMetaInfoCollector * collectorPtr;
        uint16_t positionInMetaInfo;  // set to USHRT_MAX when invalid.
    };
private:
    /// Indicates whether _miCache is valid for current bucket.
    mutable bool _cacheValid;
    /// Keeps RTTI mappings for suppInfo.
    mutable std::unordered_map<std::type_index, MetaInfoCache> _miCache;
protected:
    /// Shall return bucket's metainfo hdr. May be overriden by descendants in
    /// case the metainfo shall not be obtained from bucket itself.
    virtual const events::BucketInfoEntry & _metainfo( uint16_t ) const;

    /// Invalidates supp. info caches
    void invalidate_supp_info_caches() const;

    /// Will use metainfo() method to set up internal caches ready for various
    /// supp_info acquizition.
    virtual void _recache_supp_info();

    /// Has to return C++ RTTI type hash for given supp info data type.
    virtual std::type_index _V_get_collector_type_hash( const std::string & ) = 0;

    /// Has to allocate new cache entry with set name and collector ptr fields.
    virtual MetaInfoCache _V_new_cache_entry( const std::string & ) = 0;
public:
    SuppInfoBucketReader( events::Bucket * bucketPtr );

    /// Writes the content of supp info of target type to destination by
    /// reference and returns true. Returns false if no supp info of this type
    /// available in current bucket.
    template<typename T> bool supp_info( T & dest ) const {
        auto it = _miCache.find( typeid(T) );
        if( _miCache.end() == it
         || it->second.positionInMetaInfo == USHRT_MAX ) {
            return false;
        }
        it->second.collectorPtr->unpack_suppinfo(
                    _metainfo( it->second.positionInMetaInfo ).suppinfo() );
    }
};

/**@brief A compressed bucket reader.
 * @class CompressedBucketReader
 *
 * A class incapsulating decompression for buckets compressed inside the
 * DeflatedBucket data structure. Will automatically apply decompression
 * for data when bucket will be necessary.
 *
 * @ingroup analysis
 */
class CompressedBucketReader : public SuppInfoBucketReader {
public:
    typedef std::unordered_map<int, const iDecompressor *>
            Decompressors;
private:
    /// Keeps pointer to DeflatedBucket instance that stores the compressed
    /// data.
    events::DeflatedBucket * _dfltdBucketPtr;

    /// True if decompressed bucket is available and was obtained from
    /// current compressed one.
    mutable bool _decompressedBucketValid;

    /// Dictionary of decompressors. Will be appended by reader class by need.
    const Decompressors * _decompressors;

    /// Temporary buffer keeping decompressed data.
    mutable std::vector<uint8_t> _dcmBuffer;

    /// Will decompress bucket upon call if it was not decompressed before.
    virtual const events::Bucket & _V_bucket() const override;
protected:
    /// Overrides default metainfo getter to obtain compressed metainfo instead
    /// of raw.
    virtual const events::BucketInfoEntry & _metainfo( uint16_t ) const override;

    /// Look-up for required decompression algorithm.
    virtual const iDecompressor * _decompressor( iDecompressor::CompressionAlgo ) const;

    /// Performs decompression, sets the internal caches to handle decompressed
    /// data as bucket.
    virtual void _decompress_bucket() const;

    /// For descendants --- mutable deflated bucket getter. Do not forget to
    /// mark current decompressed bucket as invalid with
    /// invalidate_decompressed_bucket_cache() after making changes.
    events::DeflatedBucket * _mutable_deflated_bucket_ptr()
                                                    { return _dfltdBucketPtr; }
public:
    CompressedBucketReader( events::DeflatedBucket *,
                            events::Bucket *,
                            const Decompressors * );

    /// Returns reference to associated deflated bucket isntance.
    const events::DeflatedBucket & deflated_bucket() const;

    /// Returns true when bucket pointer was associated with this handle.
    virtual bool is_deflated_bucket_set() const { return _dfltdBucketPtr; }

    /// _decompressedBucketValid getter.
    bool is_decompressed_bucket_valid() const { return _decompressedBucketValid; }

    /// Marks bucket cache as invalid before forwarding invokation to parent.
    virtual void set_bucket_ptr( events::Bucket * ptr );

    /// Sets deflated bucket pointer to handle the provided instance.
    virtual void set_deflated_bucket_ptr( events::DeflatedBucket * );

    /// Marks decompressed bucket cache as invalid.
    void invalidate_decompressed_bucket_cache()
        { _decompressedBucketValid = false; }
};

class BucketStreamReader : public CompressedBucketReader {
private:
    std::istream * _iStreamPtr;
protected:
    virtual void _V_acquire_next_bucket();
    virtual bool _V_buckets_available() const;
    virtual bool _V_events_available() const;
public:
    
};


}  // namespace ::sV::buckets

}  // namespace sV

# endif  // RPC_PROTOCOLS

# endif  // H_STROMA_V_SVBP_READER_H

