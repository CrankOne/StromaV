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
# ifndef H_STROMA_V_IBUCKET_DISPATCHER_H
# define H_STROMA_V_IBUCKET_DISPATCHER_H

# include "sV_config.h"

# ifdef RPC_PROTOCOLS

# include "uevent.hpp"

# include <goo_dict/dict.hpp>

# include <unordered_map>

namespace sV {
namespace buckets {

/**@class iDispatcher
 * @brief Helper event accumulation class.
 *
 * The idea of "bucket" is quite similar to ROOT's "basket" and in a nutshell
 * is just a buffer containing discrete number of events. These "buckets" can
 * be further compressed and forwarded to storage/stdout/network as a solid
 * data chunks.
 *
 * One need such a thing because it leads for much more efficient compression
 * and transmission comparing to ordinary per-event basis.
 *
 * This class may represents an automatic buffer that will accumulate data
 * (events) until associated buffer will not be full.
 *
 * User code has to choose which bucket has to considered as "full" by
 * implementation of _V_is_full() interface method. Particular behaviour of
 * bucket being "dropped" has to be implemented by _V_drop_bucket() as well.
 * */
class iDispatcher {
public:
    typedef events::Bucket Bucket;
protected:
    /// (IM) Shall define how to perform "drop"
    virtual size_t _V_drop_bucket() = 0;

    /// (IM) Has to return whether bucket is full and thus has to be dropped.
    virtual bool _V_is_full() const = 0;

    /// Buffering bucket instance
    events::Bucket * _rawBucketPtr;
public:
    iDispatcher();

    virtual ~iDispatcher();

    /// Returns current buffering bucket instance
    virtual events::Bucket & bucket() { return *_rawBucketPtr; }

    /// Returns current buffering bucket instance (const getter)
    virtual const events::Bucket & bucket() const { return *_rawBucketPtr; }

    /// Performs consideration of event. Upon one of the above criteria is
    /// reached, invokes drop_bucket().
    virtual void push_event(const events::Event & );

    /// Returns true if no events had been considered after last drop (or
    /// instance creation)
    virtual bool is_empty() const;

    /// Returns whether bucket is full and thus has to be dropped.
    bool is_full() const { return _V_is_full(); }

    /// Performs dropping procedure (usually defined by subclasses) if bucket
    /// is not empty.
    virtual size_t drop_bucket();

    /// Clears current state of an instance
    virtual void clear_bucket();
};  // class iDispatcher

}  // namespace ::sV::buckets
}  // namespace sV

# endif  //  RPC_PROTOCOLS
# endif  //  H_STROMA_V_IBUCKET_DISPATCHER_H

