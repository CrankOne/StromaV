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

# include "net/peer.hpp"

# include <goo_exception.hpp>

# include <cstring>
# include <cassert>

# include <unistd.h>

namespace sV {
namespace net {

PeerConnection::PeerConnection() : _sockID(0) {
    bzero( &_addr, sizeof(_addr) );
}

PeerConnection::PeerConnection( int sockID, const struct sockaddr_in & o ) :
                _sockID(sockID) {
    set_sockaddr( o );
}

PeerConnection::~PeerConnection() {
}

size_t
PeerConnection::send( const char * data, size_t length ) {
    ssize_t ret = ::send( socket_id(), data, length, 0);
    if( ret < 0 ) {
        emraise( nwGeneric, "send(): %s.", strerror(errno) );
    }
    return ret;
}

size_t
PeerConnection::recieve( char * output, size_t maxLength ) {
    assert( _sockID );
    ssize_t ret = recv( socket_id(), output, maxLength, 0 );
    if( ret < 0 ) {
        emraise( nwGeneric, "recv(): %s.", strerror(errno) );
    }
    return ret;
}

int
PeerConnection::socket_status_code( int socketID ) {
    int errCode;
    ::socklen_t errCodeSize = sizeof(errCode);
    if( 0 > getsockopt( socketID, SOL_SOCKET, SO_ERROR,
                &errCode, &errCodeSize ) ) {
        emraise( nwGeneric, "Unable to identify socket error: \"%s\"",
            strerror(errno) );
    }
    return errCode;
}

void
PeerConnection::close() {
    if( _sockID ) {
        ::close( socket_id() );
        reset_socket_id();
    }
}

void
PeerConnection::socket_id( int val ) {
    if( _sockID ) {
        emraise( badState, "PeerConnection %p already has socket identifier set. "
            "Did you forget to close() it?", this );
    }
    if( !val ) {
        emraise( badState, "PeerConnection %p trying to set zero socket "
            "descriptor. If it is implied by design, use reset_socket_id() "
            "instead.", this );
    }
    _sockID = val;
}

int
PeerConnection::socket_id() const {
    if( !_sockID ) {
        emraise( badState, "Socket identifier is null. Connection wasn't "
            "initialized." );
    }
    return _sockID;
}

void
PeerConnection::set_sockaddr( const struct sockaddr_in & o ) {
    memcpy( &_addr, &o, sizeof(struct sockaddr_in) );
}

}  // namespace ::sV::net
}  // namespace sV

