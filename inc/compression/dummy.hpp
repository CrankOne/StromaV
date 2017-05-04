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
# ifndef H_STROMA_V_DUMMYCOMPRESSOR_H
# define H_STROMA_V_DUMMYCOMPRESSOR_H

# include "sV_config.h"

# ifdef RPC_PROTOCOLS

# include "iCompressor.hpp"
# include "iDecompressor.hpp"
# include "event.pb.h"

# include "ctrs_dict.hpp"

namespace sV {
namespace compression {

class DummyCompressor : public iCompressor {
protected:
    virtual size_t _V_compress_series(
        const uint8_t * uncomprBuf, size_t lenUncomprBuf,
        uint8_t * comprBuf, size_t ) const override;
public:
    DummyCompressor() : iCompressor(events::DeflatedBucketMetaInfo_CompressionMethod_UNCOMPRESSED) {}
    DummyCompressor( const goo::dict::Dictionary & );
    virtual ~DummyCompressor() {};
};  // class DummyCompressor


class DummyDecompressor : public iDecompressor {
protected:
    virtual size_t _V_decompress_series(
        uint8_t * uncomprBuf, size_t lenUncomprBuf,
        uint8_t * comprBuf, size_t ) const override;
public:
    DummyDecompressor( const goo::dict::Dictionary & );
    DummyDecompressor( ) :
        iDecompressor(events::DeflatedBucketMetaInfo_CompressionMethod_UNCOMPRESSED)
        { }
    virtual ~DummyDecompressor() {};
};  // class DummyDecompressor

}  // namespace compression
}  // namespace sV

# endif  // RPC_PROTOCOLS
# endif  // H_STROMA_V_DUMMYCOMPRESSOR_H

