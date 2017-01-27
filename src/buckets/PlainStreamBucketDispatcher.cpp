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
# include "buckets/PlainStreamBucketDispatcher.hpp"

# ifdef RPC_PROTOCOLS

# include <iostream>

namespace sV {

PlainStreamBucketDispatcher::PlainStreamBucketDispatcher(
        std::ostream & streamRef,
        size_t nMaxKB = 16,
        size_t nMaxEvents = 0 ) :
    iBucketDispatcher( nMaxKB,
                       nMaxEvents ),
    _streamRef(streamRef) {
}

PlainStreamBucketDispatcher::~PlainStreamBucketDispatcher() {
    drop_bucket();
}

size_t PlainStreamBucketDispatcher::_V_drop_bucket() {

    size_t bucketSize = n_Bytes()*1024;
    if ( _streamRef.good() ) {
        if (!_currentBucket.SerializeToOstream(&_streamRef)) {
            std::cerr << "Failed to serialize into stream." << std::endl;
            return EXIT_FAILURE;
        }
        else {
        }
    }
    else {
        std::cerr << "Stream for serialized output isn't good." << std::endl;
        return EXIT_FAILURE;
    }
    clear_bucket();
    return bucketSize;;
}

}  // namespace sV
# endif  // RPC_PROTOCOLS

