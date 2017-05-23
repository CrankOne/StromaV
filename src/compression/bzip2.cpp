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

# include "compression/bzip2.hpp"

# ifdef BZIP2_FOUND

namespace sV {
namespace compression {

//
// BZip2Compression
/////////////////

BZip2Compression::BZip2Compression( const goo::dict::Dictionary & dct ) :
        iCompressor( events::CompressedData_CompressionAlgorithm_BZIP2 ),
                    _blockSize100k( dct["blockSize100k"].as<int>() ),
                    _verbosity(     dct["verbosity"].as<int>() ),
                    _workFactor(    dct["workFactor"].as<int>() ) {}

size_t
BZip2Compression::_V_compress_series( const uint8_t * src, size_t srcLen,
                                      uint8_t * dst, size_t dstMaxLen ) const {
    unsigned int destLen = dstMaxLen;
    int rc = BZ2_bzBuffToBuffCompress(
        /*char*         dest .......... */  (char*) dst,
        /*unsigned int* destLen ....... */  &destLen,
        /*char*         source ........ */  (char*) const_cast<uint8_t *>(src),
        /*unsigned int  sourceLen ..... */  srcLen,
        /*int           blockSize100k . */  _blockSize100k,
        /*int           verbosity ..... */  _verbosity,
        /*int           workFactor .... */  _workFactor
        );
    if( BZ_OK != rc ) {
        switch( rc ) {
            case BZ_CONFIG_ERROR :
                emraise( thirdParty, "BZIP2 compression error: the library has "
                    "been mis-compiled.");
            case BZ_PARAM_ERROR :
                emraise( thirdParty, "BZIP2 compression error: dest is NULL or "
                    "destLen is NULL"
                    "or blockSize100k < 1 or blockSize100k > 9"
                    "or verbosity < 0 or verbosity > 4"
                    "or workFactor < 0 or workFactor > 250");
            case BZ_MEM_ERROR :
                emraise( thirdParty, "BZIP2 compression error: insufficient "
                    "memory is available (where %zu bytes).", dstMaxLen );
            case BZ_OUTBUFF_FULL :
                emraise( thirdParty, "BZIP2 compression error: the size of "
                    "the compressed data exceeds destination." );
            default:
                emraise( thirdParty, "BZIP2 compression error. "
                    "BZ2_bzBuffToBuffCompress() returned code %d.", rc );
        };
    }
    return destLen;
}

void
BZip2Compression::_V_set_compression_info( events::CompressedData & cDatRef ) {
    cDatRef.mutable_bz2()->set_blocksize100k( _blockSize100k );
    cDatRef.mutable_bz2()->set_workfactor(    _workFactor );
}

size_t
BZip2Compression::_V_compressed_dest_buffer_len( const uint8_t *, size_t n ) const {
    // See http://www.bzip.org/1.0.3/html/util-fns.html :
    // "...To guarantee that the compressed data will fit in its buffer, 
    // allocate an output buffer of size 1% larger than the uncompressed
    // data, plus six hundred extra bytes..."
    return n + double(n)*0.01 + 600;
}

//
// BZip2Decompression
///////////////////

BZip2Decompression::BZip2Decompression( const goo::dict::Dictionary & dct ) :
            iDecompressor( events::CompressedData_CompressionAlgorithm_BZIP2 ),
            _small( dct["small"].as<bool>() ),
            _verbosity( dct["verbosity"].as<int>() ) {}

size_t
BZip2Decompression::_V_decompress_series(   const uint8_t * input, size_t inLen,
                                            uint8_t * output, size_t outMaxLen ) const {
    unsigned int destLen = outMaxLen;
    int rc = BZ2_bzBuffToBuffDecompress(
        /* char*         dest ......... */ (char*) output,
        /* unsigned int* destLen ...... */ &destLen,
        /* char*         source ....... */ (char*) const_cast<uint8_t *>(input),
        /* unsigned int  sourceLen .... */ inLen,
        /* int           small ........ */ (_small ? 1 : 0),
        /* int           verbosity .... */ _verbosity
        );
    # if 0
    if( BZ_OK != rc ) {
BZ_CONFIG_ERROR
  if the library has been mis-compiled
BZ_PARAM_ERROR
  if dest is NULL or destLen is NULL
  or small != 0 && small != 1
  or verbosity < 0 or verbosity > 4
BZ_MEM_ERROR
  if insufficient memory is available 
BZ_OUTBUFF_FULL
  if the size of the compressed data exceeds *destLen
BZ_DATA_ERROR
  if a data integrity error was detected in the compressed data
BZ_DATA_ERROR_MAGIC
  if the compressed data doesn't begin with the right magic bytes
BZ_UNEXPECTED_EOF
  if the compressed data ends unexpectedly
BZ_OK
  otherwise
    }
    # endif
    return destLen;
}

}  // namespace compression
}  // namespace sV

using sV::compression::BZip2Compression;
StromaV_COMPRESSOR_DEFINE_MCONF( BZip2Compression, "bzip2" ) {
    goo::dict::Dictionary bzip2CmprssnDct( "bzip2",
        "BZip2 compressor algorithm with incapsulated reentrant stream "
        "instance and adjustible compression parameters." );
    bzip2CmprssnDct.insertion_proxy()
        .p<int>( "blockSize100k",
                "specifies the block size to be used for compression. "
                "It should be a value between 1 and 9 inclusive, and the "
                "actual block size used is 100000 x this figure. 9 gives "
                "the best compression but takes most memory.",
            5 )
        .p<int>( "verbosity",
                "should be set to a number between 0 and 4 inclusive. 0 is "
                "silent, and greater numbers give increasingly verbose "
                "monitoring/debugging output. If the library has been "
                "compiled with -DBZ_NO_STDIO, no such output will appear for "
                "any verbosity setting",
            0 )
        .p<int>( "workFactor",
                "controls how the compression phase behaves when presented "
                "with worst case, highly repetitive, input data. If "
                "compression runs into difficulties caused by repetitive "
                "data, the library switches from the standard sorting "
                "algorithm to a fallback algorithm. The fallback is slower "
                "than the standard algorithm by perhaps a factor of three, "
                "but always behaves reasonably, no matter how bad the input"
                "Allowable values range from 0 to 250 inclusive.",
            30 )
        ;
    goo::dict::DictionaryInjectionMap injM;
        injM( "blockSize100k",  "compressors.bzip2.blockSize100k"   )
            ( "verbosity",      "compressors.bzip2.verbosity"       )
            ( "workFactor",     "compressors.bzip2.workFactor"      )
            ;
    return std::make_pair( bzip2CmprssnDct, injM );
}

using sV::compression::BZip2Decompression;
StromaV_DECOMPRESSOR_DEFINE_MCONF( BZip2Decompression, "bzip2" ) {
    goo::dict::Dictionary bzip2DcmprssnDct( "bzip2",
        "Few BZip2 decompression parameters." );
    bzip2DcmprssnDct.insertion_proxy()
        .flag( "small",
                "If set, the library will use an alternative decompression "
                "algorithm which uses less memory but at the cost of "
                "decompressing more slowly (roughly speaking, half the "
                "speed, but the maximum memory requirement drops to around "
                "2300k)." )
        .p<int>( "verbosity",
                "should be set to a number between 0 and 4 inclusive. 0 is "
                "silent, and greater numbers give increasingly verbose "
                "monitoring/debugging output. If the library has been "
                "compiled with -DBZ_NO_STDIO, no such output will appear for "
                "any verbosity setting",
            0 )
        ;
    goo::dict::DictionaryInjectionMap injM;
        injM( "small",          "compressors.bzip2.memopt-decompression"    )
            ( "verbosity",      "compressors.bzip2.verbosity"               )
            ;
    return std::make_pair( bzip2DcmprssnDct, injM );
}

# endif


