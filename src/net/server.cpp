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

# include "net/server.hpp"

# include <goo_exception.hpp>

# include <unistd.h>
# include <cstring>

namespace sV {
namespace net {

ServerConnection::ServerConnection() :
                    _sockID(0),
                    _port(0),
                    _listeningSet(false) {
    _sockID = socket(AF_INET, SOCK_STREAM, 0);
    if( _sockID < 0 ) {
        emraise( nwGeneric, "socket(): \"%s\"", strerror(errno) );
    }

    bzero( &_addr, sizeof(_addr) );

    _addr.sin_family = AF_INET;
    _addr.sin_port = htons(_port);
}

ServerConnection::ServerConnection(
            int portNo ) :
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
ServerConnection::_setup_on_loopback_iface() {
    _addr.sin_addr.s_addr = htonl( INADDR_LOOPBACK );
}

void
ServerConnection::_setup_on_any_interface( ) {
    _addr.sin_addr.s_addr = htonl( INADDR_ANY );
    // ^^^ Means that we do accept connections from any interface on current
    // host.
}

void
ServerConnection::_bind_and_listen( uint8_t backlog ) {
    if( ::bind( _sockID, (struct sockaddr *) &_addr, sizeof(_addr) ) < 0 ) {
        emraise( nwGeneric, "bind(): \"%s\"", strerror(errno) );
    }
    if( ::listen( _sockID, backlog ) < 0 ) {
        emraise( nwGeneric, "listen(): \"%s\"", strerror(errno) );
    }
    _listeningSet = true;
}

int
ServerConnection::socket_status_code() {
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
ServerConnection::disconnect() {
    close(_sockID);
}

void
ServerConnection::port( int port_ ) {
    if( _listeningSet ) {
        emraise( badState, "Unable to change destination port since "
            "destination point was set (prev. port %d, new port %d).",
            _port, port_ );
    }
    _port = port_;
}

void
ServerConnection::serve_connections( TreatmentCallback cllb ) {
    int peerSock;
    for(;;) {
        if( 0 > (peerSock = accept(listener, NULL, NULL) )) {
            emraise( nwGeneric, "accept(): \"%s\"", strerror(errno) );
        }
        peerConnectionPtr = new PeerConnection( peerSock );
        cllb( *peerConnectionPtr );
        close( peerSock );
        delete [] peerConnectionPtr;
    }
}

}  // namespace ::sV::net
}  // namespace sV


