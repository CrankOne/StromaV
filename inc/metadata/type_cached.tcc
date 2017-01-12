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

# ifndef H_STROMA_V_METADATA_TYPE_CACHED_H
# define H_STROMA_V_METADATA_TYPE_CACHED_H

# include "traits.tcc"
# include "dictionary.tcc"
# include "analysis/pipeline.hpp"
# include "analysis/evSource_bulk.tcc"
# include "store.tcc"
# include "batch_handle.tcc"

namespace sV {

namespace aux {
template<typename EventIDT,
         typename MetadataT,
         typename SourceIDT>
class BatchEventsHandle;
}  // namespace aux

/**@class iTCachedMetadataType
 * @brief Interfacing template for storable metadata that can be cached or
 *        retrieved from specific IMetadataStore instances.
 *
 * This class implements common (re)caching logic for physical data source(s)
 * keeping metadata and events. This is a top-level interface, mostly
 * interesting for practiacal usage.
 */
template<typename EventIDT,
         typename MetadataT,
         typename SourceIDT>
class iTCachedMetadataType : public sV::iTMetadataType<EventIDT,
                                                      MetadataT> {
public:
    sV_METADATA_IMPORT_SECT_TRAITS( EventIDT, MetadataT, SourceIDT );

    typedef iTMetadataType<EventID, Metadata> ImmanentParent;
    typedef iBulkEventSource<EventID, Metadata> BaseDataSource;
private:
    std::list<iMetadataStore *> _mdStores;
    std::list<typename Traits::iDisposableSourceManager *> _dspSrcMngrs;
    std::list<typename Traits::iEventQueryableStore *> _singleEventQueryables;
    std::list<typename Traits::iRangeQueryableStore *> _rangeQueryables;
    std::list<typename Traits::iSetQueryableStore *> _setQueryables;

    /// Handle for querying events. May change its state after being retreived.
    mutable aux::BatchEventsHandle<EventID, Metadata, SourceID> _batchHandle;

    /// Aux method adding particular store instance into appropriate list.
    void _put_store( iMetadataStore * basePtr ) {
        _mdStores.push_back( basePtr );
        # define M_sV_store_put( tname, lname ) {                           \
            auto ptr = dynamic_cast<typename Traits:: tname *>(basePtr);    \
            if( ptr ) { lname.push_back(ptr); } }
        M_sV_store_put( iDisposableSourceManager, _dspSrcMngrs )
        M_sV_store_put( iEventQueryableStore, _singleEventQueryables )
        M_sV_store_put( iRangeQueryableStore, _rangeQueryables )
        M_sV_store_put( iSetQueryableStore, _setQueryables )
        # undef M_sV_store_put
    }

    /// Aux method removing particular store instance into appropriate list.
    void _remove_store( iMetadataStore * basePtr ) {
        # define M_sV_store_put( tname, lname ) {                           \
            auto ptr = dynamic_cast<typename Traits:: tname *>(basePtr);    \
            if( ptr ) { lname.remove(ptr); } }
        M_sV_store_put( iDisposableSourceManager, _dspSrcMngrs )
        M_sV_store_put( iEventQueryableStore, _singleEventQueryables )
        M_sV_store_put( iRangeQueryableStore, _rangeQueryables )
        M_sV_store_put( iSetQueryableStore, _setQueryables )
        # undef M_sV_store_put
        _mdStores.remove( basePtr );
    }
protected:
    /// Tries to find metadata instance for given source ID.
    virtual Metadata * _look_up_for( const SourceID & ) const;

    /// Overrides unidentifiable source access with dynamic cast. Note, that
    /// this call can affect performance or cause emerging of std::bad_cast
    /// exception and is not using in sV API for iTCachedMetadataType.
    virtual Metadata & _V_acquire_metadata( BaseDataSource & ds ) final;

    /// (IF) Returns true if metadata information is full.
    virtual bool _V_is_complete( const Metadata & ) const = 0;


    /// (IF) Has to obtain metadata instance from given source w.r.t. to
    /// provided source and its ID and write pointer to newly allocated
    /// metadata instance. May return false if it wasn't possible. The ptr
    /// to metadata pointer is NULL at invokation. Adding to stores has
    /// to be implemented inside this method.
    virtual bool _V_extract_metadata(
                                const SourceID *,
                                iEventSource &,
                                Metadata *&,
                                std::list<iMetadataStore *> stores) const = 0;

    /// (IF) Merges metadata information from multiple instances into one.
    /// Resulting instance is supposed to exist only at rhe run time and not
    /// being cached.
    /// Lifetime of new instance has be controlled by this class or descendant.
    virtual Metadata * _V_merge_metadata(
                            const std::list<Metadata *> & ) const = 0;
    
    /// (IF) If metadata instance is not complete, this abstract method has
    /// to provide its appending to become complete. This feature helps user
    /// code to keep maintain deprecated metadata instances stored somewhere
    /// when its type is extended. Basically this method provides backward
    /// compatibility between different metadata types.
    virtual bool _V_append_metadata( iEventSource & s,
                                     Metadata & md ) const = 0;

    /// (IF) This method has to implement saving of the metadata at specific
    /// storaging instance(s). It may to operate with one particular store or
    /// distribute metadata parts among available stores provided at third
    /// argument. Metadata may be associated with source by its ID
    /// with store's `put_metadata()` method.
    virtual void _V_cache_metadata( const SourceID &,
                                    const Metadata &,
                                    std::list<iMetadataStore *> & ) const = 0;

    /// (IF) Method has to fill the sub-range mark-up structure with
    /// range interval corresponding to the particular fragemnt. Used for
    /// building range-iteration query results. True has to be returned if
    /// range no further appending of `muRef` argument can be performed.
    virtual void _V_get_subrange(
                    const EventID & low, const EventID & up,
                    const SourceID & sid,
                    typename Traits::SubrangeMarkup & muRef ) const = 0;
public:
    /// Default ctr (no store associated).
    iTCachedMetadataType( const std::string & tnm ) :
                                            ImmanentParent(tnm),
                                            _batchHandle(*this) {}

    /// Ctr immediately associating type with store.
    iTCachedMetadataType(iMetadataStore & store) { _mdStores.push_back(&store); }

    /// Dtr.
    virtual ~iTCachedMetadataType() {}

    /// Associates store with type.
    void add_store( iMetadataStore & store ) { _put_store(&store); }

    /// Removes association between store and type.
    void remove_store( iMetadataStore & store ) { _remove_store(&store); }

    /// Whether the provided metadata instance is complete?
    bool is_complete( const Metadata & md ) const
        { return _V_is_complete( md ); }

    /// Performs extraction of metadata from source w.r.t. to provided source
    /// ID and writes pointer to newly allocated metadata instance.
    bool extract_metadata( const SourceID * sidPtr,
                           iEventSource & s,
                           Metadata *& mdPtrRef ) const
        { return _V_extract_metadata( sidPtr, s, mdPtrRef, _mdStores ); }

    /// Merges metadata information from multiple instances into one.
    Metadata * merge_metadata(
                            const std::list<Metadata *> & mds ) const {
        return _V_merge_metadata( mds );
    }

    /// Puts store(s) (or appends with) extracted metadata.
    bool append_metadata( iEventSource & s,
                          Metadata & md ) const {
        return _V_append_metadata( s, md );
    }

    /// Obtaines or (re)caches metadata for given source performing interaction
    /// with associated metadata store instance(s).
    virtual Metadata & acquire_metadata_for( iEventSource & s );

    /// Returns interim object incapsulating acquizition events from specific
    /// source instances.
    virtual aux::BatchEventsHandle<EventID, Metadata, SourceID> &
                                batch_handle() const { return _batchHandle; }

    /// Sets range-read mark-up entry with ranges available for source with a
    /// given ID. May also set the source/metadata ptrs.
    virtual void get_subrange( const EventID & low, const EventID & up,
                               const SourceID & sid,
                               typename Traits::SubrangeMarkup & muRef ) const
        { _V_get_subrange( low, up, sid, muRef ); }

    friend class aux::BatchEventsHandle<EventIDT, MetadataT, SourceIDT>;
};  // iTCachedMetadataType


template<typename EventIDT,
         typename MetadataT,
         typename SourceIDT> MetadataT *
iTCachedMetadataType<EventIDT,
                    MetadataT,
                    SourceIDT>::_look_up_for( const SourceID & sid ) const {
    if( _mdStores.empty() ) {
        sV_logw( "No stores associated with type \"%s\" (id:%#0x, ptr:%p) "
                 "while tried to retrieve metadata info for source.\n",
                this->name().c_str(),
                (size_t) this->get_index(), this );
        return nullptr;
    }
    std::list<Metadata *> results;
    for( const auto & it : _mdStores ) {
        Metadata * result = it->get_metadata_for( sid );
        if( result ) {
            results.push_back( result );
        }
    }
    if( results.empty() ) {
        return nullptr;
    } else if( results.size() > 1 ) {
        return merge_metadata( results );
    }
    return *results.begin();
}

/**Optionally performs synchronization against particular metadata store.
 * This method tries to lookup the specific metadata corresponding to
 * given data source at the associated storage and obtain reference to
 * valid instance, if possible. If look-up is not succeeded the method has to
 * construct new instance (and append the associated storage, if has).
 * */
template<typename EventIDT,
         typename MetadataT,
         typename SourceIDT> MetadataT &
iTCachedMetadataType<EventIDT, MetadataT, SourceIDT>::acquire_metadata_for(
            iEventSource & s ) {
    const SourceID * sidPtr = s.id_ptr();
    Metadata * metadataPtr = nullptr;
    if( ! _mdStores.empty() ) {
        if( sidPtr ) {
            metadataPtr = _look_up_for( *sidPtr );
        } else {
            sV_logw( "Can not permanently store/retrieve "
                "metadata information for source \"%s\" (%p) since it has no "
                "ID set.\n", s.textual_id().c_str(), &s );
        }
    } else {
        // Has no stores associated.
        sV_logw( "Unable to permanently store/retrieve metadata "
                 "information for source \"%s\" (%p) since metadata type "
                 "\"%s\" (id:%#x, ptr:%p) has no associated reentrant "
                 "indexes storage.\n",
                 s.textual_id().c_str(), &s,
                 this->name().c_str(), this->get_index(), this );
    }

    if( !metadataPtr ) {
        sV_log2( "Extracting metadata information for source \"%s\" (%p).\n",
                 s.textual_id().c_str(), &s );
        if( !extract_metadata( sidPtr, s, metadataPtr ) ) {
            emraise( thirdParty, "Unable to extract metadata from "
                     "source \"%s\" (%p).", s.textual_id().c_str(), &s );
        }
        sV_log2( "Metadata extracted for source %p.\n", &s );
        if( sidPtr && !_mdStores.empty() ) {
            _V_cache_metadata( *sidPtr, *metadataPtr, _mdStores );  //< TODO?
        } else {
            sV_logw( "Metadata for source \"%s\" (%p) can not be stored since "
                "either the ID for source is not set, or no reentrant indexes "
                "storages were associated with type \"%s\" (id:%#x, ptr:%p).\n",
                s.textual_id().c_str(), &s, this->name().c_str(),
                (int) this->type_index(), this );
        }
    }
    if( !is_complete( *metadataPtr ) ) {
        append_metadata( s, *metadataPtr );
    }
    return *metadataPtr;
}

template<typename EventIDT,
         typename MetadataT,
         typename SourceIDT>
MetadataT &
iTCachedMetadataType<EventIDT, MetadataT, SourceIDT>::_V_acquire_metadata(
                            BaseDataSource & ds_ ) {
    sV_log3( "Inetrface method acquire_metadata() called for "
             "iTCachedMetadataType<...> type. Can affect performance.\n" );
    iEventSource & ds = dynamic_cast<iEventSource &>(ds_);
    return acquire_metadata_for( ds );
}

}  // namespace sV

# endif  // H_STROMA_V_METADATA_TYPE_CACHED_H

