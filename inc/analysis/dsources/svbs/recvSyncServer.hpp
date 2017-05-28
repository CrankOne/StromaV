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

# ifndef H_STROMA_V_SVBP_READER_BUCKET_RECIEVING_SYNC_SERVER_H
# define H_STROMA_V_SVBP_READER_BUCKET_RECIEVING_SYNC_SERVER_H

# include "sV_config.h"

# ifdef RPC_PROTOCOLS

# include "net/server.hpp"

namespace sV {

class Buckets;

namespace buckets {

class RecievingServer : public net::ServerConnection {
public:
    class ClientConnection : public net::PeerConnection {
    private:
        Buckets * _authority;
    protected:
        Buckets & authority() { return *_authority; }
    public:
        ClientConnection(
            int socketID,
            Buckets * a,
            const struct sockaddr_in & sain) : 
                        net::PeerConnection(socketID, sain),
                        _authority(a) {}
        friend class ::sV::Buckets;
    };
private:
    ClientConnection * _issuerPtr;
    Buckets * _bucketsAuthority;
protected:
    /// Returns ClientConnection instance. Will be called after incoming
    /// connection is accepted.
    virtual net::PeerConnection * _V_new_connection( int, const struct sockaddr_in & ) override;
    /// Clears ClientConnection instance.
    virtual void _V_free_connection( net::PeerConnection * ) override;
public:
    RecievingServer( int portNo, Buckets * );
    virtual ~RecievingServer();
};  // RecievingServer

}  // namespace ::sV::buckets
}  // namespace sV

# endif  // RPC_PROTOCOLS

# endif  // H_STROMA_V_SVBP_READER_BUCKET_RECIEVING_SYNC_SERVER_H


