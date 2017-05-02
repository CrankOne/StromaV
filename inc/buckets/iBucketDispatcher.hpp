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
# include <boost/program_options.hpp>

namespace sV {

namespace po = boost::program_options;

/**@class iBucketDispatcher
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
class iBucketDispatcher {
private:
    size_t _nMaxKB;
    size_t _nMaxEvents;
protected:
    virtual size_t _V_drop_bucket() = 0;
    events::Bucket _currentBucket;  // TODO: use arena?
public:
    typedef sV::events::Bucket Bucket;

    iBucketDispatcher( size_t nMaxKB, size_t nMaxEvents );
    virtual ~iBucketDispatcher();

    events::Bucket & bucket() { return _currentBucket; }

    const events::Bucket & bucket() const { return _currentBucket; }

    virtual void push_event(const events::Event & reentrantEvent);

    size_t n_max_KB() const {return _nMaxKB;};
    size_t n_max_events() const {return _nMaxEvents;};

    size_t n_bytes() const {
        return (size_t)(bucket().ByteSize());
    };
    size_t n_events() const {
        return (size_t)(bucket().events_size());
    };

    virtual bool is_bucket_full();
    virtual bool is_bucket_empty();
    size_t drop_bucket();
    virtual void clear_bucket();
};  // class iBucketDispatcher

}  //  namespace sV

# endif  //  RPC_PROTOCOLS
# endif  //  H_STROMA_V_IBUCKET_DISPATCHER_H

