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

# ifndef H_STROMA_V_SVBP_READER_BUCKET_READERS_AUX_H
# define H_STROMA_V_SVBP_READER_BUCKET_READERS_AUX_H

# include "sV_config.h"

# include <goo_exception.hpp>

# ifdef RPC_PROTOCOLS

# include "uevent.hpp"

namespace sV {
namespace aux {
struct SHA256BucketHash;
}  // namespace ::sV::aux
}  // namespace sV

namespace std {
std::ostream & operator<<(std::ostream& stream, const ::sV::aux::SHA256BucketHash &);
}

namespace sV {
namespace aux {

struct SHA256BucketHash {
    uint32_t hash[8];
    friend std::ostream & ::std::operator<<(std::ostream& stream, const SHA256BucketHash &);

    SHA256BucketHash( sV::events::CommonBucketDescriptor & msg ) {
        if( sizeof(hash) != msg.sha256hash().size() ) {
            emraise( badValue, "Expected length of SHA256 hash is 32 bytes "
                "while incoming protobuf message carries %d.",
                msg.sha256hash().size() );
        }
        memcpy( hash, msg.sha256hash().c_str(), sizeof(hash) );
    }
};

}  // namespace ::sV::aux
}  // namespace sV

//
// Template specialization for SHA256 hash used as map key
namespace std {
template<>
struct hash<::sV::aux::SHA256BucketHash> {
    /// Implements murmur3 hash function around SHA256 with hardcoded seed.
    size_t operator()( const ::sV::aux::SHA256BucketHash & sha256 ) const;
};
template<>
struct equal_to<::sV::aux::SHA256BucketHash> {
    /// Performs direct comparison.
    size_t operator()( const ::sV::aux::SHA256BucketHash & sha256l,
                       const ::sV::aux::SHA256BucketHash & sha256r ) const;
};
}  // namespace std

# endif  // RPC_PROTOCOLS

# endif  // H_STROMA_V_SVBP_READER_BUCKET_READERS_AUX_H

