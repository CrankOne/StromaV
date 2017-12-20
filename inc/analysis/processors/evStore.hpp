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

# ifndef H_STROMA_V_BUCKET_STORAGE_PROCESSOR_H
# define H_STROMA_V_BUCKET_STORAGE_PROCESSOR_H

# include "sV_config.h"

# if defined(RPC_PROTOCOLS) && defined(ANALYSIS_ROUTINES)

# if 0

# include "evStreamDispatch.hpp"

# include <fstream>

namespace sV {
namespace dprocessors {

/**@class EventsStore
 * @brief Pipeline handler performing saving of the events to file.
 *
 * This class uses \ref iBundlingDispatcher interface to perform high-level
 * version of write-back caching.
 * */
class EventsStore : public EventsDispatcher {
public:
    typedef AnalysisPipeline::iEventProcessor Parent;
    typedef sV::events::Event Event;
private:
    std::fstream _file;
public:
    EventsStore( const goo::dict::Dictionary & );
    virtual ~EventsStore();
};  // class EventsStore

}  // namespace ::sV::dprocessors
}  // namespace sV

# endif

# endif  // defined(RPC_PROTOCOLS) && defined(ANALYSIS_ROUTINES)
# endif  // H_STROMA_V_BUCKET_STORAGE_PROCESSOR_H

