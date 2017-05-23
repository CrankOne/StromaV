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

# include "compression/lzma.hpp"

# ifdef LIBLZMA_FOUND

namespace sV {
namespace compression {

//
// LZMACompression
/////////////////

LZMACompression::LZMACompression( const goo::dict::Dictionary & dct ) :
        iCompressor( events::CompressedData_CompressionAlgorithm_LZMA ) {}

size_t
LZMACompression::_V_compress_series( const uint8_t * src, size_t srcLen,
                                     uint8_t * dst, size_t dstMaxLen ) const {
    // ...
}

void
LZMACompression::_V_set_compression_info( events::CompressedData & cDatRef ) {
    // ...
}

size_t
LZMACompression::_V_compressed_dest_buffer_len( const uint8_t *, size_t n ) const {
    // ...
}

//
// LZMADecompression
///////////////////

LZMADecompression::LZMADecompression( const goo::dict::Dictionary & dct ) :
            iDecompressor( events::CompressedData_CompressionAlgorithm_LZMA ) {}

size_t
LZMADecompression::_V_decompress_series(   const uint8_t * input, size_t inLen,
                                            uint8_t * output, size_t outMaxLen ) const {
    // ...
}

}  // namespace compression
}  // namespace sV

using sV::compression::LZMACompression;
StromaV_COMPRESSOR_DEFINE_MCONF( LZMACompression, "lzma" ) {
    goo::dict::Dictionary LZMACmprssnDct( "lzma-compression",
        "..." );
    LZMACmprssnDct.insertion_proxy()
        // ...
        ;
    goo::dict::DictionaryInjectionMap injM;
        //injM( "blockSize100k",  "compressors.lzma.blockSize100k"   )
            ;
    return std::make_pair( LZMACmprssnDct, injM );
}

using sV::compression::LZMADecompression;
StromaV_DECOMPRESSOR_DEFINE_MCONF( LZMADecompression, "lzma" ) {
    goo::dict::Dictionary LZMADcmprssnDct( "lzma-decompression",
        "..." );
    LZMADcmprssnDct.insertion_proxy()
        // ...
        ;
    goo::dict::DictionaryInjectionMap injM;
        //injM( "verbosity",      "compressors.bzip2.verbosity"               )
            ;
    return std::make_pair( LZMADcmprssnDct, injM );
}

# endif  // LIBLZMA_FOUND



