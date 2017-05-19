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
# include "buckets/iBundlingDispatcher.hpp"
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
 * This class introduces access to buckets supplementary info based on
 * supp. info collector classes implementation.
 *
 * @ingroup analysis
 */
class SuppInfoBucketReader : public BucketReader {
public:
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
    virtual const events::BucketInfoEntry & _metainfo( uint16_t ) const;

    /// Invalidates supp. info caches
    void invalidate_supp_info_caches() const;

    /// Will use _metainfo() method to set up internal caches ready for various
    /// supp_info_entries() acquizition.
    virtual void _recache_supp_info();

    /// Has to return C++ RTTI type hash for given supp info data type.
    virtual std::type_index _V_get_collector_type_hash( const std::string & ) = 0;

    /// Has to allocate new cache entry with set name and collector ptr fields.
    virtual MetaInfoCache _V_new_cache_entry( const std::string & ) = 0;

    /// Returns mutable reference to supplementary info container of current
    /// bucket. Usually used as a setter.
    events::BucketInfo & supp_info_entries();
public:
    SuppInfoBucketReader( events::Bucket * bucketPtr,
                          events::BucketInfo * bucketInfoPtr );

    /// Returns immutable reference to supplementary info container of current
    /// bucket.
    const events::BucketInfo & supp_info_entries() const;

    /// Writes the content of supp info of target type to destination by
    /// reference and returns true. Returns false if no supp info of this type
    /// available in current bucket.
    template<typename T> bool supp_info_entry( T & dest ) const {
        auto it = _miCache.find( typeid(T) );
        if( _miCache.end() == it
         || it->second.positionInMetaInfo == USHRT_MAX ) {
            return false;
        }
        it->second.collectorPtr->unpack_suppinfo(
                    _metainfo( it->second.positionInMetaInfo ).suppinfo() );
        return true;
    }
};

/**@brief A compressed bucket reader.
 * @class CompressedBucketReader
 *
 * A class incapsulating decompression for buckets compressed inside the
 * DeflatedBucket data structure. Will automatically apply decompression
 * for data when bucket instance will be necessary to acquire.
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

    /// For descendants --- mutable compressed bucket getter. Do not forget to
    /// mark current decompressed bucket as invalid with
    /// invalidate_decompressed_bucket_cache() after making changes.
    events::DeflatedBucket * _mutable_compressed_bucket_ptr()
                                                    { return _dfltdBucketPtr; }
public:
    CompressedBucketReader( events::DeflatedBucket *,
                            events::Bucket *,
                            events::BucketInfo *,
                            const Decompressors * );

    /// Returns reference to associated compressed bucket isntance.
    const events::DeflatedBucket & compressed_bucket() const;

    /// Returns true when bucket pointer was associated with this handle.
    virtual bool is_compressed_bucket_set() const { return _dfltdBucketPtr; }

    /// _decompressedBucketValid getter.
    bool is_decompressed_bucket_valid() const { return _decompressedBucketValid; }

    /// Marks bucket cache as invalid before forwarding invokation to parent.
    virtual void set_bucket_ptr( events::Bucket * ptr );

    /// Sets compressed bucket pointer to handle the provided instance.
    virtual void set_compressed_bucket_ptr( events::DeflatedBucket * );

    /// Marks decompressed bucket cache as invalid.
    void invalidate_decompressed_bucket_cache()
        { _decompressedBucketValid = false; }
};


/**@class BucketStreamReader
 * @brief A class wrapping std::istream providing compressed bucket reading
 *        interface.
 * */
template<typename BucketIDT,
         typename BucketKeyInfoT>
class BucketStreamReader : public CompressedBucketReader {
public:
    typedef BucketIDT BucketID;
    typedef BucketKeyInfoT BucketKeyInfo;
    typedef std::unordered_multimap<BucketID, std::istream::streampos> OffsetsMap;
    typedef CompressedBucketReader Parent;
private:
    /// Cache keeping physical offsets of buckets with certain ID.
    OffsetsMap _offsetsMap;

    /// Iterator pointing to current entry in offsets cache.
    typename OffsetsMap::const_iterator _cBucketIt;

    /// Pointer to stream.
    std::istream * _iStreamPtr;

    /// Internal buffer for reading byte sequence.
    std::vector<uint8_t> _readingBuffer;

    BucketKeyInfoT * _bucketKeyInfoMsg;
protected:
    std::istream & stream();

    virtual bool _V_is_good() override;
    virtual void _V_next_event( Event *& ) override;
    virtual Event * _V_initialize_reading() override;
    virtual void _V_finalize_reading() override;

    /// Adds new bucket position entry to cache.
    void _emplace_bucket_offset( const BucketID &, std::istream::streampos );

    /// Performs iterating of the entire stream from the beginning collecting
    /// physical offsets for each bucket. Performance heavy, need for
    /// random-access routines only.
    void _build_offsets_map();

    /// Performs reading of next bucket.
    virtual void _V_acquire_next_bucket();
public:
    /// Generic ctr.
    BucketStreamReader( events::DeflatedBucket *,
                        events::Bucket *,
                        BucketKeyInfoT *,
                        const Decompressors *,
                        std::istream * is=nullptr );

    ~BucketStreamReader();

    /// Returns true, if stream is set for this instance.
    bool is_stream_set() const { return _iStreamPtr; }

    /// Associates the stream with this instance. Causes re-caching of offsets
    /// map if it was not provided.
    void set_stream( std::istream &, const OffsetsMap & );

    const OffsetsMap & offsets_map() const;

    /// Prints human-readable excerpt of buckets available in the stream.
    void dump_offsets_map( std::ostream & );
};

// BucketStreamReader
////////////////////

template<typename BucketIDT, typename BucketKeyInfoT>
BucketStreamReader<BucketIDT, BucketKeyInfoT>::BucketStreamReader(
                        events::DeflatedBucket * dfltBcktPtr,
                        events::Bucket * bcktPtr,
                        BucketKeyInfoT * keyMsgPtr,
                        const Decompressors * dcmprssrsMap,
                        std::istream * is ) :
        CompressedBucketReader(dfltBcktPtr, bcktPtr, dcmprssrsMap),
        _iStreamPtr(is),
        _bucketKeyInfoMsg(keyMsgPtr) {}

template<typename BucketIDT, typename BucketKeyInfoT> std::istream &
BucketStreamReader<BucketIDT, BucketKeyInfoT>::stream() {
    if( !is_stream_set() ) {
        emraise( badState, "Stream is not associated with reader "
            "instance %p.", this );
    }
    return *_iStreamPtr;
}

template<typename BucketIDT, typename BucketKeyInfoT> void
BucketStreamReader<BucketIDT, BucketKeyInfoT>::_emplace_bucket_offset(
            const BucketID & bid,
            std::istream::streampos sPos ) {
    _offsetsMap.emplace( bid, sPos );
}

template<typename BucketIDT, typename BucketKeyInfoT> void
BucketStreamReader<BucketIDT, BucketKeyInfoT>::_build_offsets_map() {
    offsets_map().clear();

    bool parsingResult;
    size_t nRead;
    uint32_t bucketSize,
             suppInfoSize,
             nBucket = 0;
    stream().seekg( 0, stream().beg );
    while( *_iStreamPtr ) {
        stream().read( (char*)(&bucketSize), sizeof(uint32_t) );
        stream().read( (char*)(&suppInfoSize), sizeof(uint32_t) );

        _readingBuffer.resize( suppInfoSize );
        nRead = stream().read( (char*) _readingBuffer.data(), suppInfoSize );
        if( suppInfoSize != nRead ) {
            emraise( ioError, "Unable to read bucket supp. info of size %u "
                "from stream %p. Read %zu bytes.", suppInfoSize, _iStreamPtr,
                nRead )
        }
        parsingResult = supp_info_entries().ParseFromArray( _readingBuffer.data(), suppInfoSize );
        if( parsingResult ) {
            emraise( thirdParty, "protobuf unable to parse bucket supp. info of size %u "
                "from stream %p.", suppInfoSize, _iStreamPtr )
        }

        std::istream::streampos cBucketOffset = stream().tellg();

        # if 0
        _readingBuffer.resize(bucketSize);
        nRead = _iStreamPtr->read( (char*) _readingBuffer.data(), bucketSize );
        parsingResult = compressed_bucket().ParseFromArray( _readingBuffer.data(), bucketSize );
        # endif

        if( ! supp_info_entry( *_bucketKeyInfoMsg ) ) {
            sV_logw( "Bucket %zu of stream %p has no supp. info of type %s that "
                     "was expected for bucket key type %s. Bucket won't be indexed.\n",
                nBucket, _iStreamPtr,
                _bucketKeyInfoMsg->GetTypeName().c_str(),
                typeid(BucketID).name() );
        }
        _emplace_bucket_offset( *_bucketKeyInfoMsg, cBucketOffset );
        ++nBucket;

        stream().seekg( bucketSize, stream().cur );
    }
}

template<typename BucketIDT, typename BucketKeyInfoT> bool
BucketStreamReader<BucketIDT, BucketKeyInfoT>::_V_is_good() {
    return Parent::_V_is_good() && _cBucketIt != _offsetsMap.end();
}

template<typename BucketIDT, typename BucketKeyInfoT> void
BucketStreamReader<BucketIDT, BucketKeyInfoT>::_V_next_event( BucketReader::Event *& epr ) {
    if( Parent::_V_is_good() ) {
        return Parent::_V_next_event( epr );
    }
    _V_acquire_next_bucket();
    epr = Parent::_V_initialize_reading();
}

template<typename BucketIDT, typename BucketKeyInfoT> BucketReader::Event *
BucketStreamReader<BucketIDT, BucketKeyInfoT>::_V_initialize_reading() {
    _cBucketIt = offsets_map().begin();
    Event * ePtr;
    _V_next_event( ePtr );
    return ePtr;
}

template<typename BucketIDT, typename BucketKeyInfoT> void
BucketStreamReader<BucketIDT, BucketKeyInfoT>::_V_finalize_reading() {
    _cBucketIt = offsets_map.end();
    Parent::_V_finalize_reading();
}


template<typename BucketIDT, typename BucketKeyInfoT> void
BucketStreamReader<BucketIDT, BucketKeyInfoT>::_V_acquire_next_bucket() {
    _TODO_  // TODO: ...
}

}  // namespace ::sV::buckets

}  // namespace sV

//
//
//

namespace sV {
namespace aux {

struct SHA256BucketHash {
    uint32_t hash[8];
    friend std::ostream & operator<<(std::ostream& stream, const SHA256BucketHash &);
};

}  // namespace ::sV::aux
}  // namespace sV


//
// Template specialization for SHA256 hash used as map key
namespace std {
template<>
struct hash<::sV::aux::SHA256BucketHash> {
    /// Implements murmur3 hash function around SHA256 with hardcoded seed.
    size_t operator()( const ::sV::aux::SHA256BucketHash & sha256 ) const;
};
template<>
struct equal_to<::sV::aux::SHA256BucketHash> {
    /// Performs direct comparison.
    size_t operator()( const ::sV::aux::SHA256BucketHash & sha256l,
                       const ::sV::aux::SHA256BucketHash & sha256r ) const;
};
}  // namespace std

namespace sV {
/**@class Buckets
 * @brief Compressed buckets streaming handle available as sV events source.
 *
 * Offers a wrapper built around multiple STL streams providing random access
 * to serialized deflated buckets and their supplementary info indexed
 * by SHA256 hash.
 *
 * TODO: At this level of abstraction we still have no implications about event
 *       ID, isn't it?
 * */
class Buckets : public buckets::BucketStreamReader<
                                aux::SHA256BucketHash,
                                events::CommonBucketDescriptor>,
                public AbstractApplication::ASCII_Entry {
public:
    typedef buckets::BucketStreamReader< aux::SHA256BucketHash,
                                         events::CommonBucketDescriptor> Parent;
private:
    // ...
protected:
public:
    Buckets( events::DeflatedBucket * dfltdBcktPtr,
             events::Bucket * bucketPtr,
             events::BucketInfo * bucketInfoPtr,
             const Decompressors * decompressors );
    Buckets( const goo::dict::Dictionary & );
};  // class BucketsFile
}  // namespace sV

# endif  // RPC_PROTOCOLS

# endif  // H_STROMA_V_SVBP_READER_H

