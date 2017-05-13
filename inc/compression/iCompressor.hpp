/*
 * Copyright (c) 2016 Renat R. Dusaev <crank@qcrypt.org>
 * Author: Bogdan Vasilishin <togetherwithra@gmail.com>
 * Author: Renat R. Dusaev <crank@qcrypt.org>
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
# ifndef H_STROMA_V_ICOMPRESSOR_H
# define H_STROMA_V_ICOMPRESSOR_H

# include "sV_config.h"

# ifdef RPC_PROTOCOLS

# include "app/abstract.hpp"

# include <cstdlib>
# include <stdint.h>

# include "event.pb.h"

namespace sV {

/**@brief A compression algorithm interface for compression routine.
 * @class iCompressor
 *
 * Various compression algorithms are used by sV for in-memory compression of
 * data chunks. This class provides a generic interface that has to be
 * implemented by specific algorithms.
 *
 * The common trait of the most of existing algorithms performing in-memory
 * compression is that they use input buffer of known length to predict maximal
 * length of output buffer. This trait has to be expressed with
 * _V_compressed_dest_buffer_len() interfacing method.
 *
 * This interface class is complementary to iDecompressor.
 * */
class iCompressor {
public:
    typedef events::CompressedData_CompressionAlgorithm CompressionAlgo;
private:
    /// String describing current compression algorithm.
    const CompressionAlgo _compressionAlgo;
protected:
    /// (IM) Has to perform the compression using input bytes of length
    /// inLen into the output buffer of maximal length outMaxLen. Has to return
    /// number of bytes in output buffer that is occupied by compressed data.
    virtual size_t _V_compress_series( const uint8_t * input, size_t inLen,
                                       uint8_t * output, size_t outMaxLen ) = 0;

    /// (IM) Has to return desired length of output buffer for compressed
    /// series basing on input buffer length.
    virtual size_t _V_compressed_dest_buffer_len( const uint8_t *, size_t ) const = 0;

    /// (IM, opt) Has to set the compression parameters union for compressed
    /// message field information. For other than sV's system algorithm, use
    /// the `custom` field of the compression parameters field.
    virtual void  _V_set_compression_info( events::CompressedData & ) = 0;
public:

    /// Ctr. Receives the name of particular compression algorithm.
    iCompressor( CompressionAlgo algo=events::CompressedData_CompressionAlgorithm_other ) :
                    _compressionAlgo( algo ) {}
    virtual ~iCompressor() {}

    /// Returns string describing current compression algorithm.
    CompressionAlgo algorithm() const
                { return _compressionAlgo; }

    /// Returns desired length for output buffer.
    size_t compressed_dest_buffer_len( const uint8_t * inbf, size_t inLen ) const
                { return _V_compressed_dest_buffer_len( inbf, inLen ); }

    /// Performs compression of input series into output buffer.
    size_t compress_series( const uint8_t * input, size_t inLen,
                            uint8_t * output, size_t outMaxLen ) {
        return _V_compress_series( input, inLen, output, outMaxLen ); }

    void  set_compression_info( events::CompressedData & cdatRef ) {
        _V_set_compression_info( cdatRef );
    }
}; // class iCompressor

} // namespace sV


# define StromaV_COMPRESSOR_DEFINE( cxxClassName,               \
                                    name )                      \
StromaV_DEFINE_STD_CONSTRUCTABLE( cxxClassName, name, sV::iCompressor )


# define StromaV_COMPRESSOR_DEFINE_MCONF( cxxClassName,             \
                                    name )                          \
StromaV_DEFINE_STD_CONSTRUCTABLE_MCONF( cxxClassName, name, sV::iCompressor )

# endif  // RPC_PROTOCOLS
# endif  // H_STROMA_V_ICOMPRESSOR_H

