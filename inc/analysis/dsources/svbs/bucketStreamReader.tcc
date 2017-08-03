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

# ifndef H_STROMA_V_SVBP_READER_BUCKET_STREAM_READER_H
# define H_STROMA_V_SVBP_READER_BUCKET_STREAM_READER_H

# include "sV_config.h"

# ifdef RPC_PROTOCOLS

# include "analysis/dsources/svbs/compressedBucketReader.hpp"

namespace sV {
namespace buckets {

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

    /// Associates the stream with this instance. Causes invalidation of all
    /// related caches.
    void set_stream( std::istream & );

    const OffsetsMap & offsets_map() const;

    /// Prints human-readable excerpt of buckets available in the stream.
    //void dump_offsets_map( std::ostream & );

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
        CompressedBucketReader(dfltBcktPtr, bcktPtr, bInfo, dcmprssrsMap ),
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
        sV_mylog2( "Bucket %s events indexed by offset %zu.\n",
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
    sV_mylog2( "BucketStreamReader<...> %p: current bucket depleted. Acquiring "
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
        sV_mylog2( "BucketStreamReader<...> %p: Unable to read from stream %p "
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
        sV_mylog2( "BucketStreamReader<...> %p: omitting reading of supp. info.\n",
            this );
    }
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
        sV_mylog2( "BucketStreamReader<...> %p: new bucket at position %zu indexed.\n",
                this, cBucketOffset);
    }
    sV_mylog3( "BucketStreamReader<...> %p: parsed supp info header of "
            "size %u.\n",
            this, suppInfoLength );
    if( !read_bucket( stream(), bucketLength ) ) {
        emraise( thirdParty, "Bucket reader %p was unable to parse compressed "
                "bucket from stream %p.", this, _iStreamPtr  );
    }
    epr = Parent::_V_initialize_reading();
    sV_mylog3( "BucketStreamReader<...> %p: read %u/%u bytes: bucket/supp. info.\n",
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
    sV_mylog3( "BucketStreamReader<...> %p: read %d supp info caches from "
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

template<typename BucketIDT, typename BucketKeyInfoT> void
BucketStreamReader<BucketIDT, BucketKeyInfoT>::set_stream( std::istream & newStreamRef ) {
    invalidate_supp_info_caches();
    invalidate_decompressed_bucket_cache();
    _offsetsMap.clear();
    _cBucketIt = offsets_map().end();
    _iStreamPtr = &newStreamRef;
}

}  // namespace ::sV::buckets
}  // namespace sV

# endif  // RPC_PROTOCOLS

# endif  // H_STROMA_V_SVBP_READER_BUCKET_STREAM_READER_H

