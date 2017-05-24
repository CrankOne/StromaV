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
# include "logging.hpp"
# include "utils.h"

# include <goo_mixins/iterable.tcc>
# include <goo_dict/parameters/path_parameter.hpp>

# include <fstream>

namespace sV {
namespace aux {
struct SHA256BucketHash;
}  // namespace ::sV::aux
}  // namespace sV

namespace std {
std::ostream & operator<<(std::ostream& stream, const ::sV::aux::SHA256BucketHash &);
}

namespace sV {
namespace aux {

struct SHA256BucketHash {
    uint32_t hash[8];
    friend std::ostream & ::std::operator<<(std::ostream& stream, const SHA256BucketHash &);

    SHA256BucketHash( sV::events::CommonBucketDescriptor & msg ) {
        if( sizeof(hash) != msg.sha256hash().size() ) {
            emraise( badValue, "Expected length of SHA256 hash is 32 bytes "
                "while incoming protobuf message carries %d.",
                msg.sha256hash().size() );
        }
        memcpy( hash, msg.sha256hash().c_str(), sizeof(hash) );
    }
};

}  // namespace ::sV::aux
}  // namespace sV

namespace sV {

namespace buckets {

/**@brief Performs basic reading from events bucket.
 * @class BucketReader
 *
 * Provides basic implementation for reading events from bucket. This class
 * keeps track of single bucket associated with current instance and implements
 * basic iEventSequence interface for iterating it within sV's events pipeline
 * framework.
 *
 * This rudimentary implementation makes no assumption about bucket contents
 * and does not involve usage of any supplementary information within. For
 * supp. info handling reader, see SuppInfoBucketReader.
 *
 * @ingroup analysis
 * @ingroup buckets
 *
 * @see SuppInfoBucketReader
 */
class BucketReader : public virtual sV::aux::iEventSequence {
public:
    /// Parent interface class.
    typedef sV::aux::iEventSequence Parent;
    /// Unified events type.
    typedef typename sV::AnalysisPipeline::Event Event;
    /// Bucket event id. Used for custom iterator class.
    template<typename BucketT>
    struct BucketEventID {
        BucketT * authorityPtr;
        size_t nEvent;
        BucketEventID( BucketT * bPtr, size_t nEve=0 ) :
                                            authorityPtr(bPtr), nEvent(nEve) {}
        BucketEventID & operator++() { ++nEvent; return *this; }
    };
    /// Template declaration for i/o iterator related to bucket reader.
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
    /// Reading iterator.
    typedef _BucketIterator<const events::Event *, const events::Bucket>
                                                            ConstBucketIterator;
private:
    /// Reading bucket instance pointer.
    events::Bucket * _cBucket;
    /// Current iterator for reading associated bucket instance.
    mutable ConstBucketIterator _it;
protected:
    /// Reads true if iterator points to existing event instance inside a bucket.
    virtual bool _V_is_good() override;
    /// Reads next event using internal iterator.
    virtual void _V_next_event( Event *& ) override;
    /// Resets internal iterator and read first event returning pointer to it.
    virtual Event * _V_initialize_reading() override;
    /// Sets internal iterator to the end of associated buffer.
    virtual void _V_finalize_reading() override;
    /// This immutable bucket getter may be overriden by user classes that may
    /// implement some caching procedures.
    virtual const events::Bucket & _V_bucket() const;
    /// Mutable bucket getter. Just returns mutable bucket pointer.
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
    ConstBucketIterator begin() const
        { return ConstBucketIterator( &(bucket()) ); }
    /// Returns iterator pointing to the end of events.
    ConstBucketIterator end() const
        { return ConstBucketIterator( &(bucket()), bucket().events_size() ); }
    /// Event getter operator.
    const events::Event & operator[]( size_t n ) { return bucket().events(n); }
    /// Wil set internal bucket iterator to beginning.
    void reset_bucket_iterator() const;
};  // BucketReader


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
                             public aux::Logger {
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
    typedef std::unordered_map<int, iDecompressor *>
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
    virtual const events::BucketInfoEntry & _supp_info( uint16_t ) const override;

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
 *
 * The default implementation of this class implies that compressed (deflated)
 * buckets an their supplementary information lie sequentially in the stream
 * according to the following scheme:
 *      1. uint32_t byte length of the bucket block
 *      2. uint32_t byte length of the supplementary info block
 *      3. Block of serialized supp. info data
 *      4. Block of serialized DeflatedBucket
 * The reading logic is summed up in the _V_acquire_next_bucket() method.
 * */
template<typename BucketIDT,
         typename BucketKeyInfoT>
class BucketStreamReader : public CompressedBucketReader {
public:
    typedef BucketIDT BucketID;
    typedef BucketKeyInfoT BucketKeyInfo;
    typedef std::unordered_multimap<BucketID, std::istream::streampos> OffsetsMap;
    typedef CompressedBucketReader Parent;
    typedef BucketStreamReader<BucketID, BucketKeyInfo> Self;
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
    typename OffsetsMap::const_iterator
            _emplace_bucket_offset( const BucketID &, std::istream::streampos );

    /// Performs iterating of the entire stream from the beginning collecting
    /// physical offsets for each bucket. Performance heavy, need for
    /// random-access routines only.
    void _build_offsets_map( std::istream & is );

    /// Performs reading of next bucket.
    virtual bool _V_acquire_next_bucket( BucketReader::Event *& epr );
public:
    /// Generic ctr.
    BucketStreamReader( events::DeflatedBucket *,
                        events::Bucket *,
                        events::BucketInfo * bInfo,
                        BucketKeyInfoT *,
                        const Decompressors *,
                        std::istream * is=nullptr );

    ~BucketStreamReader() {}

    /// Returns true, if stream is set for this instance.
    bool is_stream_set() const { return !! _iStreamPtr; }

    /// Associates the stream with this instance. Causes re-caching of offsets
    /// map if it was not provided.
    void set_stream( std::istream &, const OffsetsMap & );

    const OffsetsMap & offsets_map() const;

    /// Prints human-readable excerpt of buckets available in the stream.
    void dump_offsets_map( std::ostream & );

    /// Reads the supp info message from given stream.
    bool read_supp_info( std::istream &, size_t );

    /// Reads the bucket from given stream.
    bool read_bucket( std::istream &, size_t );
};

// BucketStreamReader
////////////////////

template<typename BucketIDT, typename BucketKeyInfoT>
BucketStreamReader<BucketIDT, BucketKeyInfoT>::BucketStreamReader(
                        events::DeflatedBucket * dfltBcktPtr,
                        events::Bucket * bcktPtr,
                        events::BucketInfo * bInfo,
                        BucketKeyInfoT * keyMsgPtr,
                        const Decompressors * dcmprssrsMap,
                        std::istream * is ) :
        aux::iEventSequence(0x0),
        CompressedBucketReader(dfltBcktPtr, bcktPtr, bInfo, dcmprssrsMap),
        _iStreamPtr( is ),
        _bucketKeyInfoMsg(keyMsgPtr) {}

template<typename BucketIDT, typename BucketKeyInfoT> std::istream &
BucketStreamReader<BucketIDT, BucketKeyInfoT>::stream() {
    if( !is_stream_set() ) {
        emraise( badState, "Stream is not associated with reader "
            "instance %p.", this );
    }
    return *_iStreamPtr;
}

template<typename BucketIDT, typename BucketKeyInfoT>
typename BucketStreamReader<BucketIDT, BucketKeyInfoT>::OffsetsMap::const_iterator
BucketStreamReader<BucketIDT, BucketKeyInfoT>::_emplace_bucket_offset(
            const BucketID & bid,
            std::istream::streampos sPos ) {
    auto ir = _offsetsMap.emplace( bid, sPos );
    {
        std::stringstream ss;
        ss << bid;
        log_message( aux::Logger::verbose,
            "Bucket %s events indexed by offset %zu.\n",
            ss.str().c_str(), sPos );
    }
    return ir;
}

template<typename BucketIDT, typename BucketKeyInfoT>
const typename BucketStreamReader<BucketIDT, BucketKeyInfoT>::OffsetsMap &
BucketStreamReader<BucketIDT, BucketKeyInfoT>::offsets_map() const {
    return _offsetsMap;
}

template<typename BucketIDT, typename BucketKeyInfoT> void
BucketStreamReader<BucketIDT, BucketKeyInfoT>::_build_offsets_map( std::istream & is ) {
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

        if( suppInfoSize ) {
            read_supp_info( is, suppInfoSize );
        }

        std::istream::streampos cBucketOffset = stream().tellg();

        # if 0
        _readingBuffer.resize(bucketSize);
        nRead = _iStreamPtr->read( (char*) _readingBuffer.data(), bucketSize );
        parsingResult = compressed_bucket().ParseFromArray( _readingBuffer.data(), bucketSize );
        # endif

        if( ! supp_info_entry<buckets::GenericCollector>( *_bucketKeyInfoMsg ) ) {
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
    return _cBucketIt != _offsetsMap.end() && Parent::_V_is_good();
}

template<typename BucketIDT, typename BucketKeyInfoT> void
BucketStreamReader<BucketIDT, BucketKeyInfoT>::_V_next_event( BucketReader::Event *& epr ) {
    if( Self::_V_is_good() ) {
        return Parent::_V_next_event( epr );
    }
    log_message( aux::Logger::verbose,
            "BucketStreamReader<...> %p: current bucket depleted. Acquiring "
            "next...\n", this );
    _V_acquire_next_bucket( epr );
}

template<typename BucketIDT, typename BucketKeyInfoT> BucketReader::Event *
BucketStreamReader<BucketIDT, BucketKeyInfoT>::_V_initialize_reading() {
    Event * ePtr;
    _V_acquire_next_bucket( ePtr );
    return ePtr;
}

template<typename BucketIDT, typename BucketKeyInfoT> void
BucketStreamReader<BucketIDT, BucketKeyInfoT>::_V_finalize_reading() {
    _cBucketIt = offsets_map().end();
    Parent::_V_finalize_reading();
}


template<typename BucketIDT, typename BucketKeyInfoT> bool
BucketStreamReader<BucketIDT, BucketKeyInfoT>::_V_acquire_next_bucket( BucketReader::Event *& epr ) {
    invalidate_supp_info_caches();
    invalidate_decompressed_bucket_cache();

    uint32_t bucketLength,
             suppInfoLength;
    stream().read( (char*) &bucketLength,   sizeof(uint32_t) );
    if( !stream().good() ) {
        log_message( aux::Logger::verbose,
            "BucketStreamReader<...> %p: Unable to read from stream %p "
            "anymore.\n",
            this, _iStreamPtr );
        return false;
    }
    stream().read( (char*) &suppInfoLength, sizeof(uint32_t) );
    if( suppInfoLength ) {
        if( !read_supp_info( stream(), suppInfoLength ) ) {
            emraise( thirdParty, "Bucket reader %p was unable parse supp info from "
                "stream %p.", this, _iStreamPtr );
        }
    } else {
        log_message( aux::Logger::verbose,
            "BucketStreamReader<...> %p: omitting reading of supp. info.\n",
            this );
    }
    # if 1
    std::istream::streampos cBucketOffset = stream().tellg();
    if( ! supp_info_entry<GenericCollector>( *_bucketKeyInfoMsg ) ) {
        sV_logw( "Bucket at position %zu in stream %p has no supp. info of "
                 "type %s that was expected for bucket key type %s. Bucket "
                 "won't be indexed.\n",
            cBucketOffset, _iStreamPtr,
            //GenericCollector::CollectingType::GetTypeName().c_str(),
            typeid(GenericCollector::CollectingType).name(),
            typeid(BucketID).name() );
    } else if( offsets_map().find( BucketID(*_bucketKeyInfoMsg) )
            == offsets_map().end() ) {
        _cBucketIt = _emplace_bucket_offset( *_bucketKeyInfoMsg, cBucketOffset );
        log_message( aux::Logger::loquacious,
                "BucketStreamReader<...> %p: new bucket at position %zu indexed.\n",
                this, cBucketOffset);
    }
    # else
    _TODO_  // TODO
    # endif
    log_message( aux::Logger::loquacious,
            "BucketStreamReader<...> %p: parsed supp info header of "
            "size %u.\n",
            this, suppInfoLength );
    if( !read_bucket( stream(), bucketLength ) ) {
        emraise( thirdParty, "Bucket reader %p was unable to parse compressed "
                "bucket from stream %p.", this, _iStreamPtr  );
    }
    epr = Parent::_V_initialize_reading();
    log_message( aux::Logger::loquacious,
            "BucketStreamReader<...> %p: read %u/%u bytes: bucket/supp. info.\n",
            this, bucketLength, suppInfoLength );
    return true;
}

template<typename BucketIDT, typename BucketKeyInfoT> bool
BucketStreamReader<BucketIDT, BucketKeyInfoT>::read_supp_info( std::istream & is, size_t length ) {
    _readingBuffer.resize( length );
    /*size_t nRead = */is.read( (char*) _readingBuffer.data(), length );
    /*if( length != nRead ) {
        emraise( ioError, "Bucket reader %p was unable to read supp info from "
            "stream %p.", this, &is );
    }*/
    bool ret = mutable_supp_info_entries().ParseFromArray( _readingBuffer.data(), length );
    log_message( aux::Logger::loquacious,
        "BucketStreamReader<...> %p: read %d supp info caches from "
        "stream %p.\n", this, mutable_supp_info_entries().entries_size(), &is );
    return ret;
}

template<typename BucketIDT, typename BucketKeyInfoT> bool
BucketStreamReader<BucketIDT, BucketKeyInfoT>::read_bucket( std::istream & is, size_t length ) {
    _readingBuffer.resize( length );
    /*size_t nRead = */is.read( (char*) _readingBuffer.data(), length );
    /*if( length != nRead ) {
        emraise( ioError, "Bucket reader %p was unable to read compressed "
            "bucket info from stream %p.", this, &is );
    }*/
    if( _mutable_compressed_bucket_ptr()->ParseFromArray( _readingBuffer.data(), length ) ) {
        invalidate_decompressed_bucket_cache();
        return true;
    }
    return false;
}

}  // namespace ::sV::buckets

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
 * At this level of abstraction we still have no implications about event ID.
 * */
class Buckets : public buckets::BucketStreamReader<
                                aux::SHA256BucketHash,
                                events::CommonBucketDescriptor>,
                public AbstractApplication::ASCII_Entry {
public:
    typedef buckets::BucketStreamReader< aux::SHA256BucketHash,
                                         events::CommonBucketDescriptor> Parent;
private:
    /// List of sources paths.
    std::vector<goo::filesystem::Path> _paths;
    /// Indicates whether the last invokation of _V_next_event() performed
    /// successfully.
    bool _lastEvReadingWasGood;
    /// Progressbar parameters.
    PBarParameters * _pbParameters;  ///< set to nullptr when unused
    size_t _maxEventsNumber, ///< Maximum events number to read. 0 is for all available.
           _eventsRead;  ///< Counter of read events.
    /// Own container for aggregated collector instances performing supp. info
    /// deserialization by ancestor class (see SuppInfoBucketReader).
    mutable std::unordered_map<std::string, sV::buckets::iAbstractInfoCollector *> _collectors;
    /// Own container for aggregated decompressor instances involved into
    /// buckets decompression procedures used by ancestor class
    /// (see CompressedBucketReader).
    mutable Buckets::Decompressors _decompressors;
    /// Own instance referring to reading file source.
    std::ifstream _file;
    /// Iterator referring to current source in a list.
    std::vector<goo::filesystem::Path>::const_iterator _sourcesIt;
protected:
    /// Returns C++ RTTI type hash for given supp info data type using the sV's
    /// system VCtr dict.
    virtual std::type_index _V_get_collector_type_hash( const std::string & ) const override;
    /// Allocates new cache entry with set name and collector ptr fields.
    virtual MetaInfoCache _V_new_cache_entry( const std::string & ) const override;
    /// Returns true, while there are available sources remained in the list.
    virtual bool _V_is_good() override;
    /// Sets up reading from first source and reads the first available event.
    virtual Event * _V_initialize_reading() override;
    /// Causes re-acquizition of available bucket from stream. If there is no
    /// more buckets in stream, opens next in a list.
    virtual bool _V_acquire_next_bucket( BucketReader::Event *& epr ) override;
    /// Instead of raising an exception, creates a new decompressor.
    virtual const iDecompressor * _decompressor( iDecompressor::CompressionAlgo ) const override;
public:
    /// Common C++ ctr.
    Buckets( events::DeflatedBucket * dfltdBcktPtr,
             events::Bucket * bucketPtr,
             events::BucketInfo * bInfo,
             events::CommonBucketDescriptor * particularInfo,
             const std::list<goo::filesystem::Path> & paths,
             size_t nMaxEvents,
             bool enablePBar=false,
             uint8_t ASCII_lines=1);
    /// VCtr interfacing ctr.
    Buckets( const goo::dict::Dictionary & );
    /// Frees resources.
    ~Buckets();
};  // class BucketsFile
}  // namespace sV

# endif  // RPC_PROTOCOLS

# endif  // H_STROMA_V_SVBP_READER_H

