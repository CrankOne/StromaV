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

# ifndef H_STROMA_V_MULTICASTING_DATA_PROCESSOR_H
# define H_STROMA_V_MULTICASTING_DATA_PROCESSOR_H

# include "sV_config.h"

# if defined(RPC_PROTOCOLS) && defined(ANALYSIS_ROUTINES)

# include <boost/asio.hpp>
# include <mutex>
# include <boost/circular_buffer.hpp>
# include <boost/thread.hpp>

# include <goo_dict/dict.hpp>

# include "app/analysis.hpp"
# include "uevent.hpp"
# include "mCastSender.hpp"

namespace sV {
namespace dprocessors {

namespace aux {
/**@brief An event-server pipeline class.
 *
 * Class implementing a pipeline approach. Represents an open queue
 * (limeted FIFO stack). It recieves a number events and keeps them
 * stacked until FIFO stack limit will be reached. Then newly added
 * events causes erasing of stack bottom.
 *
 * The EventPipelineStorage instance by itself does not provide any
 * event treatment. It only manages a circular queue of serialized
 * gprotobuf's MulticastMessage.
 * */
class EventPipelineStorage : public AnalysisPipeline::iEventProcessor {
public:
    typedef ::sV::events::Event Event;
private:
    size_t _nProcessed;
    boost::circular_buffer<Event> _queue;
protected:
    std::mutex _queueMutex;

    virtual bool _push_event_to_queue( const Event & );
    virtual bool _V_process_event( Event * ) override;
public:
    EventPipelineStorage( const std::string & pName, size_t queueLength );
    ~EventPipelineStorage();

    const boost::circular_buffer<Event> & events_queue() const { return _queue; }
    boost::circular_buffer<Event> & events_queue() { return _queue; }

    size_t n_processed() const { return _nProcessed; }
    bool is_empty() const { return _queue.empty(); }
};  // class EventPipeline
}  // namespace aux


/**@class EventMulticaster
 * @brief Class implementing network multicasting with deferred buffering.
 *
 * This class implements storaging and sending interfaces of event
 * multicasting API.
 */
class EventMulticaster : public net::iMulticastEventSender,
                         public aux::EventPipelineStorage {
public:
    typedef aux::EventPipelineStorage::Event Event;
    typedef ::sV::events::MulticastMessage Message;
private:
    boost::asio::io_service * _ioServicePtr;
    bool _ownIOService;
    Message _reentrantMessageKeeper;
protected:
    virtual bool _V_do_continue_transmission() const override;
    virtual void _V_send_next_message() override;
    virtual bool _V_process_event( Event * eventPtr ) override;
    virtual void _V_print_brief_summary( std::ostream & os ) const override;
public:
    EventMulticaster( const std::string & pn,
                      const boost::asio::ip::address & multicastAddress,
                      size_t queueLength,
                      int portNo=30001,
                      boost::asio::io_service * ioServicePtr=nullptr,
                      size_t sendingBufferSize=1024*1024 );
    EventMulticaster( const goo::dict::Dictionary & );
    ~EventMulticaster();

    boost::asio::io_service & ioservice() { return *_ioServicePtr; }
    const boost::asio::io_service & ioservice() const { return *_ioServicePtr; }
};  // class EventMulticaster

}  // namespace sV
}  // namespace dprocessors

# endif  // defined(RPC_PROTOCOLS) && defined(ANALYSIS_ROUTINES)
# endif  // H_STROMA_V_MULTICASTING_DATA_PROCESSOR_H

