/*
 * Copyright (c) 2016 Renat R. Dusaev <crank@qcrypt.org>
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

# ifndef H_STROMA_V_METADATA_STORE_H
# define H_STROMA_V_METADATA_STORE_H

# include "dictionary.tcc"
# include "analysis/pipeline.hpp"
# include "analysis/evSource_bulk.tcc"

namespace sV {

template<typename EventIDT,
         typename SpecificMetadataT,
         typename SourceIDT>
class iSectionalEventSource;

template<typename EventIDT,
         typename SpecificMetadataT>
class iBulkEventSource;

/**@class iMetadataStore
 * @brief Interface to entities storing metadata of specific type, performing
 *        look-up and querying operations for events range and sources.
 * 
 * This abstract class declares logic for retreiving and caching methadata
 * fragments for sectioned (fragmentated) sources whose metadata is partially
 * extracted from sectional source.
 *
 * The actual implementation of its methods may imply in-memory collections
 * (like hashmaps) as well as database handle. One important case is the
 * store keeping pointers to currently opened files.
 *
 * Note: may be "store" is not a best choice for naming such entities but I had
 * to higlight that it is not a collection but rather a deposition place in a
 * wide sense.
 **/
template<typename EventIDT,
         typename SpecificMetadataT,
         typename SourceIDT>
class iMetadataStore {
public:
    typedef EventIDT EventID;
    typedef SpecificMetadataT SpecificMetadata;
    typedef SourceIDT SourceID;
    typedef iMetadataType<EventID, SpecificMetadata> SpecificMetadataType;
    typedef iSectionalEventSource<EventID, SpecificMetadata, SourceID>
            iSpecificSectionalEventSource;
protected:
    /// (IF) Returns pointer to metadata instance corresponding to specific
    /// sectional source.
    virtual SpecificMetadata * _V_get_metadata_for(const SourceID &) const = 0;

    /// (IF) Has to save metadata associated with provided source ID.
    virtual void _V_put_metadata( const SourceID &,
                                  const SpecificMetadata & ) = 0;

    /// (IF) Has to return an instance of sectional data source
    /// corresponding to particular source ID or return null pointer if store
    /// instance does not support such function.
    /// (todo: move to another class?)
    virtual iSpecificSectionalEventSource * 
                                _V_source( const SourceID & ) = 0;

    /// (IF) Has to free previously acquired event source by ptr, if needed.
    /// (todo: move to another class?)
    virtual void _V_free_source( iSpecificSectionalEventSource * srcPtr ) = 0;

    /// (IF) Has to return an identifier of sectional source containing the
    /// event with specific ID.
    virtual bool _V_source_id_for( const EventID &, SourceID & ) const = 0;

    /// (IF) Has to perform filling of source identifiers list corresponding
    /// to specific range of event identifiers.
    virtual void _V_collect_source_ids_for_range(
                                const EventID & from,
                                const EventID & to,
                                std::list<SourceID> & ) const = 0;

    /// (IF) Has to perform filling of source identifiers list corresponding
    /// to list of specific event identifiers.
    virtual void _V_collect_source_ids_for_set(
                                const std::list<EventID> &,
                                std::list<SourceID> & ) const = 0;
public:
    iMetadataStore() {}
    virtual ~iMetadataStore() {}

    /// Returns nullptr if source with such ID not found in store.
    virtual SpecificMetadata * get_metadata_for( const SourceID & sid ) const
        { return _V_get_metadata_for( sid ); }

    /// Saves metadata associated with given source ID.
    virtual void put_metadata( const SourceID & sid,
                               const SpecificMetadata & mdRef )
        { _V_put_metadata( sid, mdRef ); }

    /// Constructs an instance of sectional data source corresponding to
    /// particular source ID.
    ///
    ///@see iMetadataStore::free_source()
    virtual iSpecificSectionalEventSource *
                                source( const SourceID & sid ) {
        return _V_source(sid); }

    /// Frees previously constructed event source.
    ///
    ///@see iMetadataStore::source()
    virtual void free_source( iSpecificSectionalEventSource * srcPtr ) {
        return _V_free_source( srcPtr );}

    /// Returns source identifier for specific event.
    virtual bool source_id_for( const EventID & eid, SourceID & sid ) const {
        return _V_source_id_for( eid, sid );
    }

    /// Fills the list of source identifiers corresponding to specific range
    /// of event identifiers.
    virtual void collect_source_ids_for_range(
                                const EventID & from,
                                const EventID & to,
                                std::list<SourceID> & l ) const {
        _V_collect_source_ids_for_range( from, to, l ); }

    /// Fills list of source identifiers corresponding to given list of
    /// specific event identifiers.
    virtual void collect_source_ids_for_set(
                                const std::list<EventID> & evl,
                                std::list<SourceID> & sl ) const {
        _V_collect_source_ids_for_set( evl, sl ); }
};  // class iMetadataStore

namespace aux {
template<typename EventIDT,
         typename SpecificMetadataT,
         typename SourceIDT>
class BatchEventsHandle;
}  // namespace aux

/**@class iCachedMetadataType
 * @brief Interfacing template for storable metadata that can be cached or
 *        retrieved from specific iMetadataStore instances.
 *
 * This class implements common (re)caching logic for physical data source(s)
 * keeping metadata and events. This is a top-level interface, mostly
 * interesting for practiacal usage.
 */
template<typename EventIDT,
         typename SpecificMetadataT,
         typename SourceIDT>
class iCachedMetadataType : public sV::iMetadataType<EventIDT,
                                                     SpecificMetadataT> {
public:
    typedef EventIDT EventID;
    typedef SpecificMetadataT SpecificMetadata;
    typedef SourceIDT SourceID;
    typedef iMetadataType<EventID, SpecificMetadata> ImmanentParent;
    typedef iSectionalEventSource<EventID, SpecificMetadata, SourceIDT> DataSource;
    typedef iBulkEventSource<EventID, SpecificMetadata> BaseDataSource;
    typedef iMetadataStore<EventID, SpecificMetadata, SourceID> MetadataStore;
private:
    mutable std::list<MetadataStore *> _mdStores;
    mutable aux::BatchEventsHandle<EventID,
                                    SpecificMetadata, SourceID> _batchHandle;
protected:
    /// Tries to find metadata instance for given source ID.
    virtual SpecificMetadata * _look_up_for( const SourceID & ) const;

    /// Overrides unidentifiable source access with dynamic cast. Note, that
    /// this call can affect performance or cause emerging of std::bad_cast
    /// exception and is not using in sV API for iCachedMetadataType.
    virtual SpecificMetadata & _V_acquire_metadata(
                                BaseDataSource & ds ) const final;

    /// (IF) Returns true if metadata information is full.
    virtual bool _V_is_complete( const SpecificMetadata & ) const = 0;


    /// (IF) Has to obtain metadata instance from given source w.r.t. to
    /// provided source and its ID and write pointer to newly allocated
    /// metadata instance. May return false if it wasn't possible. The ptr
    /// to metadata pointer is NULL at invokation. Adding to stores has
    /// to be implemented inside this method.
    virtual bool _V_extract_metadata(
                                const SourceID *,
                                DataSource &,
                                SpecificMetadata *&,
                                std::list<MetadataStore *> stores) const = 0;

    /// (IF) Merges metadata information from multiple instances into one.
    /// Resulting instance is supposed to exist only at rhe run time and not
    /// being cached.
    /// Lifetime of new instance has be controlled by this class or descendant.
    virtual SpecificMetadata * _V_merge_metadata(
                            const std::list<SpecificMetadata *> & ) const = 0;
    
    /// (IF) If metadata instance is not complete, this abstract method has
    /// to provide its appending to become complete. This feature helps user
    /// code to keep maintain deprecated metadata instances stored somewhere
    /// when its type is extended. Basically this method provides backward
    /// compatibility between different metadata types.
    virtual bool _V_append_metadata( DataSource & s,
                                     SpecificMetadata & md ) const = 0;

    /// (IF) This method has to implement saving of the metadata at specific
    /// storaging instance(s). It may to operate with one particular store or
    /// distribute metadata parts among available stores provided at third
    /// argument. Metadata may be associated with source by its ID
    /// with store's `put_metadata()` method.
    virtual void _V_cache_metadata( const SourceID &,
                                    const SpecificMetadata &,
                                    std::list<MetadataStore *> & ) const = 0;
public:
    /// Default ctr (no store associated).
    iCachedMetadataType( const std::string & tnm ) :
                                            ImmanentParent(tnm),
                                            _batchHandle(*this) {}

    /// Ctr immediately associating type with store.
    iCachedMetadataType(MetadataStore & store) { _mdStores.push_back(&store); }

    /// Dtr.
    virtual ~iCachedMetadataType() {}

    /// Associates store with type.
    void add_store( MetadataStore & store ) { _mdStores.push_back(&store); }

    /// Removes association between store and type.
    void remove_store( MetadataStore & store ) { _mdStores.remove(&store); }

    /// Whether the provided metadata instance is complete?
    bool is_complete( const SpecificMetadata & md ) const
        { return _V_is_complete( md ); }

    /// Performs extraction of metadata from source w.r.t. to provided source
    /// ID and writes pointer to newly allocated metadata instance.
    bool extract_metadata( const SourceID * sidPtr,
                           DataSource & s,
                           SpecificMetadata *& mdPtrRef ) const
        { return _V_extract_metadata( sidPtr, s, mdPtrRef, _mdStores ); }

    /// Merges metadata information from multiple instances into one.
    SpecificMetadata * merge_metadata(
                            const std::list<SpecificMetadata *> & mds ) const {
        return _V_merge_metadata( mds );
    }

    /// Puts store(s) (or appends with) extracted metadata.
    bool append_metadata( DataSource & s,
                          SpecificMetadata & md ) const {
        return _V_append_metadata( s, md );
    }

    /// Obtaines or (re)caches metadata for given source performing interaction
    /// with associated metadata store instance(s).
    virtual SpecificMetadataT & acquire_metadata_for( DataSource & s ) const;

    /// Returns interim object incapsulating acquizition events from specific
    /// source instances.
    virtual aux::BatchEventsHandle<EventID, SpecificMetadata, SourceID> &
    batch_handle() const { return _batchHandle; }

    std::list<MetadataStore *> & stores() const { return _mdStores; }
};  // iCachedMetadataType


template<typename EventIDT,
         typename SpecificMetadataT,
         typename SourceIDT> SpecificMetadataT *
iCachedMetadataType<EventIDT,
                    SpecificMetadataT,
                    SourceIDT>::_look_up_for( const SourceID & sid ) const {
    if( _mdStores.empty() ) {
        sV_logw( "No stores associated with type \"%s\" (id:%#0x, ptr:%p) "
                 "while tried to retrieve metadata info for source.\n",
                this->name().c_str(),
                (size_t) this->get_index(), this );
        return nullptr;
    }
    std::list<SpecificMetadata *> results;
    for( const auto & it : _mdStores ) {
        SpecificMetadata * result = it->get_metadata_for( sid );
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
         typename SpecificMetadataT,
         typename SourceIDT> SpecificMetadataT &
iCachedMetadataType<EventIDT, SpecificMetadataT, SourceIDT>::acquire_metadata_for(
            DataSource & s ) const {
    const SourceID * sidPtr = s.id_ptr();
    SpecificMetadata * metadataPtr = nullptr;
    if( ! _mdStores.empty() ) {
        if( sidPtr ) {
            metadataPtr = _look_up_for( *sidPtr );
        } else {
            sV_logw( "Can not permanently store/retrieve "
                "metadata information for source \"%s\" (%p) since it has no "
                "ID set.\n", s.textual_id(), &s );
        }
    } else {
        // Has no stores associated.
        sV_logw( "Unable to permanently store/retrieve metadata "
                 "information for source \"%s\" (%p) since metadata type "
                 "\"%s\" (id:%#x, ptr:%p) has no associated reentrant "
                 "indexes storage.\n",
                 s.textual_id(), &s, this );
    }

    if( !metadataPtr ) {
        sV_log2( "Extracting metadata information for source \"%s\" (%p).\n",
                 s.textual_id(), &s );
        if( !extract_metadata( sidPtr, s, metadataPtr ) ) {
            emraise( thirdParty, "Unable to extract metadata from "
                     "source \"%s\" (%p).", s.textual_id(), &s );
        }
        sV_log2( "Metadata extracted for source %p.\n", &s );
        if( sidPtr && !_mdStores.empty() ) {
            _V_cache_metadata( *sidPtr, *metadataPtr, stores() );
        } else {
            sV_logw( "Metadata for source \"%s\" (%p) can not be stored since "
                "either the ID for source is not set, or no reentrant indexes "
                "storages were associated with type \"%s\" (id:%#x, ptr:%p).\n",
                s.textual_id(), &s, this->name().c_str(),
                (int) this->type_index(), this );
        }
    }
    if( !is_complete( *metadataPtr ) ) {
        append_metadata( s, *metadataPtr );
    }
    return *metadataPtr;
}

template<typename EventIDT,
         typename SpecificMetadataT,
         typename SourceIDT>
SpecificMetadataT &
iCachedMetadataType<EventIDT, SpecificMetadataT, SourceIDT>::_V_acquire_metadata(
                            BaseDataSource & ds_ ) const {
    sV_log3( "Inetrface method acquire_metadata() called for "
             "iCachedMetadataType<...> type. Can affect performance.\n" );
    DataSource & ds = dynamic_cast<DataSource &>(ds_);
    return acquire_metadata_for( ds );
}


namespace aux {

/**@class BatchEventsHandle
 * @brief Auxilliary class representing a subset all of iSectionalEventSource
 *        instances of particular type.
 * 
 * This interim class is usually instantiated by the internals of
 * iCachedMetadataType in order to provide unified look-up mechanics among
 * available stores by providing a proxy instance referring to metadata type's
 * stores list. User can consider it as a one humongous data source containing
 * all available events physically distributed among multiple artifacts. User
 * probably will never instantiate it directly but rather obtaine a reference
 * to it via the `batch_handle()` method of corresponding metadata type.
 *
 * @see IRandomAccessEventStream
 */
template<typename EventIDT,
         typename SpecificMetadataT,
         typename SourceIDT>
class BatchEventsHandle : public IRandomAccessEventStream<EventIDT> {
public:
    typedef EventIDT EventID;
    typedef SpecificMetadataT SpecificMetadata;
    typedef SourceIDT SourceID;
    typedef sV::events::Event Event;
    typedef iMetadataStore<EventID, SpecificMetadata, SourceID> MetadataStore;
    typedef iCachedMetadataType<EventID, SpecificMetadata, SourceID>
            iSpecificCachedMetadataType;
    typedef iSectionalEventSource<EventID, SpecificMetadata, SourceID>
            iSpecificSectionalEventSource;
private:
    iSpecificCachedMetadataType & _mdt;
    void _assert_mdt_non_empty() {
        if( _mdt.stores().empty() ) {
            emraise( badState, "No stores associated with cached metadata "
                     "type \"%s\" (id:%#x, ptr:%p). Unable to retreive "
                     "cached metadata.", _mdt.name().c_str(),
                     _mdt.type_index(), &_mdt );
        }
    }
    Event _reentrantSingleEvent;
public:
    BatchEventsHandle( iSpecificCachedMetadataType & mdt ) :
                                                    _mdt(mdt) {}
    virtual ~BatchEventsHandle() {}

    /// Note that returned event ptr refers to internal reentrant instance and
    /// will be re-written on next invokation of this method. Lifetime of
    /// this instance is restricted by lifetime of owning iCachedMetadataType
    /// instance.
    virtual Event * event_read_single( const EventID & eid ) override {
        _assert_mdt_non_empty();
        SourceID sid;
        bool found = false;
        for( auto storePtr : _mdt.stores() ) {
            if( (found = storePtr->source_id_for( eid, sid )) ) {
                break;
            }
        }
        if( !found ) {
            emraise( noSuchKey, "Unable to locate source containing event "
                                "with specified ID." );
        }
        iSpecificSectionalEventSource * evSourcePtr = nullptr;
        MetadataStore * owningStore = nullptr;
        for( auto storePtr : _mdt.stores() ) {
            if( nullptr != (evSourcePtr = storePtr->source(sid)) ) {
                owningStore = storePtr;
                break;
            }
        }
        if( !owningStore ) {
            emraise( badState, "Event with specified ID located, but none of "
                "available metadata caching store instances can provide "
                "accessible handle to source." );
        }
        // todo: may optimize it a little by using direct querying of metadata
        // here:
        //Event * eventReadPtr = evSourcePtr->_md_event_read_single( eid );
        Event * eventReadPtr = evSourcePtr->event_read_single( eid );
        _reentrantSingleEvent.CopyFrom( *eventReadPtr );
        owningStore->free_source(evSourcePtr);
        return &_reentrantSingleEvent;
    }

    virtual std::unique_ptr<iEventSequence> event_read_range(
                                const EventID & lower,
                                const EventID & upper ) override {
        _assert_mdt_non_empty();
        _TODO_  // TODO
    }

    virtual std::unique_ptr<iEventSequence> event_read_list(
                                const std::list<EventID> & list ) override {
        _assert_mdt_non_empty();
        _TODO_  // TODO
    }
    
};  // class BatchEventsHandle

}  // namespace aux

}  // namespace sV

# endif  // H_STROMA_V_METADATA_STORE_H

