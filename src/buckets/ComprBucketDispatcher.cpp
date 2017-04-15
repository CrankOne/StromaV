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
# include "buckets/ComprBucketDispatcher.hpp"

# ifdef RPC_PROTOCOLS

# include <goo_exception.hpp>
# include <iostream>

namespace sV {

ComprBucketDispatcher::ComprBucketDispatcher( iCompressor * compressor,
        std::ostream & streamRef,
        size_t nMaxKB,
        size_t nMaxEvents,
        size_t bufSizeKB) :
    iBucketDispatcher( nMaxKB,
                       nMaxEvents ),
    _compressor(compressor),   // possible SEGFAULT?
    _bufSizeKB(bufSizeKB),
    _streamRef(streamRef) {

    if ( nMaxKB > bufSizeKB) {
        emraise(badState, "Buffer Size is insufficient (is lesser than \
            nMaxKB bucket size): %zu < %zu.", bufSizeKB, nMaxKB);
    }
    _uncomprBuf = alloc_buffer(_uncomprBuf, 1024*_bufSizeKB);
    _comprBuf = alloc_buffer(_uncomprBuf, 1024*_bufSizeKB);
}

ComprBucketDispatcher::~ComprBucketDispatcher() {
    drop_bucket();
    clear_buffer( _uncomprBuf);
    clear_buffer( _comprBuf );
}

size_t ComprBucketDispatcher::compress_bucket() {
    return _compressor->compress_series( _uncomprBuf,
                _currentBucket.ByteSize(),
                _comprBuf,
                _bufSizeKB * 1024);
}

void ComprBucketDispatcher::set_metainfo() {
    _deflatedBucket.mutable_metainfo()->set_comprmethod(
            _compressor->compr_method() );
    // TODO optional other meta
}

size_t ComprBucketDispatcher::_V_drop_bucket() {

    size_t bucketSize = _currentBucket.ByteSize();
    // check either current bucketSize larger than expected
    if (bucketSize > 1024*_bufSizeKB ) {
        emraise(badState, "Buffer Size is insufficient (is lesser than \
            current bucket size): %zu KB < %zu B.", _bufSizeKB, bucketSize);
    }
    _currentBucket.SerializeToArray( _uncomprBuf, bucketSize );

    size_t comprBufSize = compress_bucket();

    _deflatedBucket.set_deflatedcontent(_comprBuf, comprBufSize);
    // XXX std::cout << "Compressor buf size: " << comprBufSize << std::endl;
    set_metainfo();
    // TODO temporary string compr method for check
    if ( _streamRef.good() ) {
        // Write size of the bucket to be dropped into output file
        size_t deflatedBucketSize = _deflatedBucket.ByteSize();
        _streamRef.write((char*)(&deflatedBucketSize), sizeof(uint32_t));
        // Then write the bucket
        if (!_deflatedBucket.SerializeToOstream(&_streamRef)) {
            std::cerr << "Failed to serialize into stream." << std::endl;
            return EXIT_FAILURE;
        }
        else {
        }
    }
    else {
        std::cerr << "Stream for serialized output isn't good." << std::endl;
        return EXIT_FAILURE;
    }
    clear_bucket();
    return bucketSize;;
}

uint8_t * ComprBucketDispatcher::alloc_buffer(uint8_t * buf,
        const size_t & size) {
    buf = new uint8_t[size];
    return buf;
}

uint8_t * ComprBucketDispatcher::realloc_buffer(uint8_t * buf,
        const size_t & size) {
    delete [] buf;
    buf = new uint8_t[size];
    return buf;
}

void ComprBucketDispatcher::clear_buffer(uint8_t * buf) {
    delete [] buf;
}

}  // namespace sV
# endif  // RPC_PROTOCOLS

