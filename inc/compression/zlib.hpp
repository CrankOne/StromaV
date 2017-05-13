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

# ifndef H_STROMA_V_ZLIB_COMPRESSOR_H
# define H_STROMA_V_ZLIB_COMPRESSOR_H

# include "sV_config.h"

# ifdef ZLIB_FOUND 

# include "compression/iCompressor.hpp"

# include "zlib.h"

namespace sV {
namespace compression {

class ZLibCompression : public iCompressor {
private:
    z_stream _zstrm;
    int8_t _zLvl;
protected:
    virtual size_t _V_compress_series( const uint8_t *, size_t,
                                       uint8_t *, size_t ) override;

    virtual size_t _V_compressed_dest_buffer_len( const uint8_t *, size_t n ) const override;

    virtual void  _V_set_compression_info( events::CompressedData & ) override;
public:
    ZLibCompression( const goo::dict::Dictionary & );

    /// A helper function, common to merely all ZLib routines.
    static void init_zlib_stream( z_stream & strm );
};

}  // namespace compression
}  // namespace sV

# endif  // ZLIB_FOUND

# endif  // H_STROMA_V_ZLIB_COMPRESSOR_H

