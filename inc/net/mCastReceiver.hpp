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

# ifndef H_STROMA_V_EVENT_MULTICAST_CLIENT_H
# define H_STROMA_V_EVENT_MULTICAST_CLIENT_H

# include "sV_config.h"

# if defined(RPC_PROTOCOLS)

# include <boost/asio.hpp>

# include "uevent.hpp"

namespace sV {
namespace net {

/**@class iMulticastEventReceiver
 * @brief Interface class providing network multicast reciever of serialized
 * MulticastMessage objects.
 *
 * Intended to be complementary with iMulticastEventSender class.
 * */
class iMulticastEventReceiver {
public:
    typedef ::sV::events::MulticastMessage Message;
private:
    boost::asio::ip::udp::socket _udpSocket;
    boost::asio::ip::udp::endpoint _udpSenderEndpoint;
    UByte * _dataReentrantBufferPtr;
    size_t _dataReentrantBufferLength,
           _acceptedMessages,
           _declinedMessages;
    //Message _reentrantMessageInstance;

    /**@brief Receiver handling method.
     *
     * Accepts serialized event, deserializes it and performs virtual
     * methods invokation. Should not be called directly from descendant
     * code.
     * @param error a boost error-descriptive code, if any.
     * @param nBytesRecvd a number of bytes recieved.
     * */
    void _handle_receive( const boost::system::error_code& error,
                          size_t nBytesRecvd );
protected:
    /**@brief Event receiver interface constructor.
     *
     * Constructs the receiver part of descendant class. Immediately
     * creates the socket and sets handlers.
     *
     * @param ioService a boost I/O service instance;
     * @param listenAddress a multicast listening address; Set to 0.0.0.0
     * for local connections.
     * @param multicastAddress a multicast listening address; Set, for
     * instance, to 239.255.0.1 for local connections.
     * @param bufferLength a maximal serialized message length.
     * */
    iMulticastEventReceiver( boost::asio::io_service & ioService,
                    const boost::asio::ip::address & listenAddress,
                    const boost::asio::ip::address & multicastAddress,
                    int portNo=30001,
                    size_t bufferLength=1024*1024 /*1 kb default*/ );
    /**@brief Reallocates buffer. 
     * */
    virtual void _reallocate_reentrant_buffer( size_t newSize );

    /**@brief Interface function that is called on event received.
     *
     * Should return false, when receiving should be interrupt.
     * */
    virtual bool _V_treat_incoming_message( const unsigned char *, size_t ) = 0;
public:
    virtual ~iMulticastEventReceiver();

    /// Returns false on treatment failure.
    bool treat_incoming_message( const unsigned char * msg, size_t length ) {
        if( _V_treat_incoming_message(msg, length) ) {
            ++_acceptedMessages;
            return true;
        } else {
            ++_declinedMessages;
            return false;
        }
    }
};  // class EventReceiver

}  // namespace net
}  // namespace sV

# else // defined(RPC_PROTOCOLS)
# error "Event multicasting routines requires RPC protocols in StromaVlib build."
# endif  // defined(RPC_PROTOCOLS) && defined(ANALYSIS_ROUTINES)
# endif  // H_STROMA_V_EVENT_MULTICAST_CLIENT_H

