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
# ifndef H_STROMA_V_COMPR_BUCKET_DISPATCHER_H
# define H_STROMA_V_COMPR_BUCKET_DISPATCHER_H

# include "sV_config.h"

# ifdef RPC_PROTOCOLS

# include "buckets/iBucketDispatcher.hpp"
# include "compression/iCompressor.hpp"

# include <ostream>

namespace sV {

class CompressedBucketDispatcher : public iBucketDispatcher {
private:
    iCompressor * _compressor;
    uint8_t * _srcBuffer,
            * _dstBuffer
            ;

    size_t  _srcBfSize,
            _dstBfSize
            ;
    std::ostream * _streamPtr;
    events::DeflatedBucket * _deflatedBucketPtr;

    struct {
        size_t rawLen, compressedLen;
    } _latestDrop;
protected:
    virtual size_t _V_drop_bucket() override;

    virtual size_t _compress_bucket();
    virtual uint8_t * _alloc_buffer( const size_t size );
    virtual void _realloc_buffer( uint8_t *& buf, const size_t size );
    virtual void _clear_buffer( uint8_t *& buf );
    virtual void _set_metainfo();
protected:
    CompressedBucketDispatcher( iCompressor * compressorPtr,
                                std::ostream * streamPtr,
                                size_t nMaxKB, size_t nMaxEvents );
public:
    CompressedBucketDispatcher( iCompressor * compressorPtr,
                                std::ostream & streamRef,
                                size_t nMaxKB=0, size_t nMaxEvents=0 ) :
            CompressedBucketDispatcher( compressorPtr, &streamRef, nMaxKB, nMaxEvents ) {}

    CompressedBucketDispatcher( iCompressor * compressorPtr,
                                size_t nMaxKB=0, size_t nMaxEvents=0 ) :
            CompressedBucketDispatcher( compressorPtr, nullptr, nMaxKB, nMaxEvents ) {}

    virtual ~CompressedBucketDispatcher();

    void set_out_stream( std::ostream & strRef ) { _streamPtr = &strRef; }
    std::ostream * get_out_stream_ptr() { return _streamPtr; }
    bool stream_is_set() const { return !!_streamPtr; }

    iCompressor & compressor() { return *_compressor; }

    size_t latest_dropped_raw_len() const { return _latestDrop.rawLen; }
    size_t latest_dropped_compressed_len() const { return _latestDrop.compressedLen; }
};  // class CompressedBucketDispatcher

}  //  namespace sV

# endif  //  RPC_PROTOCLS
# endif  //  H_STROMA_V_COMPR_BUCKET_DISPATCHER_H

