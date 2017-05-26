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

# include "sV_config.h"

# ifdef RPC_PROTOCOLS

# include "analysis/dsources/svbs/buckets.hpp"

namespace sV {
namespace buckets {

//
// RecievingServer
/////////////////

RecievingServer::RecievingServer( int portNo, Buckets * b ) :
                        net::ServerConnection(portNo),
                        _issuerPtr(nullptr),
                        _bucketsAuthority( b ) { }

RecievingServer::~RecievingServer() {
    if( _issuerPtr ) {
        delete _issuerPtr;
    }
}

net::PeerConnection *
RecievingServer::_V_new_connection( int sockID,
                                    const struct sockaddr_in & sain ) {
    if( _issuerPtr ) {
        _issuerPtr->socket_id( sockID );
        _issuerPtr->set_sockaddr( sain );
    } else {
        _issuerPtr = new ClientConnection( _bucketsAuthority, sain );
    }
    return _issuerPtr;
}

void
RecievingServer::_V_free_connection( net::PeerConnection * ptr ) {
    ptr->close();
}

}  // namespace ::sV::buckets
}  // namespace sV

# endif  // RPC_PROTOCOLS

