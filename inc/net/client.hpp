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

# ifndef H_STROMA_V_NETWORKING_CLIENT_H
# define H_STROMA_V_NETWORKING_CLIENT_H

# include "sV_config.h"

# include "peer.hpp"

# include <string>

struct hostent;

namespace sV {
namespace net {

/**@brief Implements rudimentary socket client wrapper.
 * @class ClientSocket
 *
 * This a pretty simple wrapper around native <sys/socket.h> routines providing
 * typical handles for managing client network connection via blocking AF_INET
 * socket.
 **/
class ClientConnection : public PeerConnection {
private:
    int _port;      ///< port number
    struct sockaddr_in _addr;
    /// True if either loopback or destination point was set.
    bool _destinationSet;
    /// Refers to particular host, for non-loopback connections.
    struct hostent * _server;
protected:
    /// Sets up INADDR_LOOPBACK socket for transmission. Only port number has
    /// to be set.
    void _setup_loopback_destination();
    /// Sets up socket for transmission with remote destination point
    /// identified with string hostname.
    void _setup_destination_host( const std::string & hostname );
public:
    /// Constructs unbound socket.
    ClientConnection();
    /// Constructs socket and sets it up for recieving/sending. Set
    /// serverHostname to an empty string or to "loopback" string to set up the
    /// loopback socket.
    ClientConnection( int portNo, const std::string serverHostname );
    /// Establishes a connection with native connect() function. Will throw
    /// nwGeneric, if connect() return <0.
    void connect();
    /// Port number getter.
    int port() const { return _port; }
    /// Prot number setter.
    void port( int );
};  // ClientConnection

}  // namespace ::sV::net
}  // namespace sV

# endif  // H_STROMA_V_NETWORKING_CLIENT_H

