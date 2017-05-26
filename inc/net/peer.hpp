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

# ifndef H_STROMA_V_NETWORKING_PEER_H
# define H_STROMA_V_NETWORKING_PEER_H

# include "sV_config.h"

# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>

namespace sV {
namespace net {

class PeerConnection {
private:
    int _sockID; ///< std socket identifier
    struct sockaddr_in _addr;
protected:
    struct sockaddr_in & addr();
public:
    PeerConnection();
    PeerConnection( const struct sockaddr_in & );
    virtual ~PeerConnection();
    const struct sockaddr_in & addr() const { return _addr; }
    int socket_id() const { return _sockID; }
    /// Wraps native send() call with error-checking code. If error occurs
    /// (send() returned <0), nwgeneric exception will be thrown. Returns
    /// number of bytes sent.
    size_t send( const char *, size_t );
    /// Wraps native recv() call with error-checking code. If error occurs
    /// (recv() returned <0), nwgeneric exception will be thrown. Returns
    /// number of bytes recieved.
    size_t recieve( char *, size_t );
    /// Closes the socket.
    void close();

    /// Uses getsockopt() to obtain and return latest SO_ERROR code. Resets the
    /// socket. If getsockopt() fails, raises nwGeneric exception.
    static int socket_status_code( int );

    /// Raises a badState, if connection was not previously closed.
    virtual void socket_id( int );
    virtual void reset_socket_id() { _sockID = 0; }

    virtual void set_sockaddr( const struct sockaddr_in & );
};  // class PeerConnection

}  // namespace ::sV::net
}  // namespace sV

# endif  // H_STROMA_V_NETWORKING_PEER_H


