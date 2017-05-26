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

# include "analysis/processors/evSend.hpp"

# if defined(RPC_PROTOCOLS) && defined(ANALYSIS_ROUTINES)

# include "buckets/compressedDispatcher.hpp"

namespace sV {
namespace dprocessors {

EventsSend::EventsSend( const goo::dict::Dictionary & dct ) :
                    EventsDispatcher( "eventsSend", dct, &_sendingBufferStream ),
                    _sendingBufferStream( _sendingBuffer ),
                    _connection(
                            dct["portNo"].as<int>(),
                            dct["destination"].as<std::string>() ) {}

EventsSend::~EventsSend() {}

size_t
EventsSend::_V_drop_bucket() {
    size_t nBytes = EventsDispatcher::_V_drop_bucket(), nSent = 0;
    _connection.connect();
    do {
        nSent += _connection.send(
                    _sendingBuffer.c_str() + nSent,
                    _sendingBuffer.size() );
    } while( nSent < nBytes );
    _connection.close();
    return nBytes;
}

StromaV_ANALYSIS_PROCESSOR_DEFINE_MCONF( EventsSend, "eventsSend" ) {
    goo::dict::Dictionary evSendPDict( "eventsSend",
        "Will perform accumulation events into a bucket, similar to "
        "\"eventsStore\" processor, but instead of saving this bucket into a "
        "file sends bucket by TCP over a network (or using the loopback "
        "interface). The sending is synchroneous and may cost time if "
        "network connection has low bandwith." );
    evSendPDict.insertion_proxy()
        .p<size_t>("maxBucketSize_kB",
                "Maximum bucket capacity (in kilobytes). 0 disables"
                "criterion.",
            500)
        .p<size_t>("maxBucketSize_events",
                "Maximum bucket capacity (number of enents). 0 disables "
                "criterion.",
            0)
        .p<std::string>("compression",
                "Algorithm name for compressing buckets.",
            "bz2")
        .list<std::string>( "suppInfo",
                    "Supplementary info collectors.",
                    { "Generic" } )
        .p<std::string>( "destination",
                "Set it to host name to send by TCP to the certain host. "
                "Specify the empty string or \"loopback\" to send it on "
                "loopback interface.",
            "loopback" )
        .p<int>( "portNo",
                "Port number on destination host (or loopback interface) "
                "to communicate with.",
            23011 )
        ;
    goo::dict::DictionaryInjectionMap injM;
        injM( "maxBucketSize_kB",       "analysis.processors.send.maxBucketSize_kB" )
            ( "maxBucketSize_events",   "analysis.processors.send.maxBucketSize_events" )
            ( "compression",            "analysis.processors.send.compression" )
            ( "suppInfo",               "analysis.processors.send.suppInfo" )
            ( "destination",            "analysis.processors.send.destination" )
            ( "portNo",                 "analysis.processors.send.portNo" )
            ;
    return std::make_pair( evSendPDict, injM );
}

}  // namespace ::sV::dprocessors
}  // namespace sV

# endif  // defined(RPC_PROTOCOLS) && defined(ANALYSIS_ROUTINES)

