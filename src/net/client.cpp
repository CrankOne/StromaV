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

# include <unistd.h>
# include <cstring>

namespace sV {
namespace net {

ClientConnection::ClientConnection() :
                    _sockID(0),
                    _port(0),
                    _server(nullptr),
                    _destinationSet(false) {
    _sockID = socket(AF_INET, SOCK_STREAM, 0);
    if( _sockID < 0 ) {
        emraise( nwGeneric, "socket(): \"%s\"", strerror(errno) );
    }

    bzero( &_addr, sizeof(_addr) );

    _addr.sin_family = AF_INET;
    _addr.sin_port = htons(_port);
}

ClientConnection::ClientConnection(
            int portNo,
            const std::string serverHostname ) :
                _sockID( 0 ),
                _port( portNo ),
                _server( nullptr ),
                _destinationSet( false ) {
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
    _server = gethostbyname( hostname.c_str() );
    bcopy(  (char *) server->h_addr,
            (char *) &_addr.sin_addr.s_addr,
            server->h_length );
    _destinationSet = true;
}

void
ClientConnection::connect() {
    if( 0 > ::connect( _sockID, (struct sockaddr *) &_addr, sizeof(_addr)) ) {
        emraise( nwGeneric, "connect(): \"%s\"", strerror(errno) );
    }
}

size_t
ClientConnection::send( const char * data, size_t length ) {
    ssize_t ret = ::send( _sockID, data, length, 0);
    if( ret < 0 ) {
        emraise( nwGeneric, "send(): \"%s\"", strerror(errno) );
    }
    return ret;
}

size_t
ClientConnection::recieve( char * output, size_t maxLength ) {
    size_t acqSize = 0;
    ssize_t ret = recv( _sockID, output, maxLength, 0 );
    if( ret < 0 ) {
        emraise( nwGeneric, "recv(): \"%s\"", strerror(errno) );
    }
}

int
ClientConnection::socket_status_code() {
    int errCode;
    int errCodeSize = sizeof(errCode);
    if( 0 > getsockopt( _sockID, SOL_SOCKET, SO_ERROR,
                &error_code, &errCodeSize ) ) {
        emraise( nwGeneric, "Unable to identify socket error: \"%s\"",
            strerror(errno) );
    }
    return errCode;
}

void
ClientConnection::disconnect() {
    close(_sockID);
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

