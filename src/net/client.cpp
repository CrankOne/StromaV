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

Client::Client() : _sockID(0),
                   _port(0) {
    _sockID = socket(AF_INET, SOCK_STREAM, 0);
    if( _sockID < 0 ) {
        emraise( nwGeneric, "socket(): \"%s\"", strerror(errno) );
    }

    bzero( &_addr, sizeof(_addr) );

    _addr.sin_family = AF_INET;
    _addr.sin_port = htons(_port);
    _addr.sin_addr.s_addr = htonl( INADDR_LOOPBACK );  // TODO: addr
}

void
Client::connect() {
    if( 0 > ::connect( _sockID, (struct sockaddr *) &_addr, sizeof(_addr)) ) {
        emraise( nwGeneric, "connect(): \"%s\"", strerror(errno) );
    }
}

size_t
Client::send( const char * data, size_t length ) {
    ssize_t ret = ::send( _sockID, data, length, 0);
    if( ret < 0 ) {
        emraise( nwGeneric, "send(): \"%s\"", strerror(errno) );
    }
    // TODO: more alborated code.
    // see, e.g. https://stackoverflow.com/questions/12402549/check-if-socket-is-connected-or-not
    return ret;
}

size_t
Client::recieve( char * output, size_t maxLength ) {
    size_t acqSize = 0;
    ssize_t lastRet = recv( _sockID, output, maxLength, 0 );
    _TODO_  // TODO: more alborated code.
            // see, e.g. https://stackoverflow.com/questions/8470403/socket-recv-hang-on-large-message-with-msg-waitall
}

void
Client::disconnect() {
    close(_sockID);
}

}  // namespace ::sV::net
}  // namespace sV

