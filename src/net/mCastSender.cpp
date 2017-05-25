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

# include "net/mCastSender.hpp"

# if defined(RPC_PROTOCOLS)

# include "app/app.h"
# include <boost/bind.hpp>

namespace sV {
namespace net {

iMulticastEventSender::iMulticastEventSender( boost::asio::io_service & ioService,
                            const boost::asio::ip::address & multicastAddress,
                            int portNo,
                            size_t sendingBufferSize ) :
                    _udpEndpoint(multicastAddress, portNo),
                    _socket(ioService, _udpEndpoint.protocol()),
                    _serializedMessagePtr(nullptr),
                    _serializedMessageLength(0),
                    _isOperating(false),
                    _sendingWorkPtr( nullptr ),
                    _thread( nullptr ),
                    _ioServiceRef( ioService ) {
    _resize_sending_buffer( sendingBufferSize );
}

iMulticastEventSender::~iMulticastEventSender() {
    if( _sendingWorkPtr ) {
        # if 0
        _socket.shutdown(boost::asio::ip::tcp::socket::shutdown_send); // TODO: is it a correct way?
        // No, that's not correct. See:
        // http://stackoverflow.com/questions/10118943/getting-transport-endpoint-is-not-connected-in-udp-socket-programming-in-c
        # endif
        _reentrantStatusMessage.mutable_status()->set_senderstatus(
                ::sV::events::MulticastMessage_SenderStatusMessage_SenderStatus::
                MulticastMessage_SenderStatusMessage_SenderStatus_QUENCHING
            );
        send_message( _reentrantStatusMessage, true );
        delete _sendingWorkPtr;
    }
    if( _serializedMessagePtr ) {
        delete [] _serializedMessagePtr;
    }
}

void
iMulticastEventSender::_setup_transmission() {
    //_sendingMutex.lock();
    // Go!
    _thread = new boost::thread( boost::bind(&boost::asio::io_service::run, &_ioServiceRef) );
    _sendingWorkPtr = new boost::asio::io_service::work(_ioServiceRef);
    // initialize multicasting message.
    _reentrantStatusMessage.Clear();
    _reentrantStatusMessage.mutable_status()->set_senderstatus(
            ::sV::events::MulticastMessage_SenderStatusMessage_SenderStatus::
            MulticastMessage_SenderStatusMessage_SenderStatus_OPERATING
        );
    //_sendingMutex.unlock();  // XXX
    // Send initial message.
    send_message( _reentrantStatusMessage );
}

void
iMulticastEventSender::_serialize_message_to_send( const Message & msg ) {
    //msg.SerializeToArray( _serializedMessagePtr, _serializedMessageLength );
    msg.SerializeWithCachedSizesToArray( _serializedMessagePtr );
}

void
iMulticastEventSender::_resize_sending_buffer( size_t newSize ) {
    assert( newSize );
    if( _serializedMessagePtr ) {
        delete [] _serializedMessagePtr;
    }
    _serializedMessagePtr = new UByte [_serializedMessageLength = newSize];
}

// Handlers:

void
iMulticastEventSender::_handle_send_to( const boost::system::error_code& error ) {
    sending_mutex().lock();
    _isOperating = false;
    if( !error && do_continue_transmission() ) {
        _V_send_next_message();
    }
    sending_mutex().unlock();
}

/** Locks the sending mutex which can be unlocked either by sending
 * finished (_handle_send_to).
 *
 * Invoker have to guarantee lifetime of message unitl the end of this
 * procedure.
 * */
void
iMulticastEventSender::send_message( const Message & msg, bool sync ) {
    assert( sync || !_isOperating );
    if( !_thread ) {
        _setup_transmission();
    }
    _isOperating = true;
    size_t serializedLength = msg.ByteSize();
    _serialize_message_to_send( msg );
    sV_log3( "Sending a message of %zu bytes length.\n", serializedLength );
    if( !sync ) {
        _socket.async_send_to(
            boost::asio::buffer(_serializedMessagePtr, serializedLength), _udpEndpoint,
            boost::bind( &iMulticastEventSender::_handle_send_to, this,
                         boost::asio::placeholders::error ));
    } else {
        _socket.send_to(
            boost::asio::buffer(_serializedMessagePtr, serializedLength), _udpEndpoint );
    }
}

}  // namespace net
}  // namespace sV

# endif  // defined(RPC_PROTOCOLS)

