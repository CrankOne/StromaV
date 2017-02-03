/*
 * Copyright (c) 2016 Renat R. Dusaev <crank@qcrypt.org>
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

# ifndef H_STROMA_V_BUCKET_STORAGE_PROCESSOR_H
# define H_STROMA_V_BUCKET_STORAGE_PROCESSOR_H

# include "config.h"

# if defined(RPC_PROTOCOLS) && defined(ANALYSIS_ROUTINES)

# include "app/analysis.hpp"
# include "uevent.hpp"
# include "buckets/iBucketDispatcher.hpp"

namespace sV {
namespace dprocessors {

/**@class Bucketer
 * @brief Pipeline handler performing packaging of the events.
 *
 * The idea of "bucket" is quite similar to ROOT's "basket" and in a nutshell
 * is just a buffer containing discrete number of events. These "buckets" can
 * be further compressed and forwarded to storage/stdout/network as a solid
 * data chunks.
 *
 * We need such a thing because it leads for much more efficient compression
 * and transmission comparing to ordinary per-event basis.
 *
 * This class is designed as a pipeline handler. It will acquire event-by-event
 * until its bucket will not be "full" and then will perform compression
 * (and forwarding) "full" bucket (we call it "drop").
 *
 * One can choose which bucket this processor has to consider as "full": either
 * by event number or by uncompressed size. If both criteria will be non-bull,
 * the first matching criterion will trigger bucket drop. If both criteria are
 * set to 0, the handler will never drop the buket until handler's destructor
 * will be invoked.
 * */
class Bucketer : public AnalysisPipeline::iEventProcessor {
public:
    typedef AnalysisPipeline::iEventProcessor Parent;
    typedef AnalysisApplication::Event Event;
    /*
    struct CompressionParameters {
        // ...
    };
    */
private:
    //sV::events::Bucket _reentrantBucket;
    //
protected:
    sV::iBucketDispatcher * _bucketDispatcher;
    virtual bool _V_process_event( Event * ) override;
public:
    Bucketer( const std::string & pn,
              sV::iBucketDispatcher * bucketDispatcher ) :
              AnalysisPipeline::iEventProcessor( pn ) {}

    /// Returns true if "full" criterion(-ia) triggered.
    //  bool is_bucket_full() const;

    /// Causes current bucket to be compressed (and optionally forwarded) and,
    /// further, cleared.
    //  void drop_bucket();
};  // class Bucketer

}  // namespace dprocessors
}  // namespace sV

# endif  // defined(RPC_PROTOCOLS) && defined(ANALYSIS_ROUTINES)

# endif  // H_STROMA_V_BUCKET_STORAGE_PROCESSOR_H

