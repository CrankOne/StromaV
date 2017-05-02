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

    events::Bucket _deflatedBucket;  // TODO: use arena?
protected:
    virtual size_t _V_drop_bucket() override;
    virtual size_t compress_bucket( const events::DeflatedBucket & );

    virtual uint8_t * alloc_buffer( const size_t size );
    virtual void realloc_buffer( uint8_t *& buf, const size_t size );
    virtual void clear_buffer( uint8_t *& buf );
    virtual void set_metainfo();
protected:
    CompressedBucketDispatcher( iCompressor * compressor,
                                std::ostream * streamPtr,
                size_t nMaxKB, size_t nMaxEvents );
public:
    CompressedBucketDispatcher( iCompressor * compressor,
                            std::ostream & streamRef,
                            size_t nMaxKB=0, size_t nMaxEvents=0 ) :
            CompressedBucketDispatcher( compressor, &streamRef, nMaxKB, nMaxEvents ) {}

    CompressedBucketDispatcher( iCompressor * compressor,
                                    size_t nMaxKB=0, size_t nMaxEvents=0 ) :
            CompressedBucketDispatcher( compressor, nullptr, nMaxKB, nMaxEvents ) {}

    virtual ~CompressedBucketDispatcher();

    void set_out_stream( std::ostream & strRef ) { _streamPtr = &strRef; }
    std::ostream * get_out_stream_ptr() { return _streamPtr; }
    bool stream_is_set() const { return !!_streamPtr; }
};  // class CompressedBucketDispatcher

}  //  namespace sV

# endif  //  RPC_PROTOCLS
# endif  //  H_STROMA_V_COMPR_BUCKET_DISPATCHER_H

