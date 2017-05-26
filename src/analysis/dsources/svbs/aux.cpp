/*
 * Copyright (c) 2017 Renat R. Dusaev <crank@qcrypt.org>
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

# include "sV_config.h"

# ifdef RPC_PROTOCOLS

# include "analysis/dsources/svbs/aux.hpp"

namespace sV {
namespace buckets {

# if 0
//void
//BucketReader::_V_print_brief_summary( std::ostream & ) const {}

BucketsFileReader::BucketsFileReader(
            const std::list<goo::filesystem::Path> & filenames,
            size_t maxEvents,
            bool enableProgressbar ) :
                        Parent( 0x0 ),  // TODO: md support
                        sV::AbstractApplication::ASCII_Entry( goo::aux::iApp::exists() ?
                                &goo::app<AbstractApplication>() : nullptr, 1 ),
                        _filenames( filenames.begin(), filenames.end() ),
                        _nEventsMax(0),
                        _pbParameters(nullptr) {
    if( enableProgressbar && maxEvents ) {
        _pbParameters = new PBarParameters;
        bzero( _pbParameters, sizeof(PBarParameters) );
        _pbParameters->mtrestrict = 250;
        _pbParameters->length = 80;
        _pbParameters->full = maxEvents;
    }
}
# endif

}  // namespace ::sV::buckets
}  // namespace sV

namespace std {

size_t
hash<::sV::aux::SHA256BucketHash>::operator()(
                        const ::sV::aux::SHA256BucketHash & sha256 ) const {
    const uint8_t * key = (const uint8_t *) sha256.hash;
    uint32_t h = 157;  // seed, actually
    const uint32_t* key_x4 = (const uint32_t*) key;
    size_t i = 32 >> 2;
    do {
        uint32_t k = *key_x4++;
        k *= 0xcc9e2d51;
        k = (k << 15) | (k >> 17);
        k *= 0x1b873593;
        h ^= k;
        h = (h << 13) | (h >> 19);
        h += (h << 2) + 0xe6546b64;
    } while (--i);
    key = (const uint8_t*) key_x4;

    h ^= 32;
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;
    return h;
}

size_t
equal_to<::sV::aux::SHA256BucketHash>::operator()(
                const ::sV::aux::SHA256BucketHash & sha256l,
                const ::sV::aux::SHA256BucketHash & sha256r ) const {
    return !memcmp( sha256l.hash, sha256r.hash, sizeof(sha256l.hash) );
}

std::ostream &
operator<<(std::ostream & os, const ::sV::aux::SHA256BucketHash & sha256h) {
    os << std::hex;
    for( uint8_t i = 0; i < 8; ++i ) {
        os << sha256h.hash[i];
    }
    os << std::dec;
    return os;
}

}  // namespace std

# endif  // RPC_PROTOCOLS

