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

# include "net/client.hpp"

# include <goo_exception.hpp>

# include <netdb.h>
# include <unistd.h>

# include <cstring>

namespace sV {
namespace net {

ClientConnection::ClientConnection() :
                    _port(0),
                    _destinationSet(false),
                    _server(nullptr) {
    socket_id( socket(AF_INET, SOCK_STREAM, 0) );
    if( socket_id() < 0 ) {
        emraise( nwGeneric, "socket(): %s.", strerror(errno) );
    }

    bzero( &_addr, sizeof(_addr) );

    _addr.sin_family = AF_INET;
    _addr.sin_port = htons(_port);
}

ClientConnection::ClientConnection(
            int portNo,
            const std::string serverHostname ) :
                _port( portNo ),
                _destinationSet( false ),
                _server( nullptr ) {
    socket_id( socket(AF_INET, SOCK_STREAM, 0) );
    if( socket_id() < 0 ) {
        emraise( nwGeneric, "socket(): %s.", strerror(errno) );
    }

    bzero( &_addr, sizeof(_addr) );

    _addr.sin_family = AF_INET;
    _addr.sin_port = htons(_port);

    if( serverHostname.empty()
     || "loopback" == serverHostname ) {
        _setup_loopback_destination();
    } else {
        _setup_destination_host( serverHostname );
    }
}

void
ClientConnection::_setup_loopback_destination() {
    _addr.sin_addr.s_addr = htonl( INADDR_LOOPBACK );
    _destinationSet = true;
}

void
ClientConnection::_setup_destination_host( const std::string & hostname ) {
    _server = ::gethostbyname( hostname.c_str() );
    bcopy(  (char *) _server->h_addr,
            (char *) &_addr.sin_addr.s_addr,
            _server->h_length );
    _destinationSet = true;
}

void
ClientConnection::connect() {
    if( 0 > (::connect( socket_id(), (struct sockaddr *) &_addr, sizeof(_addr))) ) {
        emraise( nwGeneric, "connect(): %s.", strerror(errno) );
    }
}

void
ClientConnection::port( int port_ ) {
    if( _destinationSet ) {
        emraise( badState, "Unable to change destination port since "
            "destination point was set (prev. port %d, new port %d).",
            _port, port_ );
    }
    _port = port_;
}

}  // namespace ::sV::net
}  // namespace sV

