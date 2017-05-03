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

void
ZLibCompressor::init_zlib_stram( z_stream & strm ) {
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
}

ZLibCompressor::ZLibCompressor( const goo::dict::Dictionary & parameters ) :
        iCompressor( events::DeflatedBucketMetaInfo_CompressionMethod_ZLIB ) {
    int comprsnLvl;
    const std::string strComprLvl = parameters["level"].as<std::string>();
    if( "Z_DEFAULT_COMPRESSION" == strComprLvl ) {
        comprsnLvl = Z_DEFAULT_COMPRESSION;
    } else if( "Z_BEST_SPEED" == strComprLvl ) {
        comprsnLvl = Z_BEST_SPEED;
    } else if( "Z_BEST_COMPRESSION" == strComprLvl ) {
        comprsnLvl = Z_BEST_COMPRESSION;
    } else if( "Z_NO_COMPRESSION" == strComprLvl ) {
        comprsnLvl = Z_NO_COMPRESSION;
    } else {
        comprsnLvl = atoi( strComprLvl.c_str() );
    }
    int rc;
    init_zlib_stram( _zstrm );
    rc = deflateInit( &_zstrm, comprsnLvl );
    if( rc != Z_OK ) {
        emraise( thirdParty, "Failed to initialize zlib stream for deflating. "
            "Z_ERRNO code: %d.", rc );
    }
}

size_t
ZLibCompressor::_V_compress_series( const uint8_t * src, size_t srcLen,
                                    uint8_t * dst, size_t dstMaxLen ) const {
    int rc;

    if( deflateBound( &_zstrm, srcLen ) > dstMaxLen ) {
        emraise( thirdParty, "ZLib deflateBound() points that output buffer "
            "has to be >=%zu, but only %zu was supplied by user code.",
            deflateBound( &_zstrm, srcLen ), dstMaxLen );
    }

    _zstrm.avail_in = srcLen;
    _zstrm.next_in = const_cast<uint8_t *>(src);
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
    return _zstrm.avail_out;
}

}  // namespace compression
}  // namespace sV

using sV::compression::ZLibCompressor;
StromaV_COMPRESSOR_DEFINE_MCONF( ZLibCompressor, "zlib" ) {
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
        injM( "level", "compressor.zlib.level" );
    return std::make_pair( zlibCmprssnDct, injM ); 
}

# endif

