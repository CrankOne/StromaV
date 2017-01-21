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

# include "config.h"

# ifdef RPC_PROTOCOLS

# include "buckets/iBucketDispatcher.hpp"
# include "compr/iCompressor.hpp"
# include <ostream>

namespace sV {

class ComprBucketDispatcher : public iBucketDispatcher {

    public:
        ComprBucketDispatcher( iCompressor * compressor,
                std::ostream & streamRef,
                size_t nMaxKB, size_t nMaxEvents, size_t maxBufSizeKB );

        virtual ~ComprBucketDispatcher();

    protected:
        virtual size_t _V_drop_bucket() override;
        virtual size_t compress_bucket();

        virtual uint8_t * alloc_buffer( uint8_t * buf, const size_t & size );
        virtual uint8_t * realloc_buffer( uint8_t * buf, const size_t & size );
        virtual void clear_buffer( uint8_t * buf );
        virtual void set_metainfo();
        std::ostream & _streamRef;
    private:
        iCompressor * _compressor;
        events::DeflatedBucket _deflatedBucket;
        uint8_t * _uncomprBuf;
        uint8_t * _comprBuf;
        size_t _bufSizeKB;

};  // class ComprBucketDispatcher

}  //  namespace sV

# endif  //  RPC_PROTOCLS
# endif  //  H_STROMA_V_COMPR_BUCKET_DISPATCHER_H

