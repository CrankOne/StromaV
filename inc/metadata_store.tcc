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

# include "metadata.hpp"

namespace sV {

template<typename EventIDT,
         typename SpecificMetadataT,
         typename SourceIDT>
class iSectionalEventSource;

template<typename EventIDT,
         typename SpecificMetadataT>
class iBatchEventSource;

/**@class iMetadataStore
 * @brief Interface to entities storing metadata of specific type.
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
protected:
    virtual SpecificMetadata * _V_get_metadata_for(const SourceID &) const = 0;
public:
    iMetadataStore() {}
    virtual ~iMetadataStore() {}

    /// Returns nullptr if source with such ID not found in store.
    virtual SpecificMetadata * get_metadata_for( const SourceID & sid ) const
        { return _V_get_metadata_for( sid ); }
};  // class iMetadataStore

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
    typedef iBatchEventSource<EventID, SpecificMetadata> BaseDataSource;
    typedef iMetadataStore<EventID, SpecificMetadata, SourceID> MetadataStore;
private:
    std::list<MetadataStore *> _mdStores;
protected:
    /// Tries to find metadata instance for given source ID.
    virtual SpecificMetadata * _look_up_for( const SourceID & ) const;

    /// Overrides unidentifiable source access with dynamic cast.
    virtual SpecificMetadata & _V_acquire_metadata(
                                BaseDataSource & ds ) const final;

    /// (IF) Returns true if metadata information is full.
    virtual bool _V_is_complete( const SpecificMetadata & ) const = 0;


    /// (IF) Has to obtain metadata instance from given source w.r.t. to
    /// provided source and its ID and writes pointer to newly allocated
    /// metadata instance. May return false if it wasn't possible. The ptr
    /// to metadata may be or may be not NULL at invokation but has to be
    /// non-null after. The lifetime of the obtained metadata instance is
    /// further maintained by this class instance or by descendants.
    virtual bool _V_extract_metadata( const SourceID *,
                              DataSource &,
                              SpecificMetadata *& ) const = 0;

    /// (IF) Merges metadata information from multiple instances into one.
    /// Lifetime of new instance has be controlled by this class or descendant.
    virtual SpecificMetadata * _V_merge_metadata(
                            const std::list<SpecificMetadata *> & ) const = 0;
    
    /// (IF) Has to append metadata stores with extracted metadata info.
    virtual bool _V_append_metadata( DataSource & s,
                                     SpecificMetadata & md ) const = 0;
public:
    /// Default ctr (no store associated).
    iCachedMetadataType( const std::string & tnm ) : ImmanentParent(tnm) {}

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
        { return _V_extract_metadata( sidPtr, s, mdPtrRef ); }

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
};  // iCachedMetadataType


template<typename EventIDT,
         typename SpecificMetadataT,
         typename SourceIDT> SpecificMetadataT *
iCachedMetadataType<EventIDT,
                    SpecificMetadataT,
                    SourceIDT>::_look_up_for( const SourceID & sid ) const {
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
 * valid instance, if possible. If it is not, it has to construct new
 * instance (and append the associated storage, if presented).
 * */
template<typename EventIDT,
         typename SpecificMetadataT,
         typename SourceIDT> SpecificMetadataT &
iCachedMetadataType<EventIDT, SpecificMetadataT, SourceIDT>::acquire_metadata_for(
            DataSource & s ) const {
    //DataSource & s = dynamic_cast<DataSource>( sb );
    const SourceID * sidPtr = s.id_ptr();
    // ^^^ TODO: how to guarantee? Note, that it can return NULL
    SpecificMetadata * metadataPtr;
    if( ! _mdStores.empty() ) {
        if( sidPtr ) {
            metadataPtr = _look_up_for( *sidPtr );
        } else {
            sV_logw( "Can not permanently store/retrieve "
                "metadata information for source %s (%p) since it has no "
                "ID set.\n", s.textual_id(), &s );
        }
    } else {
        // Has no stores associated.
        sV_logw( "Unable to permanently store/retrieve metadata "
                 "information for source %s (%p) since metadata type %p "
                 "has no associated reentrant indexes storage.\n",
                 s.textual_id(), &s, this );
    }

    if( !metadataPtr || !is_complete( *metadataPtr ) ) {
        // Look-up failed or metadata is not complete, we have to initiate
        // extraction metadata from this source and append storage with
        // acquired data:
        sV_log2( "Extracting metadata information for source \"%s\" (%p).\n",
            s.textual_id(), &s );
        if( !extract_metadata( sidPtr, s, metadataPtr ) ) {
            emraise( thirdParty, "Unable to extract metadata from "
                "source %s (%p).", s.textual_id(), &s );
        }
        sV_log2( "Metadata extracted for source %p.\n", &s );
        append_metadata( s, *metadataPtr );
    }
    return *metadataPtr;
}

template<typename EventIDT,
         typename SpecificMetadataT,
         typename SourceIDT>
SpecificMetadataT &
iCachedMetadataType<EventIDT, SpecificMetadataT, SourceIDT>::_V_acquire_metadata(
                            BaseDataSource & ds ) const {
    // TODO: sV_logd()?
    //sV_logw( "Inetrface method acquire_metadata() called for "
    //    "iCachedMetadataType<...> type. Can affect performance.\n" );
    //return acquire_metadata_for( dynamic_cast<DataSource &>(ds) );
    _FORBIDDEN_CALL_
}

}  // namespace sV

# endif  // H_STROMA_V_METADATA_STORE_H

