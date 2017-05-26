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

# ifndef H_STROMA_V_NETWORKING_SERVER_H
# define H_STROMA_V_NETWORKING_SERVER_H

# include "sV_config.h"

# include "peer.hpp"

# include <string>

namespace sV {
namespace net {

/**@brief Implements rudimentary socket server wrapper.
 * @class ServerSocket
 *
 * This a pretty simple wrapper around native <sys/socket.h> routines providing
 * typical handles for managing server network connection via blocking AF_INET
 * socket.
 **/
class ServerConnection {
public:
    typedef void (*TreatmentCallback)( PeerConnection * );
private:
    int _sockID,    ///< listening socket, std socket identifier
        _port;      ///< port number
    struct sockaddr_in _addr;
    bool _listeningSet;
protected:
    /// Sets up INADDR_LOOPBACK socket for listening.
    void _setup_on_loopback_iface();
    /// Sets up on any interface for listening.
    void _setup_on_any_interface();
    /// Binds the socket prior to listening.
    void _bind_and_listen( uint8_t backlog=128 );

    /// (IM) Has to allocate and return new PeerConnection instance (or its
    /// descendant). Will be called after new incoming connection is accepted.
    virtual PeerConnection * _V_new_connection( int, const struct sockaddr_in & ) = 0;
    /// (IM) Has to free PeerConnection instance (or its descendant) upon
    /// worker completes.
    virtual void _V_free_connection( PeerConnection * ) = 0;
public:
    /// Constructs unbound socket.
    ServerConnection();
    /// Constructs socket and sets it up for listening.
    ServerConnection( int portNo );
    /// Returns listening socket ID (if was bound).
    int listening_socket_id() const { return _sockID; }
    /// Closes the socket.
    virtual void shutdown();
    /// Port number getter.
    int listening_port() const { return _port; }
    /// Prot number setter.
    void listening_port( int );

    /// Internally, calls the 2 accept() function. It blocks the current
    /// execution until next incoming connection on listening port will come.
    /// Once it came, the PeerConnection instance will be created and the given
    /// treatment callback will be invoked.
    void serve_connections( TreatmentCallback cllb );
};  // ServerConnection

}  // namespace ::sV::net
}  // namespace sV

# endif  // H_STROMA_V_NETWORKING_SERVER_H

