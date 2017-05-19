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
# include "buckets/compressedDispatcher.hpp"

# ifdef RPC_PROTOCOLS

# include "app/mixins/protobuf.hpp"

# include <goo_exception.hpp>
# include <iostream>

namespace sV {
namespace buckets {

CompressedDispatcher::CompressedDispatcher(
                iCompressor * compressorPtr,
                std::ostream * streamPtr,
                size_t nMaxKB,
                size_t nMaxEvents,
                bool doPackSuppinfo ) :
                                    Parent( nMaxKB, nMaxEvents, false ),
                                    _compressor( compressorPtr ),
                                    _streamPtr( streamPtr ),
                                    _deflatedBucketPtr(
                                        google::protobuf::Arena::CreateMessage<events::DeflatedBucket>(
                                            sV::mixins::PBEventApp::arena_ptr()) ),
                                    _latestDrop{0, 0},
                                    _doPackSuppInfo2(doPackSuppinfo) {
    assert(_deflatedBucketPtr);
}

//_srcBuffer = alloc_buffer( _srcBfSize = 1024*nMaxKB );
//_dstBuffer = alloc_buffer( _dstBfSize = 1024*nMaxKB );

CompressedDispatcher::~CompressedDispatcher() {
    drop_bucket();
}

size_t CompressedDispatcher::_compress_bucket() {
    _latestDrop.rawLen = bucket().ByteSize();
    if( !_latestDrop.rawLen ) {
        return 0;
    }
    // Realloc src buf if need:
    _raw_buffer_alloc( _latestDrop.rawLen );
    bucket().SerializeToArray( raw_buffer_data(), _latestDrop.rawLen );
    const size_t desiredOutSize =
            _compressor->compressed_dest_buffer_len( raw_buffer_data(), _latestDrop.rawLen );
    if( !desiredOutSize ) {
        emraise( badState, "Compressor instance ordered 0 length output "
            "buffer for %zu input bytes.", _latestDrop.rawLen );
    }
    _compressed_buffer_alloc( desiredOutSize );
    _latestDrop.compressedLen = _compressor->compress_series(
                raw_buffer_data(),          _latestDrop.rawLen,
                compressed_buffer_data(),   compressed_buffer_length() );
    _deflatedBucketPtr->mutable_data()->set_compressedcontent(
                        compressed_buffer_data(), _latestDrop.compressedLen );
    return _latestDrop.compressedLen;
}

size_t CompressedDispatcher::_V_drop_bucket() {
    _compress_bucket();
    _append_suppinfo( *(_deflatedBucketPtr->mutable_info()) );
    // Write size of the bucket to be dropped into output file
    size_t deflatedBucketSize = _deflatedBucketPtr->ByteSize();
    _streamPtr->write((char*)(&deflatedBucketSize), sizeof(uint32_t));
    // Then write the bucket
    if (!_deflatedBucketPtr->SerializeToOstream(_streamPtr)) {
        emraise( ioError, "protobuf failed to serialize deflated bucket "
            "of size %zu to stream %p.",
            _deflatedBucketPtr->ByteSize(), _streamPtr );
    }
    _deflatedBucketPtr->Clear();
    return bucket().ByteSize();
}

void
CompressedDispatcher::_compressed_buffer_alloc( size_t nBytes ) {
    _compressedBuffer.resize( nBytes );
}

void
CompressedDispatcher::_compressed_buffer_free() {
    _compressedBuffer.clear();
}

}  // namespace ::sV::buckets
}  // namespace sV

# endif  // RPC_PROTOCOLS

