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
# include "compression/dummy.hpp"
# ifdef RPC_PROTOCOLS

namespace sV {

//
// Compressor

//namespace events {
//    typedef DeflatedBucketMetaInfo_CompressionMethod_UNCOMPRESSED UNCOMPRESSED;
//}

DummyCompressor::DummyCompressor() :
    iCompressor(events::DeflatedBucketMetaInfo_CompressionMethod_UNCOMPRESSED) {
}

size_t DummyCompressor::_V_compress_series( uint8_t * uncomprBuf,
            size_t lenUncomprBuf, uint8_t * comprBuf,
            size_t ) const {
    memcpy( comprBuf, uncomprBuf, lenUncomprBuf);
    // _V_compress_series() should return real length of compressed series
    // (not lenComprBuf, because lenComprBuf is a allocated memory for
    // compressed data and real size of this compressed data could be
    // different).
    // Here it returns lenUncomprBuf, cause in fact DummyCompressor doesn't
    // compress series and in this case real length equal to lenUncomprBuf.
    return lenUncomprBuf;
}


//
// Decompressor

DummyDecompressor::DummyDecompressor() :
    iDecompressor(events::DeflatedBucketMetaInfo_CompressionMethod_UNCOMPRESSED)
    {
}

size_t DummyDecompressor::_V_decompress_series(uint8_t * uncomprBuf,
            size_t /*lenUncomprBuf*/, uint8_t * comprBuf,
            size_t lenComprBuf) const {
    memcpy( uncomprBuf, comprBuf, lenComprBuf);
    // _V_decompress_series() should return real length of decompressed series
    // (not lenUncomprBuf, because lenUncomprBuf is a allocated memory for
    // decompressed data and real size of this decompressed data could be
    // different).
    // Here it returns lenComprBuf, cause in fact DummyDecompressor doesn't
    // decompress series and in this case real length equal to lenComprBuf.
    return lenComprBuf;
}

}  // namespace sV
# endif  // RPC_PROTOCOLS

