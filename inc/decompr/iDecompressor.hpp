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
# ifndef H_STROMA_V_IDECOMPRESSOR_H

# include "../config.h"
# ifdef RPC_PROTOCOLS
# include <cstdlib>
# include <stdint.h>

# include "event.pb.h"

namespace sV {

namespace events {
    typedef DeflatedBucketMetaInfo_CompressionMethod CompressionMethod;
}

class iDecompressor {
    public:
        events::CompressionMethod compr_method() const {return _comprMethod;}
        size_t decompress_series( uint8_t * uncomprBuf, size_t lenUncomprBuf,
                                  uint8_t * comprBuf, size_t lenComprBuf )
                                  const;
    protected:
        virtual size_t _V_decompress_series( uint8_t *, size_t,
                                             uint8_t *, size_t) const = 0;
        iDecompressor( events::CompressionMethod comprMethod);
        virtual ~iDecompressor();
    private:
        const events::CompressionMethod _comprMethod;
};  // class iDecompressor

}        // namespace sV
# endif  // RPC_PROTOCOLS
# endif  // H_STROMA_V_IDECOMPRESSOR_H

