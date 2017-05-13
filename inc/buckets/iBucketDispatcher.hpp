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
# include <boost/program_options.hpp>

namespace sV {

/// An abstract interfacing base for bucket metainformation collector.
class iAbstractBucketMetaInfoCollector {
private:
    size_t _nEvents;
protected:
    /// Has to perform consideration of an event.
    virtual void _V_consider_event( const events::Event & ) = 0;
    /// Has to clear accumulated metainformation.
    virtual void _V_clear() = 0;
    /// This method has to pack accumulated metainformation to bucket message.
    /// The source is not specified for this base class for the sake of
    /// generality.
    virtual void _V_pack_metainfo( events::BucketMetaInfo & ) = 0;
    /// This method has to obtain accumulated metainformation from bucket message.
    /// The destination is not specified for this base class for the sake of
    /// generality.
    virtual void _V_unpack_metainfo( const events::BucketMetaInfo & ) = 0;
public:
    iAbstractBucketMetaInfoCollector() : _nEvents(0) {}

    virtual ~iAbstractBucketMetaInfoCollector() {}

    /// Returns number of considered events.
    size_t n_events() const { return _nEvents; }

    /// Considers an event and appends metainformation.
    void consider_event( const events::Event & eve ) {
        ++_nEvents;
        _V_consider_event( eve );
    }

    /// Clears accumulated meta information.
    void clear() {
        _nEvents = 0;
        _V_clear();
    }

    /// Packs accumulated metainformation to bucket message.
    void pack_metainfo( events::BucketMetaInfo & miMsgRef )
        { _V_pack_metainfo( miMsgRef ); }

    /// Obtains accumulated metainformation from bucket message.
    void unpack_metainfo( const events::BucketMetaInfo & miMsgCRef )
        { _V_unpack_metainfo( miMsgCRef ); }
};

/// Provides a somewhat standard implementation for (un)packing bucket metainfo.
/// The _V_consider_event() and _V_clear() is still has to be implemented.
template<typename T>
class ITBucketMetaInfoCollector : public iAbstractBucketMetaInfoCollector {
private:
    T * _miPtr;
protected:
    virtual void _V_set_bucket_meta_info( const T & mi, events::BucketMetaInfo & msg ) {
        msg.mutable_suppinfo()->PackFrom( mi );
    }
    virtual void _V_get_bucket_meta_info( const events::BucketMetaInfo & msg, T & mi ) {
        msg.suppinfo().UnpackTo( &mi );
    }
    virtual void _V_pack_metainfo( events::BucketMetaInfo & miMsgRef ) override {
        set_bucket_meta_info( *_miPtr, miMsgRef );
    }
    virtual void _V_unpack_metainfo( const events::BucketMetaInfo & miMsgRef ) override {
        get_bucket_meta_info( miMsgRef, *_miPtr );
    }
public:
    /// Ctr. The pointer of reentrant metainfo instance is supposed to be
    /// allocated using arena.
    ITBucketMetaInfoCollector( T * miPtr ) : _miPtr(miPtr) {}

    void set_bucket_meta_info( const T & cDat, events::BucketMetaInfo & msgRef )
        { _V_set_bucket_meta_info( cDat, msgRef ); }

    void get_bucket_meta_info( const events::BucketMetaInfo & msgCRef, T & cDatRef )
        { _V_get_bucket_meta_info( msgCRef, cDatRef ); }
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
private:
    size_t _nBytesMax;
    size_t _nMaxEvents;
    bool _doPackMetaInfo;
protected:
    virtual size_t _V_drop_bucket() = 0;
    events::Bucket * _rawBucketPtr;
    iAbstractBucketMetaInfoCollector * _miCollectorPtr;

    iAbstractBucketMetaInfoCollector & metainfo_collector();
public:
    typedef sV::events::Bucket Bucket;

    /// Ctr getting drop criteria. When doPackMetaInfo is set and metainfo
    /// collector is provided, the dropping method will automatically invoke
    /// pack_metainfo() method of internal metainfo collector prior to
    /// _V_drop_bucket().
    iBucketDispatcher( size_t nMaxKB, size_t nMaxEvents, bool doPackMetaInfo=true );
    virtual ~iBucketDispatcher();

    events::Bucket & bucket() { return *_rawBucketPtr; }

    const events::Bucket & bucket() const { return *_rawBucketPtr; }

    virtual void push_event(const events::Event & );

    size_t n_max_KB() const { return _nBytesMax/1024; }
    size_t n_max_bytes() const { return _nBytesMax; }
    size_t n_max_events() const { return _nMaxEvents; }

    size_t n_bytes() const {
        return (size_t)(bucket().ByteSize());
    };
    size_t n_events() const {
        return (size_t)(bucket().events_size());
    };

    virtual bool is_bucket_full();
    virtual bool is_bucket_empty();
    size_t drop_bucket();
    virtual void clear_bucket();

    virtual bool do_pack_metainfo() const { return _doPackMetaInfo; }
    virtual void do_pack_metainfo( bool v ) { _doPackMetaInfo = v; }
    bool is_metainfo_collector_set() const { return _miCollectorPtr; }
    void metainfo_collector( iAbstractBucketMetaInfoCollector & micPtr );
    const iAbstractBucketMetaInfoCollector & metainfo_collector() const;
};  // class iBucketDispatcher

}  //  namespace sV

# endif  //  RPC_PROTOCOLS
# endif  //  H_STROMA_V_IBUCKET_DISPATCHER_H

