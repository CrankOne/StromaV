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

# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>

namespace sV {
namespace net {

/// Rudimentary socket client wrapper.
class Client {
private:
    int _sockID;
    int _port;
    struct sockaddr_in _addr;
protected:
    // ...
public:
    Client();

    /// Returns socket ID (if was bound).
    int socket_id() const { return _sockID; }

    void connect();
    void disconnect();
    size_t send( const char *, size_t );
    size_t recieve( char *, size_t );
};  // SocketConnection

}  // namespace ::sV::net
}  // namespace sV

# endif  // H_STROMA_V_NETWORKING_CLIENT_H

