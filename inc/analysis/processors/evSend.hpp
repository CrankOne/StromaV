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

# ifndef H_STROMA_V_EVENT_SEND_DATA_PROCESSOR_H
# define H_STROMA_V_EVENT_SEND_DATA_PROCESSOR_H

# include "sV_config.h"

# if defined(RPC_PROTOCOLS) && defined(ANALYSIS_ROUTINES)

# if 0

# include "evStreamDispatch.hpp"
# include "net/client.hpp"

namespace sV {
namespace dprocessors {

class EventsSend : public EventsDispatcher {
public:
    typedef AnalysisPipeline::iEventProcessor Parent;
    typedef sV::events::Event Event;
private:
    std::string _sendingBuffer;
    std::stringstream _sendingBufferStream;
    net::ClientConnection _connection;
protected:
    /// Performs sending of the bucket.
    virtual size_t _V_drop_bucket() override;
public:
    EventsSend( const goo::dict::Dictionary & );
    virtual ~EventsSend();
};  // class EventsSend

}  // namespace ::sV::dprocessors
}  // namespace sV

# endif

# endif  // defined(RPC_PROTOCOLS) && defined(ANALYSIS_ROUTINES)
# endif  // H_STROMA_V_EVENT_SEND_DATA_PROCESSOR_H

