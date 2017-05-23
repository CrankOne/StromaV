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

# ifndef H_STROMA_V_BZIP2_COMPRESSOR_H
# define H_STROMA_V_BZIP2_COMPRESSOR_H

# include "sV_config.h"

# ifdef BZIP2_FOUND

# include "compression/iCompressor.hpp"
# include "compression/iDecompressor.hpp"

# include <bzlib.h>

namespace sV {
namespace compression {

class BZip2Compression : public iCompressor {
private:
    int _blockSize100k,
        _verbosity,
        _workFactor
        ;
protected:
    virtual size_t _V_compress_series( const uint8_t *, size_t,
                                       uint8_t *, size_t ) const override;

    virtual size_t _V_compressed_dest_buffer_len( const uint8_t *, size_t n ) const override;

    virtual void  _V_set_compression_info( events::CompressedData & ) override;
public:
    BZip2Compression( const goo::dict::Dictionary & );
};  // class BZip2Compression


class BZip2Decompression : public iDecompressor {
private:
    bool _small;
    int _verbosity;
protected:
    virtual size_t _V_decompress_series( const uint8_t * input, size_t inLen,
                                         uint8_t * output, size_t outMaxLen ) const override;
public:
    BZip2Decompression( const goo::dict::Dictionary & );
};  // class BZip2Decompression

}  // namespace compression
}  // namespace sV

# endif  // BZIP2_FOUND

# endif  // H_STROMA_V_BZIP2_COMPRESSOR_H


