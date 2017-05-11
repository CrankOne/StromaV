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

# include "app/mixins/protobuf.hpp"

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
                                    _srcBuffer( nullptr ), _dstBuffer( nullptr ),
                                    _srcBfSize(0), _dstBfSize(0),
                                    _streamPtr( streamPtr ),
                                    _deflatedBucketPtr(
                                        google::protobuf::Arena::CreateMessage<events::DeflatedBucket>(
                                            sV::mixins::PBEventApp::arena_ptr()) ) {
    assert(_deflatedBucketPtr);
}

//_srcBuffer = alloc_buffer( _srcBfSize = 1024*nMaxKB );
//_dstBuffer = alloc_buffer( _dstBfSize = 1024*nMaxKB );

CompressedBucketDispatcher::~CompressedBucketDispatcher() {
    drop_bucket();
    if( _srcBuffer ) {
        _clear_buffer( _srcBuffer );
    }
    if( _dstBuffer ) {
        _clear_buffer( _dstBuffer );
    }
}

size_t CompressedBucketDispatcher::_compress_bucket() {
    const size_t origSize = bucket().ByteSize();
    assert( _srcBuffer );
    if( !origSize ) {
        return 0;
    }
    // Realloc src buf if need:
    if( origSize > _srcBfSize ) {
        _realloc_buffer( _srcBuffer, origSize );
        _srcBfSize = origSize;
    }
    bucket().SerializeToArray( _srcBuffer, origSize );
    const size_t desiredOutSize =
            _compressor->compressed_dest_buffer_len( _srcBuffer, origSize );
    if( !desiredOutSize ) {
        emraise( badState, "Compressor instance ordered 0 length output "
            "buffer for %zu input bytes.", origSize );
    }
    if( desiredOutSize > _dstBfSize ) {
        _realloc_buffer( _dstBuffer, desiredOutSize );
        _dstBfSize = desiredOutSize;
    }
    size_t compressedSize = _compressor->compress_series(
                _srcBuffer,     origSize,
                _dstBuffer,     _dstBfSize );
    _deflatedBucketPtr->set_deflatedcontent( _dstBuffer, compressedSize );
    return compressedSize;
}

void CompressedBucketDispatcher::_set_metainfo() {
    _deflatedBucketPtr->mutable_metainfo()->set_compressionalgo(
            _compressor->algorithm() );
    // TODO optional other meta!
}

size_t CompressedBucketDispatcher::_V_drop_bucket() {
    _compress_bucket();
    _set_metainfo();
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

uint8_t *
CompressedBucketDispatcher::_alloc_buffer( const size_t size ) {
    return new uint8_t [size];
}

void
CompressedBucketDispatcher::_realloc_buffer( uint8_t *& buf,
                                            const size_t size) {
    _clear_buffer( buf );
    buf = _alloc_buffer( size );
}

void CompressedBucketDispatcher::_clear_buffer( uint8_t *& buf ) {
    delete [] buf;
    buf = nullptr;
}

}  // namespace sV
# endif  // RPC_PROTOCOLS

