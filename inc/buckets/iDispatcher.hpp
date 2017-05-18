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
 * One can choose which bucket this processor has to consider as "full": either
 * by entries number or by uncompressed size. If both criteria will be non-null,
 * the first matching criterion will trigger bucket to be dropped. If both
 * criteria are set to 0, the handler will never drop the buket until handler's
 * destructor will be invoked.
 * */
class iDispatcher {
public:
    typedef events::Bucket Bucket;
private:
    /// Upper data size limit for accumulation (in bytes)
    size_t _nBytesMax;
    /// Upper size limit for accumulation (in number of events)
    size_t _nMaxEvents;
protected:
    /// (IM) Shall define how to perform "drop"
    virtual size_t _V_drop_bucket() = 0;

    /// Buffering bucket instance
    events::Bucket * _rawBucketPtr;
public:
    /// Ctr getting drop criteria. When doPackMetaInfo is set and metainfo
    /// collector is provided, the dropping method will automatically invoke
    /// pack_metainfo() method of internal metainfo collector prior to
    /// _V_drop_bucket().
    iDispatcher( size_t nMaxKB, size_t nMaxEvents );

    virtual ~iDispatcher();

    /// Returns current buffering bucket instance
    virtual events::Bucket & bucket() { return *_rawBucketPtr; }

    /// Returns current buffering bucket instance (const getter)
    virtual const events::Bucket & bucket() const { return *_rawBucketPtr; }

    /// Performs consideration of event. Upon one of the above criteria is
    /// reached, invokes drop_bucket().
    virtual void push_event(const events::Event & );

    /// Returns size limit (in kBs)
    size_t n_max_KB() const { return _nBytesMax/1024; }

    /// Returns size limit (in bytes)
    size_t n_max_bytes() const { return _nBytesMax; }

    /// Returns size limit (in events)
    size_t n_max_events() const { return _nMaxEvents; }

    /// Returns current size (in bytes)
    size_t n_bytes() const {
        return (size_t)(bucket().ByteSize());
    };

    /// Returns current size (in events)
    size_t n_events() const {
        return (size_t)(bucket().events_size());
    };

    /// Returns true when one of the size limits is reached
    virtual bool is_bucket_full();

    /// Returns true if no events had been considered after last drop (or
    /// instance creation)
    virtual bool is_bucket_empty();

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

