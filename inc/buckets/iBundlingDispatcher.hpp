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

# ifndef H_STROMA_V_IBUCKET_BUNDLING_DISPATCHER_H
# define H_STROMA_V_IBUCKET_BUNDLING_DISPATCHER_H

# include "sV_config.h"

# ifdef RPC_PROTOCOLS

# include "iDispatcher.hpp"

# include "ctrs_dict.hpp"

# include <openssl/sha.h>

# include <memory>

namespace sV {
namespace buckets {

class iBundlingDispatcher;

/**@class iAbstractInfoCollector
 * @brief An abstract interfacing base for bucket supplementary information
 *        collector.
 *
 * Descendants of this base class are used for both, packing and unpacking the
 * supplementary information about sV's events buckets.
 *
 * These info collectors can decide whether the bucket has to be "dropped"
 * within iBundlingDispatcher interface implementations.
 *
 * @ingroup buckets
 */
class iAbstractInfoCollector {
private:
    /// Will set to true once the _V_consider_event() returned true and re-set
    /// to false after clear().
    bool _doDrop;
protected:
    /// (IM) Has to perform consideration of an event.
    /// Shall return true when bucket needs to be dropped.
    virtual bool _V_consider_event( const events::Event &,
                                    const iBundlingDispatcher & ) = 0;
    /// (IM) Has to clear accumulated supp information.
    virtual void _V_clear() = 0;
    /// (IM) This method has to pack accumulated supp information to bucket
    /// message. The source is not specified for this base class for the sake
    /// of generality.
    virtual void _V_pack_suppinfo( ::google::protobuf::Any*,
                                   const iBundlingDispatcher & ) = 0;
    /// (IM) This method has to obtain accumulated supp information from bucket
    /// message. The destination is not specified for this base class for the
    /// sake of generality.
    virtual void _V_unpack_suppinfo( const ::google::protobuf::Any & ) = 0;
public:
    iAbstractInfoCollector() : _doDrop(false) {}
    virtual ~iAbstractInfoCollector() {}
    /// Considers an event and appends supp information.
    virtual bool consider_event( const events::Event & eve,
                                 const iBundlingDispatcher & ibdsp )
        { return _doDrop |= _V_consider_event( eve, ibdsp ); }
    /// Clears accumulated supp information.
    virtual void clear() { _V_clear(); _doDrop = false; }
    /// Packs accumulated supp information to bucket message.
    void pack_suppinfo( ::google::protobuf::Any * destPtr,
                        const iBundlingDispatcher & ibdsp )
        { _V_pack_suppinfo( destPtr, ibdsp ); }
    /// Obtains accumulated supp information from bucket message.
    void unpack_suppinfo( const ::google::protobuf::Any & srcPtr )
        { _V_unpack_suppinfo( srcPtr ); }
    /// Will return true after at least one invokation of _V_consider_event()
    /// returned true. Starts return false after clear() will be invoked.
    bool do_drop() const { return _doDrop; }
};

/// Provides a somewhat standard implementation for (un)packing bucket supp 
/// info. The _V_consider_event() and _V_clear() is still has to be implemented.
/// @ingroup buckets
template<typename T>
class ITBucketSuppInfoCollector : public iAbstractInfoCollector {
public:
    typedef T CollectingType;
private:
    T * _miPtr;
protected:
    /// Packs internal message of specific type into protobyf's Any field
    /// referred by given ptr.
    virtual void _V_pack_suppinfo(
                        ::google::protobuf::Any* miMsgRef,
                        const iBundlingDispatcher & ) override {
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
    /// Ctr. The pointer of reentrant supp info instance is supposed to be
    /// allocated using arena.
    ITBucketSuppInfoCollector( T * miPtr ) : _miPtr(miPtr) {}
    const T & own_supp_info_ref() const { return *_miPtr; }
};

/**@class GenericCollector
 * @brief Performs accumulation of events number stored in a bucket, computes
 *        the size of serialized bucket and unique SHA256 hash of bucket content.
 *
 * A generic supp info collector. One may wish to insert this collector
 * instance to bucket dispatcher chains to provide basic buckets
 * identifiaction and integrity checks using SHA256 digests.
 *
 * @ingroup buckets
 */
class GenericCollector :
            public ITBucketSuppInfoCollector<events::CommonBucketDescriptor> {
public:
    typedef ITBucketSuppInfoCollector<events::CommonBucketDescriptor> Parent;
private:
    uint8_t _hash[SHA256_DIGEST_LENGTH];
    size_t _rawDataSize,
           _nEvents;
    size_t _maxEvents,
           _maxSizeInBytes
           ;
protected:
    /// Increases events counter.
    virtual bool _V_consider_event( const events::Event &,
                                    const sV::buckets::iBundlingDispatcher& ) override;
    /// Clears hash function and events counter.
    virtual void _V_clear() override;
    /// Computes hash.
    virtual void _V_pack_suppinfo(
                ::google::protobuf::Any* miMsgRef,
                const iBundlingDispatcher & ) override;
public:
    GenericCollector( events::CommonBucketDescriptor *, size_t nMaxEvents, size_t nMaxBytes );
    GenericCollector( const goo::dict::Dictionary & );
    /// Returns internal counter of raw data length in bytes.
    size_t raw_data_size() const { return _rawDataSize; }
    /// Returns internal counter of number of events in bytes.
    size_t number_of_events() const { return _nEvents; }
    /// Returns maximum number of events.
    size_t maximum_events() const { return _maxEvents; }
    /// Sets maximum number of events.
    void maximum_events( size_t n ) { _maxEvents = n; }
    /// Returns maximum length of raw data (in bytes).
    size_t maximum_raw_data_size() const { return _maxSizeInBytes; }
    /// Sets maximum length of raw data (in bytes).
    void maximum_raw_data_size( size_t n ) { _maxSizeInBytes = n; }
    /// Returns hash. Note: available ONLY inbetween of pack_suppinfo() and
    /// clear() invokations. Always has length of SHA256_DIGEST_LENGTH bytes.
    const uint8_t * hash() const { return _hash; }
};  // class GenericCollector


/**@class iBundlingDispatcher
 * @brief A buket dispatcher appending supplementary information.
 *
 * This class provides methods and supplementary structure fot packing the
 * additional information into the bucket `info` field. The information
 * collectors have to inherit the iAbstractInfoCollector interface.
 *
 * @ingroup buckets
 * */
class iBundlingDispatcher : public iDispatcher {
public:
    typedef std::unordered_map<std::string, iAbstractInfoCollector *>
        CollectorsMap;
private:
    /// Whether to append supp information to dropped bucket
    bool _doPackSuppInfo;
    /// Pointer to message instance.
    events::BucketInfo * _biEntriesPtr;
protected:
    /// Polls all the associated collectors for "is full" condition and returns
    /// true if any of them triggers.
    virtual bool _V_is_full() const override;
    /// Supp info collectors associated with this dispatcher instance
    CollectorsMap _miCollectors;
    /// Returns mutable container of associated collectors
    CollectorsMap & suppinfo_collectors();
    /// Will append supp info from all associated collectors.
    void _append_suppinfo();
    /// Returns mutable reference to supplementary info.
    virtual events::BucketInfo & supp_info();
public:
    iBundlingDispatcher( events::BucketInfo *, bool doPackSuppInfo=true );
    /// (overriden) Additionally, performs invokations of associated collectors.
    virtual void push_event(const events::Event & ) override;
    /// (overriden) Additionally, appends supp info to bucket bundle.
    virtual size_t drop_bucket() override;
    /// Whether to append supplementary information to dropped bucket
    virtual bool do_pack_suppinfo() const { return !! _doPackSuppInfo; }
    /// Sets the supplementary information flag
    virtual void do_pack_suppinfo( bool v ) { _doPackSuppInfo = v; }
    /// Returns immutable reference to supplementary info.
    const events::BucketInfo & supp_info() const;
    /// Returns true if there are supp info collectors associated with an
    /// instance
    bool are_suppinfo_collectors_set() const { return !suppinfo_collectors().empty(); }
    /// Associates the collectors with an instance
    void suppinfo_collectors( CollectorsMap & micPtr );
    /// Returns supp info collectors index associated with this instance
    const CollectorsMap & suppinfo_collectors() const;
    /// Inserts a new supp info collector.
    void add_collector( const std::string &, iAbstractInfoCollector & );
};  // class iBundlingDispatcher

}  // namespace ::sV::buckets
}  // namespace sV

/// Shortcut for define virtual ctr for bucket supp info collector without
/// common config mapping.
# define StromaV_BUCKET_INFO_COLLECTOR_DEFINE( cxxClassName,                \
                                               name )                       \
StromaV_DEFINE_STD_CONSTRUCTABLE( cxxClassName, name, sV::buckets::iAbstractInfoCollector )

/// Shortcut for define virtual ctr for bucket supp info collector with
/// common config mapping.
# define StromaV_BUCKET_INFO_COLLECTOR_DEFINE_MCONF( cxxClassName,          \
                                                  name )                    \
StromaV_DEFINE_STD_CONSTRUCTABLE_MCONF( cxxClassName, name, sV::buckets::iAbstractInfoCollector )

# endif  // RPC_PROTOCOLS

# endif  // H_STROMA_V_IBUCKET_BUNDLING_DISPATCHER_H

