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

# ifndef H_STROMA_V_BUCKET_STREAM_DISPATCHER_H
# define H_STROMA_V_BUCKET_STREAM_DISPATCHER_H

# include "sV_config.h"

# if defined(RPC_PROTOCOLS) && defined(ANALYSIS_ROUTINES)

# if 0

# include "app/analysis.hpp"
# include "uevent.hpp"
# include "buckets/compressedDispatcher.hpp"

# include <goo_dict/dict.hpp>

namespace sV {
namespace dprocessors {

/**@class EventsDispatcher
 * @brief Implements interim class for file storaging and network dispatching.
 *
 * This class uses \ref iBundlingDispatcher interface to perform high-level
 * version of write-back caching.
 * */
class EventsDispatcher : public AnalysisPipeline::iEventProcessor,
                         public sV::AbstractApplication::ASCII_Entry,
                         public buckets::CompressedDispatcher {
public:
    typedef AnalysisPipeline::iEventProcessor Parent;
    typedef sV::events::Event Event;
private:
    std::ostream * _stream;
    buckets::GenericCollector * _genericCollectorPtr;
    uint8_t _prevHash[SHA256_DIGEST_LENGTH];
    uint32_t _nBucketsDropped;

    EventsDispatcher( const std::string & pn,
                      std::ostream * streamPtr );
protected:
    virtual ProcRes _V_process_event( Event & ) override;
    virtual void _V_finalize() const override;
    void _update_stat();

    void _set_generic_collector( buckets::GenericCollector * );
    buckets::GenericCollector & _generic_collector();
public:
    EventsDispatcher( const std::string & pn,
                      const goo::dict::Dictionary &,
                      std::ostream * streamPtr );
    virtual ~EventsDispatcher();
};  // class EventsDispatcher

}  // namespace ::sV::dprocessors
}  // namespace sV

# endif

# endif  // defined(RPC_PROTOCOLS) && defined(ANALYSIS_ROUTINES)
# endif  // H_STROMA_V_BUCKET_STORAGE_PROCESSOR_H


