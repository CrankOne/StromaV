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

# include "compression/trivial.hpp"

# ifdef RPC_PROTOCOLS

namespace sV {
namespace compression {

//
// Compressor

size_t
TrivialCompression::_V_compress_series(
            const uint8_t * input, size_t inLen,
            uint8_t * output, size_t outMaxLen ) const {
    assert( outMaxLen >= inLen );
    (void)(outMaxLen);
    memcpy( output, input, inLen );
    return inLen;
}


//
// Decompressor

size_t
TrivialCompression::_V_decompress_series(
            const uint8_t * input, size_t inLen,
            uint8_t * output, size_t outMaxLen ) const {
    assert( outMaxLen >= inLen );
    (void)(outMaxLen);
    memcpy( output, input, inLen );
    return inLen;
}

}  // namespace compression
}  // namespace sV

namespace sV_internal_1 {
using sV::compression::TrivialCompression;
StromaV_COMPRESSOR_DEFINE( TrivialCompression, "trivial" ) {
    return goo::dict::Dictionary( NULL, "" );
}
}

namespace sV_internal_2 {
using sV::compression::TrivialCompression;
StromaV_DECOMPRESSOR_DEFINE( TrivialCompression, "trivial" ) {
    return goo::dict::Dictionary( NULL, "" ); 
}
}

# endif  // RPC_PROTOCOLS

