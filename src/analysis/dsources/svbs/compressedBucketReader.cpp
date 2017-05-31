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

# include "analysis/dsources/svbs/compressedBucketReader.hpp"

namespace sV {
namespace buckets {

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
        sV_mylog2( "Decompressing data of size %d with algorithm #%d.\n",
                _dfltdBucketPtr->data().compressedcontent().size(),
                _dfltdBucketPtr->data().compressionalgo() );
    }
    const iDecompressor * dcmprPtr = _decompressor(
                                    _dfltdBucketPtr->data().compressionalgo() );
    size_t decompressedLength;  // provisioned
    _dcmBuffer.resize( decompressedLength = _dfltdBucketPtr->data().originalsize() );
    size_t realDecompressedLength =
        dcmprPtr->decompress_series(
            (const uint8_t *) _dfltdBucketPtr->data().compressedcontent().c_str(),
            _dfltdBucketPtr->data().compressedcontent().size(),
            _dcmBuffer.data(),
            decompressedLength );
    if( !realDecompressedLength ) {
        sV_logw( "Decompression returned buffer of zero length. "
            "Possible errors ahead.\n" );
    }
    # if 0
        const uint8_t * dt = (const uint8_t *) _dfltdBucketPtr->data().compressedcontent().c_str();
        sV_mylog3( "%x %x %x ... [%zu] -decompression-> %x %x %x ... [%zu]\n",
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
    } else {
        sV_mylog2( "CompressedBucketReader %p: decompressed sizes: predicted=%zu, real=%zu.\n",
            this, decompressedLength, _dcmBuffer.size() );
    }
    sV_mylog3( "CompressedBucketReader %p: bucket of size %zu -> %zu "
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
    sV_mylog2( "CompressedBucketReader %p: bucket of size %zu parsed (%d events).\n",
        this, decompressedLength, bucket().events_size() );
    reset_bucket_iterator();
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

}  // namespace ::sV::buckets
}  // namespace sV

# endif  // RPC_PROTOCOLS


