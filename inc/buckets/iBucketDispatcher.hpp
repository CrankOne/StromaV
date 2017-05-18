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
# ifndef H_STROMA_V_IBUCKET_DISPATCHER_H
# define H_STROMA_V_IBUCKET_DISPATCHER_H

# include "sV_config.h"

# ifdef RPC_PROTOCOLS

# include "uevent.hpp"

# include <goo_dict/dict.hpp>

# include <openssl/sha.h>

# include <unordered_map>

namespace sV {

class iBucketDispatcher;

/// An abstract interfacing base for bucket metainformation collector.
class iAbstractBucketMetaInfoCollector {
protected:
    /// (IM) Has to perform consideration of an event.
    virtual void _V_consider_event( const events::Event & ) = 0;

    /// (IM) Has to clear accumulated metainformation.
    virtual void _V_clear() = 0;

    /// (IM) This method has to pack accumulated metainformation to bucket
    /// message. The source is not specified for this base class for the sake
    /// of generality.
    virtual void _V_pack_suppinfo( ::google::protobuf::Any*,
                                   const iBucketDispatcher & ) = 0;

    /// (IM) This method has to obtain accumulated metainformation from bucket
    /// message. The destination is not specified for this base class for the
    /// sake of generality.
    virtual void _V_unpack_suppinfo( const ::google::protobuf::Any & ) = 0;
public:
    iAbstractBucketMetaInfoCollector() {}

    virtual ~iAbstractBucketMetaInfoCollector() {}

    /// Considers an event and appends metainformation.
    virtual void consider_event( const events::Event & eve )
        { _V_consider_event( eve ); }

    /// Clears accumulated meta information.
    virtual void clear() { _V_clear(); }

    /// Packs accumulated metainformation to bucket message.
    void pack_suppinfo( ::google::protobuf::Any * destPtr,
                        const iBucketDispatcher & ibdsp )
        { _V_pack_suppinfo( destPtr, ibdsp ); }

    /// Obtains accumulated metainformation from bucket message.
    void unpack_suppinfo( const ::google::protobuf::Any & srcPtr )
        { _V_unpack_suppinfo( srcPtr ); }
};

/// Provides a somewhat standard implementation for (un)packing bucket metainfo.
/// The _V_consider_event() and _V_clear() is still has to be implemented.
template<typename T>
class ITBucketSuppInfoCollector : public iAbstractBucketMetaInfoCollector {
private:
    T * _miPtr;
protected:
    /// Packs internal message of specific type into protobyf's Any field
    /// referred by given ptr.
    virtual void _V_pack_suppinfo(
                        ::google::protobuf::Any* miMsgRef,
                        const iBucketDispatcher & ) override {
        miMsgRef->PackFrom( *_my_supp_info_ptr() );
    }

    /// Uses internal message of specific type as destination to unpack from
    /// protobyf's Any field referred by given ptr.
    virtual void _V_unpack_suppinfo( const ::google::protobuf::Any & miMsgRef ) override {
        miMsgRef.UnpackTo( _my_supp_info_ptr() );
    }

    /// May be used by descendants to perform invasive operations with supp
    /// info message data.
    virtual T * _my_supp_info_ptr() { return _miPtr; }
public:
    /// Ctr. The pointer of reentrant metainfo instance is supposed to be
    /// allocated using arena.
    ITBucketSuppInfoCollector( T * miPtr ) : _miPtr(miPtr) {}
};

/**@class CommonBucketDescription
 * @brief Performs accumulation of events number stored in a bucket and unique
 *        SHA256 hash of bucket content.
 *
 * A generic supp info collector. One may wish to insert this collector
 * instance to bucket dispatcher chains to provide basic buckets
 * identifiaction and integrity checks using SHA256 digests.
 */
class CommonBucketDescription :
            public ITBucketSuppInfoCollector<events::CommonBucketDescriptor> {
public:
    typedef ITBucketSuppInfoCollector<events::CommonBucketDescriptor> Parent;
private:
    uint8_t _hash[SHA256_DIGEST_LENGTH];
    size_t _nEvents;
protected:
    /// Increases events counter.
    virtual void _V_consider_event( const events::Event & ) override;

    /// Clears hash function and events counter.
    virtual void _V_clear() override;

    /// Computes hash.
    virtual void _V_pack_suppinfo(
                ::google::protobuf::Any* miMsgRef,
                const iBucketDispatcher & ) override;
public:
    CommonBucketDescription( events::CommonBucketDescriptor * );
    CommonBucketDescription( const goo::dict::Dictionary & );
};


/**@class iBucketDispatcher
 * @brief Helper event accumulation class.
 *
 * The idea of "bucket" is quite similar to ROOT's "basket" and in a nutshell
 * is just a buffer containing discrete number of events. These "buckets" can
 * be further compressed and forwarded to storage/stdout/network as a solid
 * data chunks.
 *
 * One need such a thing because it leads for much more efficient compression
 * and transmission comparing to ordinary per-event basis.
 *
 * This class may represents an automatic buffer that will accumulate data
 * (events) until associated buffer will not be full.
 *
 * One can choose which bucket this processor has to consider as "full": either
 * by entries number or by uncompressed size. If both criteria will be non-null,
 * the first matching criterion will trigger bucket to be dropped. If both
 * criteria are set to 0, the handler will never drop the buket until handler's
 * destructor will be invoked.
 * */
class iBucketDispatcher {
public:
    typedef std::unordered_map<std::string, iAbstractBucketMetaInfoCollector *>
        CollectorsMap;

    typedef events::Bucket Bucket;
private:
    /// Upper data size limit for accumulation (in bytes)
    size_t _nBytesMax;
    /// Upper size limit for accumulation (in number of events)
    size_t _nMaxEvents;
    /// Whether to append supp information to dropped bucket
    bool _doPackMetaInfo;
protected:
    /// (IM) Shall define how to perform "drop"
    virtual size_t _V_drop_bucket() = 0;

    /// Buffering bucket instance
    events::Bucket * _rawBucketPtr;

    /// Supp info collectors associated with this dispatcher instance
    CollectorsMap _miCollectors;

    /// Returns mutable container of associated collectors
    CollectorsMap & metainfo_collectors();

    /// Will append supp info from all associated collectors.
    void _append_suppinfo( events::BucketInfo & );
public:
    /// Ctr getting drop criteria. When doPackMetaInfo is set and metainfo
    /// collector is provided, the dropping method will automatically invoke
    /// pack_metainfo() method of internal metainfo collector prior to
    /// _V_drop_bucket().
    iBucketDispatcher( size_t nMaxKB, size_t nMaxEvents, bool doPackMetaInfo=true );

    virtual ~iBucketDispatcher();

    /// Returns current buffering bucket instance
    virtual events::Bucket & bucket() { return *_rawBucketPtr; }

    /// Returns current buffering bucket instance (const getter)
    virtual const events::Bucket & bucket() const { return *_rawBucketPtr; }

    /// Performs consideration of event. Upon one of the above criteria is
    /// reached, invokes drop_bucket().
    virtual void push_event(const events::Event & );

    /// Returns size limit (in kBs)
    size_t n_max_KB() const { return _nBytesMax/1024; }

    /// Returns size limit (in bytes)
    size_t n_max_bytes() const { return _nBytesMax; }

    /// Returns size limit (in events)
    size_t n_max_events() const { return _nMaxEvents; }

    /// Returns current size (in bytes)
    size_t n_bytes() const {
        return (size_t)(bucket().ByteSize());
    };

    /// Returns current size (in events)
    size_t n_events() const {
        return (size_t)(bucket().events_size());
    };

    /// Returns true when one of the size limits is reached
    virtual bool is_bucket_full();

    /// Returns true if no events had been considered after last drop (or
    /// instance creation)
    virtual bool is_bucket_empty();

    /// Performs dropping procedure (usually defined by subclasses) if bucket
    /// is not empty.
    virtual size_t drop_bucket();

    /// Clears current state of an instance
    virtual void clear_bucket();

    /// Whether to append supplementary information to dropped bucket
    virtual bool do_pack_metainfo() const { return !! _doPackMetaInfo; }

    /// Sets the supplementary information flag
    virtual void do_pack_metainfo( bool v ) { _doPackMetaInfo = v; }

    /// Returns true if there are supp info collectors associated with an
    /// instance
    bool are_metainfo_collectors_set() const { return !metainfo_collectors().empty(); }

    /// Associates the collectors with an instance
    void metainfo_collectors( CollectorsMap & micPtr );

    /// Returns supp info collectors index associated with this instance
    const CollectorsMap & metainfo_collectors() const;
};  // class iBucketDispatcher

}  //  namespace sV

# endif  //  RPC_PROTOCOLS
# endif  //  H_STROMA_V_IBUCKET_DISPATCHER_H

