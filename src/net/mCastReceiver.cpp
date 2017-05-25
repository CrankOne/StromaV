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

# include "net/mCastReceiver.hpp"

# if defined(RPC_PROTOCOLS)

# include "app/app.h"

# include <goo_exception.hpp>

# include <boost/bind.hpp>
# include <boost/exception/diagnostic_information.hpp>

namespace sV {
namespace net {

void
iMulticastEventReceiver::_reallocate_reentrant_buffer( size_t newSize ) {
    assert(newSize);
    if( _dataReentrantBufferPtr ) {
        delete [] _dataReentrantBufferPtr;
    }
    _dataReentrantBufferPtr = new UByte [_dataReentrantBufferLength = newSize];
}

iMulticastEventReceiver::iMulticastEventReceiver( boost::asio::io_service & ioService,
                                const boost::asio::ip::address & listenAddress,
                                const boost::asio::ip::address & multicastAddress,
                                int portNo,
                                size_t bufferLength ) :
        _udpSocket(ioService),
        _dataReentrantBufferPtr(nullptr),
        _dataReentrantBufferLength(bufferLength) {
    _reallocate_reentrant_buffer( bufferLength );
    // Create the socket so that multiple may be bound to the same address.
    boost::asio::ip::udp::endpoint listenEndpoint(
                listenAddress, portNo);
    _udpSocket.open(listenEndpoint.protocol());
    try {
        _udpSocket.set_option(boost::asio::ip::udp::socket::reuse_address(true));
        _udpSocket.bind(listenEndpoint);

        // Join the multicast group.
        _udpSocket.set_option(
            boost::asio::ip::multicast::join_group(multicastAddress));

        _udpSocket.async_receive_from(
            boost::asio::buffer(_dataReentrantBufferPtr, _dataReentrantBufferLength), _udpSenderEndpoint,
            boost::bind(&iMulticastEventReceiver::_handle_receive,
                        this,
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred));
    } catch ( const boost::exception & ) {
        emraise( thirdParty, "While creating receiver: %s.",
            boost::current_exception_diagnostic_information().c_str() );
    }
}

iMulticastEventReceiver::~iMulticastEventReceiver() {
    if( _dataReentrantBufferPtr ) {
        delete [] _dataReentrantBufferPtr;
    }
}

void iMulticastEventReceiver::_handle_receive( const boost::system::error_code& error,
                                      size_t nBytesRecvd ) {
    assert( _dataReentrantBufferPtr && _dataReentrantBufferLength );
    bool doRenewConnection = true;
    if( !error || nBytesRecvd > _dataReentrantBufferLength ) {
        //// Try to deserialize MulticastMessage here and invoke
        //// interface functions.
        //if( !_reentrantMessageInstance.ParseFromArray(
        //            _dataReentrantBufferPtr, nBytesRecvd ) ) {
        //    sV_loge( "Got deserialization error of message %d bytes size.\n",
        //                 (int) nBytesRecvd );
        //}
        if( nBytesRecvd ) {
            sV_log3( "iMulticastEventReceiver: got a message of %zu bytes length.\n" );
            doRenewConnection = treat_incoming_message( _dataReentrantBufferPtr, nBytesRecvd );
        }
    } else {
        sV_loge( "Got connection error. Subscription interrupted.\n" );
    }
    if( doRenewConnection ) {
        // Renew listener binding.
        _udpSocket.async_receive_from(
            boost::asio::buffer(_dataReentrantBufferPtr, _dataReentrantBufferLength), _udpSenderEndpoint,
            boost::bind(&iMulticastEventReceiver::_handle_receive,
                        this,
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred));
    }
}

}  // namespace net
}  // namespace sV

# endif  // defined(RPC_PROTOCOLS)

