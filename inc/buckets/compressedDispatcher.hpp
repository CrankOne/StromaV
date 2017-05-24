/*
 * Copyright (c) 2016 Renat R. Dusaev <crank@qcrypt.org>
 * Author: Renat R. Dusaev <crank@qcrypt.org>
 * Author: Bogdan Vasilishin <togetherwithra@gmail.com>
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
# ifndef H_STROMA_V_COMPR_BUCKET_DISPATCHER_H
# define H_STROMA_V_COMPR_BUCKET_DISPATCHER_H

# include "sV_config.h"

# ifdef RPC_PROTOCOLS

# include "buckets/iTPlainBufferDispatcher.tcc"
# include "compression/iCompressor.hpp"
# include "app/mixins/protobuf.hpp"

# include <ostream>

namespace sV {
namespace buckets {

class CompressedDispatcher : public iTPlainBufferDispatcher<std::allocator> {
public:
    typedef iTPlainBufferDispatcher<std::allocator> Parent;
private:
    iCompressor * _compressor;
    Parent::Buffer _compressedBuffer;
    std::ostream * _streamPtr;
    events::DeflatedBucket * _deflatedBucketPtr;

    struct {
        size_t rawLen, compressedLen;
    } _latestDrop;

    /// Controls whether to pack uncompressed supp info.
    bool _doPackSuppInfo2;
protected:
    virtual void _compressed_buffer_alloc( size_t nBytes );
    virtual void _compressed_buffer_free();

    virtual uint8_t * compressed_buffer_data() {
        return _compressedBuffer.data();
    }

    virtual size_t _V_drop_bucket() override;

    virtual size_t _compress_bucket();
protected:
    CompressedDispatcher(   iCompressor * compressorPtr,
                            std::ostream * streamPtr,
                            events::BucketInfo * biEntriesPtr,
                            bool doPackSuppinfo=true);
public:
    CompressedDispatcher(   iCompressor * compressorPtr,
                            std::ostream & streamRef ) :
            CompressedDispatcher( compressorPtr, &streamRef,
                google::protobuf::Arena::CreateMessage<events::BucketInfo>(::sV::mixins::PBEventApp::arena_ptr()) ) {}

    CompressedDispatcher(   iCompressor * compressorPtr ) :
            CompressedDispatcher( compressorPtr, nullptr,
                google::protobuf::Arena::CreateMessage<events::BucketInfo>(::sV::mixins::PBEventApp::arena_ptr()) ) {}

    virtual ~CompressedDispatcher();

    void set_out_stream( std::ostream & strRef ) { _streamPtr = &strRef; }
    std::ostream * get_out_stream_ptr() { return _streamPtr; }
    bool stream_is_set() const { return !!_streamPtr; }

    iCompressor & compressor() { return *_compressor; }

    size_t latest_dropped_raw_len() const { return _latestDrop.rawLen; }
    size_t latest_dropped_compressed_len() const { return _latestDrop.compressedLen; }

    virtual bool do_pack_suppinfo() const override { return _doPackSuppInfo2; }
    virtual void do_pack_suppinfo( bool v ) override { _doPackSuppInfo2 = v; }

    size_t compressed_buffer_length() const { return _compressedBuffer.size(); }
    virtual const uint8_t * compressed_buffer_data() const {
        return _compressedBuffer.data();
    }
};  // class CompressedDispatcher

}  // namespace ::sV::buckets
}  // namespace sV

# endif  //  RPC_PROTOCLS
# endif  //  H_STROMA_V_COMPR_BUCKET_DISPATCHER_H

