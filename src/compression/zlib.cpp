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

# include "compression/zlib.hpp"

# ifdef ZLIB_FOUND 

namespace sV {
namespace compression {

//
// ZLibCompression
/////////////////

void
ZLibCompression::init_zlib_stream( z_stream & strm ) {
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
}

ZLibCompression::ZLibCompression( const goo::dict::Dictionary & parameters ) :
        iCompressor( events::CompressedData_CompressionAlgorithm_ZLIB ) {
    const std::string strComprLvl = parameters["level"].as<std::string>();
    if( "Z_DEFAULT_COMPRESSION" == strComprLvl ) {
        _zLvl = Z_DEFAULT_COMPRESSION;
    } else if( "Z_BEST_SPEED" == strComprLvl ) {
        _zLvl = Z_BEST_SPEED;
    } else if( "Z_BEST_COMPRESSION" == strComprLvl ) {
        _zLvl = Z_BEST_COMPRESSION;
    } else if( "Z_NO_COMPRESSION" == strComprLvl ) {
        _zLvl = Z_NO_COMPRESSION;
    } else {
        _zLvl = atoi( strComprLvl.c_str() );
    }
    int rc;
    init_zlib_stream( _zstrm );
    rc = deflateInit( &_zstrm, _zLvl );
    if( rc != Z_OK ) {
        emraise( thirdParty, "Failed to initialize zlib stream for deflating. "
            "Z_ERRNO code: %d.", rc );
    }
}

size_t
ZLibCompression::_V_compress_series( const uint8_t * src, size_t srcLen,
                                    uint8_t * dst, size_t dstMaxLen ) const {
    int rc;

    if( deflateBound( &_zstrm, srcLen ) > dstMaxLen ) {
        emraise( thirdParty, "ZLib deflateBound() points that output buffer "
            "has to be >=%zu, but only %zu was supplied by user code.",
            deflateBound( &_zstrm, srcLen ), dstMaxLen );
    }

    _zstrm.avail_in = srcLen;
    _zstrm.next_in = const_cast<uint8_t *>(src);  // gah, kludge
    _zstrm.avail_out = dstMaxLen;
    _zstrm.next_out = dst;
    rc = deflate( &_zstrm, Z_FINISH );
    if( Z_STREAM_END != rc ) {
        emraise( thirdParty, "Failed to deflate series. deflate() returned "
            "Z_ERRNO code: %d.", rc );
    }
    if( 0 == _zstrm.avail_out ) {
        emraise( thirdParty, "ZLib compression failed to deflate() input "
            "series: requires more space for compression output buffer." );
    }
    rc = deflateReset( &_zstrm );
    if( Z_OK != rc ) {
        emraise( thirdParty, "Unable to reset zlib stream with "
            "deflateReset() Z_ERRNO code: %d.", rc );
    }
    return dstMaxLen - _zstrm.avail_out;
}

void
ZLibCompression::_V_set_compression_info( events::CompressedData & cDatRef ) {
    cDatRef.mutable_zlib()->set_level( _zLvl );
}

size_t
ZLibCompression::_V_compressed_dest_buffer_len( const uint8_t *, size_t n ) const {
    return deflateBound( const_cast<z_stream *>(&_zstrm), n );
}

//
// ZLibDecompression
///////////////////

ZLibDecompression::~ZLibDecompression() {
    int rc;
    if( Z_OK != (rc = inflateEnd( &_zstrm )) ) {
        sV_logw( "Unable to reset zlib stream with "
            "inflateEnd() Z_ERRNO code: %d.\n", rc );
    }
}

size_t
ZLibDecompression::_V_decompress_series( const uint8_t * input, size_t inLen,
                                         uint8_t * output, size_t outMaxLen ) const {
    int rc;

    ZLibCompression::init_zlib_stream( _zstrm );
    _zstrm.avail_in = 0;
    _zstrm.next_in = Z_NULL;
    rc = inflateInit( &_zstrm );

    _zstrm.avail_in = inLen;
    _zstrm.next_in = const_cast<Bytef*>(input);
    _zstrm.avail_out = outMaxLen;
    _zstrm.next_out = output;

    if( Z_STREAM_END != (rc = inflate( &_zstrm, Z_FINISH )) ) {
        switch( rc ) {
            case Z_NEED_DICT:
                emraise( thirdParty, "ZLib decompression failed to inflate. "
                    "Z_ERRNO code: %d, preset dictionary is needed at this "
                    "point.", rc );
            case Z_DATA_ERROR:
                emraise( thirdParty, "ZLib decompression failed to inflate. "
                    "Z_ERRNO code: %d, the input data was corrupted.", rc );
            case Z_STREAM_ERROR:
                emraise( thirdParty, "ZLib decompression failed to inflate. "
                    "Z_ERRNO code: %d, the stream structure was inconsistent.",
                    rc );
            case Z_MEM_ERROR:
                emraise( thirdParty, "ZLib decompression failed to inflate. "
                    "Z_ERRNO code: %d, there was not enough memory.",
                    rc );
            case Z_BUF_ERROR:
                emraise( thirdParty, "ZLib decompression failed to inflate. "
                    "Z_ERRNO code: %d, there was not enough room in the "
                    "output buffer.", rc );
            case Z_STREAM_END:
            default:
                emraise( thirdParty, "ZLib decompression failed to inflate. "
                    "Z_ERRNO code: %d.", rc );
        };
    }
    return outMaxLen - _zstrm.avail_out;
}

ZLibDecompression::ZLibDecompression( const goo::dict::Dictionary & ) :
    iDecompressor( events::CompressedData_CompressionAlgorithm_ZLIB ) {
}

}  // namespace compression
}  // namespace sV

using sV::compression::ZLibCompression;
StromaV_COMPRESSOR_DEFINE_MCONF( ZLibCompression, "zlib" ) {
    goo::dict::Dictionary zlibCmprssnDct( "zlib",
        "ZLib compressor algorithm with incapsulated reentrant stream "
        "instance and adjustible compression level." );
    zlibCmprssnDct.insertion_proxy().p<std::string>( "level",
            "Compression level for zlib's deflate() function. Values [0-9] "
            "are valid as well as following special string tokens: "
            "Z_DEFAULT_COMPRESSION [=-1, special value], Z_BEST_SPEED [=1], "
            "Z_BEST_COMPRESSION [=9], Z_NO_COMPRESSION [=0].",
        "Z_DEFAULT_COMPRESSION");
    goo::dict::DictionaryInjectionMap injM;
        injM( "level", "compressors.zlib.level" );
    return std::make_pair( zlibCmprssnDct, injM ); 
}

using sV::compression::ZLibDecompression;
StromaV_DECOMPRESSOR_DEFINE( ZLibDecompression, "zlib" ) {
    return goo::dict::Dictionary( NULL, "" ); 
}

# endif

