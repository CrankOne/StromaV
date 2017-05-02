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
# include "buckets/compressed_bdsp.hpp"

# ifdef RPC_PROTOCOLS

# include <goo_exception.hpp>
# include <iostream>

namespace sV {

CompressedBucketDispatcher::CompressedBucketDispatcher(
                iCompressor * compressor,
                std::ostream * streamPtr,
                size_t nMaxKB,
                size_t nMaxEvents ) :
                                    iBucketDispatcher( nMaxKB, nMaxEvents ),
                                    _compressor( compressor ),
                                    _streamPtr( streamPtr )
                                    // TODO: _deflatedBucket, using arena...
                                    {
    if( nMaxKB ) {
        _srcBuffer = alloc_buffer( _srcBfSize = 1024*nMaxKB );
        _dstBuffer = alloc_buffer( _dstBfSize = 1024*nMaxKB );
    } else {
        _srcBuffer = _dstBuffer = nullptr;
        _srcBfSize = _dstBfSize = 0;
    }
}

CompressedBucketDispatcher::~CompressedBucketDispatcher() {
    drop_bucket();
    clear_buffer( _srcBuffer );
    clear_buffer( _dstBuffer );
}

size_t CompressedBucketDispatcher::compress_bucket() {
    const size_t bucketSize = bucket().ByteSize();
    bucket().SerializeToArray( _srcBuffer, bucketSize );
    size_t compressedSize = _compressor->compress_series(
                _srcBuffer,     bucketSize,
                _dstBuffer,     _dstBfSize );
    _deflatedBucket.set_deflatedcontent( _dstBuffer, compressedSize );
    return compressedSize;
}

void CompressedBucketDispatcher::set_metainfo() {
    _deflatedBucket.mutable_metainfo()->set_comprmethod(
            _compressor->compr_method() );
    // TODO optional other meta
}

size_t CompressedBucketDispatcher::_V_drop_bucket() {
    compress_bucket();
    set_metainfo();

    // Write size of the bucket to be dropped into output file
    size_t deflatedBucketSize = _deflatedBucket.ByteSize();
    _streamPtr->write((char*)(&deflatedBucketSize), sizeof(uint32_t));
    // Then write the bucket
    if (!_deflatedBucket.SerializeToOstream(_streamPtr)) {
        emraise( ioError, "protobuf failed to serialize deflated bucket "
            "of size %zu to stream %p.",
            _deflatedBucket.ByteSize(), _streamPtr );
    }
    _deflatedBucket.Clear();

    return bucket().ByteSize();
}

uint8_t *
CompressedBucketDispatcher::alloc_buffer( const size_t size ) {
    return new uint8_t [size];
}

void
CompressedBucketDispatcher::realloc_buffer( uint8_t *& buf,
                                            const size_t size) {
    clear_buffer( buf );
    buf = alloc_buffer( size );
}

void CompressedBucketDispatcher::clear_buffer( uint8_t *& buf ) {
    delete [] buf;
    buf = nullptr;
}

}  // namespace sV
# endif  // RPC_PROTOCOLS

