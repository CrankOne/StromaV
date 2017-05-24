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
# include "buckets/plainStreamDispatcher.hpp"

# ifdef RPC_PROTOCOLS

# include "goo_exception.hpp"

namespace sV {
namespace buckets {

PlainStreamDispatcher::PlainStreamDispatcher(
        std::ostream & streamRef ) :
    _streamRef(streamRef) {
}

PlainStreamDispatcher::~PlainStreamDispatcher() {
    drop_bucket();
}

size_t
PlainStreamDispatcher::_V_drop_bucket() {
    size_t bucketSize = bucket().ByteSize();
    // Write size of the bucket to be dropped into output file
    _streamRef.write((char*)(&bucketSize), sizeof(uint32_t));
    // Then write the bucket
    if( !bucket().SerializeToOstream(&_streamRef) ) {
        emraise( ioError, "Failed to serialize into stream." );
    }
    return bucketSize;
}

}  // namespace ::sV::buckets
}  // namespace sV

# endif  // RPC_PROTOCOLS

