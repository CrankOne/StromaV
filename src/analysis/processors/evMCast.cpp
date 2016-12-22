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

# include "config.h"

# if defined(RPC_PROTOCOLS) && defined(ANALYSIS_ROUTINES)

# include "analysis/processors/evMCast.hpp"
# include <boost/bind.hpp>

namespace sV {
namespace dprocessors {

namespace aux {

// EventPipelineStorage implementation
/////////////////////////////////////

EventPipelineStorage::EventPipelineStorage( const std::string & pn,
                                            size_t queueLength ) :
            AnalysisPipeline::iEventProcessor( pn ),
            _nProcessed(0), _queue(queueLength) {
}

EventPipelineStorage::~EventPipelineStorage() {
}

bool
EventPipelineStorage::_V_process_event( Event * eventPtr ) {
    return _push_event_to_queue(*eventPtr);
}

bool
EventPipelineStorage::_push_event_to_queue( const Event & event ) {
    std::lock_guard<std::mutex> lock( _queueMutex );
    _queue.push_front();  // now back points to newly-created
    _queue.front().CopyFrom( event );
    ++_nProcessed;
    return true;
}
}  // namespace aux


// EventMulticaster
//////////////////

EventMulticaster::EventMulticaster( const std::string & pn,
                                    const boost::asio::ip::address & multicastAddress,
                                    size_t queueLength,
                                    int portNo,
                                    boost::asio::io_service * ioServicePtr,
                                    size_t sendingBufferSize ) :
            net::iMulticastEventSender(
                                *(ioServicePtr ? ioServicePtr : new boost::asio::io_service()),
                                multicastAddress, portNo, sendingBufferSize ),
            aux::EventPipelineStorage( pn, queueLength ),
            _ioServicePtr( &(net::iMulticastEventSender::socket().get_io_service()) ),
            _ownIOService(!ioServicePtr) {
}

EventMulticaster::~EventMulticaster() {
    if( _ioServicePtr && _ownIOService ) {
        delete _ioServicePtr;
    }
}

bool
EventMulticaster::_V_do_continue_transmission() const {
    return !events_queue().empty();
}

void
EventMulticaster::_V_send_next_message() {
    // Note: this method can be invoked either by event-treatment method,
    // either by sending thread handling end-of transmission for non-empty
    // queue.
    std::lock_guard<std::mutex> lock(_queueMutex);
    // Sometimes evaluation meets the empty queue here because
    // of sending thread have sent last event in queue and treatment
    // thread comes here meeting empty queue.
    if( !is_empty() ) {
        _reentrantMessageKeeper.Clear();
        _reentrantMessageKeeper.mutable_event()->CopyFrom( events_queue().front() );
        send_message( _reentrantMessageKeeper );
        events_queue().pop_front();
    }
}

bool
EventMulticaster::_V_process_event( Event * eventPtr ) {
    bool insertionResult = aux::EventPipelineStorage::_V_process_event( eventPtr );
    sending_mutex().lock();
    if( !net::iMulticastEventSender::is_operating() ) {
        _V_send_next_message();
    }
    sending_mutex().unlock();
    return insertionResult;
}

void
EventMulticaster::_V_print_brief_summary( std::ostream & os ) const {
    os << ESC_CLRGREEN "Event multicasting processor" ESC_CLRCLEAR ":" << std::endl
       << "  number of events processed . : " << n_processed() << std::endl;
    // TODO: ... other stuff
}

// Register processor:
StromaV_DEFINE_CONFIG_ARGUMENTS {
    po::options_description multicastP( "Multicasting (network multicast)" );
    { multicastP.add_options()
        ("multicast.address",
            po::value<std::string>()->default_value("239.255.0.1"),
            "Multicast address to use." )
        ("multicast.port",
            po::value<int>()->default_value(30001),
            "Multicast port number.")
        ("multicast.storage-capacity",
            po::value<size_t>()->default_value(500),
            "Event to be stored. Defines the capacitance of last read events.")
        ;
    }
    return multicastP;
}
StromaV_DEFINE_DATA_PROCESSOR( EventMulticaster ) {
    auto p = new EventMulticaster(
            "multicast",
            boost::asio::ip::address::from_string(
                goo::app<sV::AbstractApplication>().cfg_option<std::string>("multicast.address")
            ),
            goo::app<sV::AbstractApplication>().cfg_option<size_t>("multicast.storage-capacity"),
            goo::app<sV::AbstractApplication>().cfg_option<int>("multicast.port"),
            goo::app<sV::AbstractApplication>().boost_io_service_ptr()
        );
    //io_service.run();
    return p;
} StromaV_REGISTER_DATA_PROCESSOR(
    EventMulticaster,
    "multicast",
    "An asynchroneous event-multicasting pipeline." )

}  // namespace sV
}  // namespace dprocessors

# endif  // defined(RPC_PROTOCOLS) && defined(ANALYSIS_ROUTINES)

