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

# ifndef H_STROMA_V_EVENT_MULTICAST_SERVER_H
# define H_STROMA_V_EVENT_MULTICAST_SERVER_H

# include "config.h"

# if defined(RPC_PROTOCOLS)

# include <mutex>
# include <boost/asio.hpp>
# include <boost/atomic.hpp>
# include <boost/thread.hpp>
# include "uevent.hpp"

namespace sV {
namespace net {

/**@class iMulticastEventSender
 * @brief Interface class providing network multicast sender of serialized
 * MulticastMessage objects.
 *
 * Intended to be complementary with iMulticastEventSender class. Implements only
 * network logic. Actual usage implies using of queues, mutex locks, etc.
 *
 * Note: sometimes async sending procedure is faster than
 * constructor initialization of child class, so one needs to
 * somehow synchronize the access to instance. For sure, the
 * most strightforward way will be to include iMulticastEventSender
 * via aggregation instead of inheritance, but it will violate
 * integrity of some architectural patterns, so additional initialization
 * function was introduced.
 * */
class iMulticastEventSender {
public:
    typedef ::sV::events::MulticastMessage Message;
private:
    boost::asio::ip::udp::endpoint _udpEndpoint;
    boost::asio::ip::udp::socket _socket;

    UByte * _serializedMessagePtr;
    size_t _serializedMessageLength;

    Message _reentrantStatusMessage;
    boost::atomic<bool> _isOperating;
    std::mutex _sendingMutex;
    boost::asio::io_service::work * _sendingWorkPtr;
    boost::thread * _thread;

    boost::asio::io_service & _ioServiceRef;
protected:
    /**@brief Event sender interface constructor.
     * */
    iMulticastEventSender(boost::asio::io_service & ioService,
                 const boost::asio::ip::address& multicastAddress,
                 int portNo=30001,
                 size_t sendingBufferSize=1024*1024 /*1 kb default*/);

    virtual void _resize_sending_buffer( size_t newSize );
    void _serialize_message_to_send( const Message & );

    // Handlers:
    /// Invoked after sending done (succeed or not); unlocks sending mutex.
    void _handle_send_to( const boost::system::error_code& error );

    /// Returns wether or not to continue sending.
    virtual bool _V_do_continue_transmission() const = 0;
    /// If _V_do_continue_transmission() returnes true, this must
    // invoke send_message() inside.
    virtual void _V_send_next_message() = 0;

    /// One could call it from child constructor. See notes.
    void _setup_transmission();
public:
    /// Child class should call this method at the end.
    void constructor_done();

    virtual ~iMulticastEventSender();

    bool do_continue_transmission() const { return _V_do_continue_transmission(); }

    boost::asio::ip::udp::socket & socket() { return _socket; }
    const boost::asio::ip::udp::socket & socket() const { return _socket; }

    /// Serializes message and starts (a)synchroneous send. Locks sending mutex.
    void send_message( const Message &, bool sync=false );

    /// Const-getter of sending mutex. May be used in indication of current
    /// state of sending process (wether it runs or not).
    std::mutex & sending_mutex() { return _sendingMutex; }
    bool is_operating() const { return _isOperating; }
};  // class iMulticastEventSender

}  // namespace net
}  // namespace sV

# endif  // defined(RPC_PROTOCOLS)

# endif  // H_STROMA_V_EVENT_MULTICAST_SERVER_H

