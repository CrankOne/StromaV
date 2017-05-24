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

# ifndef H_STROMA_V_TEMPLATE_INTERFACE_PLAIN_BUFFER_DISPATCHER_H
# define H_STROMA_V_TEMPLATE_INTERFACE_PLAIN_BUFFER_DISPATCHER_H

# include "sV_config.h"

# ifdef RPC_PROTOCOLS

# include "buckets/iBundlingDispatcher.hpp"

namespace sV {
namespace buckets {

class IPlainBufferDispatcher : public iBundlingDispatcher {
public:
    /// Shall return internal buffer with serialized bucket.
    virtual const uint8_t * raw_buffer_data() const = 0;

    /// Shall return the length of internal buffer with serialized bucket.
    virtual size_t raw_buffer_length() const = 0;

    IPlainBufferDispatcher( events::BucketInfo * biEntriesPtr,
                            bool doPackSuppInfo ) :
                iBundlingDispatcher( biEntriesPtr, doPackSuppInfo ) {}
};  // class iPlainBufferDispatcher


template<template<class> class AllocatorT>
class iTPlainBufferDispatcher : public IPlainBufferDispatcher {
public:
    typedef AllocatorT<uint8_t> Allocator;
    typedef std::vector<uint8_t, AllocatorT<uint8_t> > Buffer;
private:
    /// Allocator instance.
    Allocator _allocator;

    /// Internal buffer for serialized data.
    Buffer _buffer;
protected:
    virtual void _raw_buffer_alloc( size_t nBytes ) {
        _buffer.resize( nBytes );
    }
    virtual void _raw_buffer_free() {
        _buffer.clear();
    }

    virtual uint8_t * raw_buffer_data() {
        return _buffer.data();
    }
public:
    iTPlainBufferDispatcher( events::BucketInfo * biEntriesPtr,
                             bool doPackSuppInfo=true,
                             const Allocator& alloc=Allocator() ) :
                IPlainBufferDispatcher( biEntriesPtr, doPackSuppInfo ),
                _allocator(alloc),
                _buffer(_allocator) {}

    virtual const uint8_t * raw_buffer_data() const override {
        return _buffer.data();
    }

    virtual size_t raw_buffer_length() const override {
        return _buffer.size();
    }
};  // iTPlainBufferDispatcher

}  // namespace buckets
}  // namespace sV

# endif  // RPC_PROTOCOLS

# endif  // H_STROMA_V_TEMPLATE_INTERFACE_PLAIN_BUFFER_DISPATCHER_H

