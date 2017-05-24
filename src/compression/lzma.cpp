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
        iCompressor( events::CompressedData_CompressionAlgorithm_LZMA ),
        _extreme( dct["extreme"].as<bool>() ),
        _level( dct["level"].as<int>() ) {}

size_t
LZMACompression::_V_compress_series( const uint8_t * src, size_t srcLen,
                                     uint8_t * dst, size_t dstMaxLen ) const {
    size_t outPos = 0;
    auto rc = lzma_easy_buffer_encode(
            /* uint32_t preset .................... */ _level | (_extreme ? LZMA_PRESET_EXTREME : 0),
            /* lzma_check check ................... */ LZMA_CHECK_CRC32,
            /* const lzma_allocator * allocator ... */ NULL,
            /* const uint8_t * in ................. */ src,
            /* size_t in_size ..................... */ srcLen,
            /* uint8_t * out ...................... */ dst,
            /* size_t * out_pos ................... */ &outPos,
            /* size_t out_size .................... */ dstMaxLen
        );
    if( LZMA_OK != rc ) {
        switch( rc ) {
            case LZMA_BUF_ERROR:
                emraise( thirdParty, "LZMA compression error %d: not enough "
                    "output buffer space.", rc );
            case LZMA_UNSUPPORTED_CHECK:
                emraise( thirdParty, "LZMA compression error %d: unsupported "
                    "check.", rc );
            case LZMA_OPTIONS_ERROR:
                emraise( thirdParty, "LZMA compression error %d: options error."
                    , rc );
            case LZMA_MEM_ERROR:
                emraise( thirdParty, "LZMA compression error %d: memory error."
                    , rc );
            case LZMA_DATA_ERROR:
                emraise( thirdParty, "LZMA compression error %d: data error."
                    , rc );
            case LZMA_PROG_ERROR:
            default:
                emraise( thirdParty, "LZMA compression error %d: data error."
                    , rc );
        }
    }
    return outPos;
}

void
LZMACompression::_V_set_compression_info( events::CompressedData & cDatRef ) {
    cDatRef.mutable_lzma()->set_extreme( _extreme );
    cDatRef.mutable_lzma()->set_level(   _level );
}

size_t
LZMACompression::_V_compressed_dest_buffer_len( const uint8_t *, size_t n ) const {
    return lzma_stream_buffer_bound( n );
}

//
// LZMADecompression
///////////////////

LZMADecompression::LZMADecompression( const goo::dict::Dictionary & dct ) :
            iDecompressor( events::CompressedData_CompressionAlgorithm_LZMA ),
            _initialMemlimitKB( dct["memlimitKB"].as<size_t>() ) {}

size_t
LZMADecompression::_V_decompress_series(   const uint8_t * input, size_t inLen,
                                            uint8_t * output, size_t outMaxLen ) const {
    uint64_t memlimit = _initialMemlimitKB*1024;
    size_t inPos = 0,
           outPos = 0
           ;
    auto rc = lzma_stream_buffer_decode(
            /* uint64_t * memlimit ................ */ &memlimit,
            /* uint32_t flags ..................... */ 0x0,
            /* const lzma_allocator * allocator ... */ NULL,
            /* const uint8_t * in ................. */ input,
            /* size_t * in_pos .................... */ &inPos,
            /* size_t in_size ..................... */ inLen,
            /* uint8_t * out ...................... */ output,
            /* size_t * out_pos ................... */ &outPos,
            /* size_t out_size .................... */ outMaxLen
        );
    if( LZMA_OK != rc ) {
        switch( rc ) {
            case LZMA_FORMAT_ERROR :
                emraise( thirdParty, "LZMA decompression error %d: "
                    "LZMA_FORMAT_ERROR." , rc );
            case LZMA_OPTIONS_ERROR:
                emraise( thirdParty, "LZMA decompression error %d: "
                    "LZMA_OPTIONS_ERROR." , rc );
            case LZMA_DATA_ERROR:
                emraise( thirdParty, "LZMA decompression error %d: "
                    "LZMA_DATA_ERROR." , rc );
            case LZMA_NO_CHECK:
                emraise( thirdParty, "LZMA decompression error %d: "
                    "LZMA_NO_CHECK." , rc );
            case LZMA_UNSUPPORTED_CHECK:
                emraise( thirdParty, "LZMA decompression error %d: "
                    "LZMA_UNSUPPORTED_CHECK." , rc );
            case LZMA_MEM_ERROR:
                emraise( thirdParty, "LZMA decompression error %d: "
                    "LZMA_MEM_ERROR." , rc );
            case LZMA_MEMLIMIT_ERROR:
                emraise( thirdParty, "LZMA decompression error %d: "
                    "Memory usage limit was reached." , rc );
            case LZMA_BUF_ERROR:
                emraise( thirdParty, "LZMA decompression error %d: "
                    "Output buffer was too small." , rc );
            case LZMA_PROG_ERROR:
            default:
                emraise( thirdParty, "LZMA decompression error %d.", rc );
        }
    }
    return outPos;
}

}  // namespace compression
}  // namespace sV

using sV::compression::LZMACompression;
StromaV_COMPRESSOR_DEFINE_MCONF( LZMACompression, "lzma" ) {
    goo::dict::Dictionary LZMACmprssnDct( "lzma-compression",
        "LZMA data compressor parameters. This implementation is based on API "
        "provided within XZ utils package and uses simple public API for "
        "in-memory compression exposed in lzma/container.h header." );
    LZMACmprssnDct.insertion_proxy()
        .p<int>( "level",
            "Select a compression preset level [0..9]. The selected compression "
            "settings determine the memory requirements of the decompressor, "
            "thus using a too high preset level might make it painful to "
            "decompress the data on an old system with little RAM.",
            LZMA_PRESET_DEFAULT & LZMA_PRESET_LEVEL_MASK )
        .flag( "extreme",
            "This flag modifies the preset to make the encoding significantly slower"
            "while improving the compression ratio only marginally. This is useful"
            "when you don't mind wasting time to get as small result as possible." )
        ;
    goo::dict::DictionaryInjectionMap injM;
        injM( "extreme",    "compressors.lzma.false"  )
            ( "level",      "compressors.lzma.level"  )
            ;
    return std::make_pair( LZMACmprssnDct, injM );
}

using sV::compression::LZMADecompression;
StromaV_DECOMPRESSOR_DEFINE_MCONF( LZMADecompression, "lzma" ) {
    goo::dict::Dictionary LZMADcmprssnDct( "lzma-decompression",
        "LZMA data decompressor parameters. This implementation is based on API "
        "provided within XZ utils package and uses simple public API for "
        "in-memory compression exposed in lzma/container.h header." );
    LZMADcmprssnDct.insertion_proxy()
        .p<size_t>( "memlimitKB",
                "Memory limit for LZMA decompression internals in KB.",
            204500 )
        ;
    goo::dict::DictionaryInjectionMap injM;
        injM( "memlimitKB",      "compressors.bzip2.memlimitKB" )
            ;
    return std::make_pair( LZMADcmprssnDct, injM );
}

# endif  // LIBLZMA_FOUND



