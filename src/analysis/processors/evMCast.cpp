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

# include "sV_config.h"

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

EventMulticaster::EventMulticaster( const goo::dict::Dictionary & dct ) :
            EventMulticaster(
                "multicast",
                boost::asio::ip::address::from_string( dct["address"].as<std::string>() ),
                dct["storageCapacity"].as<size_t>(),
                dct["port"].as<int>(),
                goo::app<sV::AbstractApplication>().boost_io_service_ptr()
            ) {}

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

StromaV_ANALYSIS_PROCESSOR_DEFINE_MCONF( EventMulticaster, "multicast" ) {
    goo::dict::Dictionary mCastPDict( "mutlicast",
        "This processor performs asynchroneous network multicasting of events "
        "received from analysis pipeline. Parameters are listed at "
        "\"multicasting\" subsection of commong config. Note, that if buffering "
        "queue is overflown, the older events will not be sent." );
    mCastPDict.insertion_proxy()
        .p<std::string>("address",
            "Multicast address to use.",
            "239.255.0.1" )
        .p<int>( "port",
            "Multicast port number.",
            30001 )
        .p<size_t>("capacity",
            "Event to be stored. Defines the capacitance of last read events.",
            500)
        ;
        goo::dict::DictionaryInjectionMap injM;
        injM( "address",        "analysis.processors.multicast.address" )
            ( "port",           "analysis.processors.multicast.port" )
            ( "capacity",       "analysis.processors.multicast.capacity" )
            ;
    return std::make_pair( mCastPDict, injM );
}

}  // namespace sV
}  // namespace dprocessors

# endif  // defined(RPC_PROTOCOLS) && defined(ANALYSIS_ROUTINES)

