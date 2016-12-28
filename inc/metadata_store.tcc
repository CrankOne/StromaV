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
# include "analysis/identifiable_ev_source.tcc"

namespace sV {

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
    typedef iMetadataType<EventID, SpecificMetadata> SpecificMetadataType;
public:
    /// Returns nullptr if source with such ID not found in store.
    SpecificMetadata * get_metadata_for( const SourceID & );
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
    typedef aux::iEventSource<EventID, SpecificMetadata, SourceIDT> DataSource;
    typedef iMetadataStore<EventID, SpecificMetadata, SourceID> MetadataStore;
private:
    std::list<MetadataStore *> _mdStores;
protected:
    /// Tries to find metadata instance for given source ID.
    virtual SpecificMetadata * _look_up_for( const SourceID & );

    /// Obtaines or (re)caches metadata for given source performing interaction
    /// with associated metadata store instance(s).
    virtual SpecificMetadata & _V_acquire_metadata( DataSource & ) const override;

    /// (IF) Returns true if metadata information can be appended.
    virtual bool _V_is_complete( const SpecificMetadata & ) const = 0;


    /// (IF) Has to obtain metadata instance from given source w.r.t. to
    /// provided source and its ID and writes pointer to newly allocated
    /// metadata instance. May return false if it wasn't possible. The ptr
    /// to metadata may be or may be not NULL at invokation but has to be
    /// non-null after.
    virtual bool _V_extract_metadata( const SourceID *,
                              DataSource &,
                              SpecificMetadata *& ) const = 0;

    /// (IF) Merges metadata information from multiple instances into one.
    /// Lifetime of new instance has be controlled by descendant or store.
    virtual SpecificMetadata * _V_merge_metadata(
                                const std::list<SpecificMetadata *> & ) = 0;
    
    /// (IF) Has to append metadata stores with extracted metadata info.
    virtual bool _V_append_metadata( const SourceID * sidPtr,
                           DataSource & s,
                           SpecificMetadata & md ) = 0;
public:
    /// Default ctr (no store associated).
    iCachedMetadataType() {}

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
    bool extract_metadata( const SourceID *,
                           DataSource &,
                           SpecificMetadata *& ) const;

    /// Merges metadata information from multiple instances into one.
    SpecificMetadata * merge_metadata(
                                const std::list<SpecificMetadata *> & mds ) {
        return _V_merge_metadata( mds );
    }

    /// Puts store(s) (or appends with) extracted metadata.
    bool append_metadata( const SourceID * sidPtr,
                           DataSource & s,
                           SpecificMetadata & md ) {
        return _V_append_metadata( sidPtr, s, md );
    }
};  // iCachedMetadataType


template<typename EventIDT,
         typename SpecificMetadataT,
         typename SourceIDT> SpecificMetadataT *
iCachedMetadataType<EventIDT,
                    SpecificMetadataT,
                    SourceIDT>::_look_up_for( const SourceID & sid ) {
    std::list<SpecificMetadata *> results;
    for( const auto & it : _mdStores ) {
        SpecificMetadata * result = it.get_metadata_for( sid );
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
    const SourceID * sidPtr = s.id_ptr();
    // ^^^ TODO: how to guarantee? Note, that it can return NULL
    SpecificMetadata * metadataPtr;
    if( ! _mdStores.empty() ) {
        if( sidPtr ) {
            metadataPtr = _look_up_for( *sidPtr );
        } else {
            sV_logw( badParameter, "Can not permanently store/retrieve "
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

    if( !metadataPtr || !is_complete( metadataPtr ) ) {
        // Look-up failed, we have to initiate extraction metadata from this
        // source and append storage with acquired data:
        sV_log2( "Extracting metadata information for source \"%s\" (%p).\n",
            s.textual_id(), &s );
        if( !extract_metadata( sidPtr, s, &metadataPtr ) ) {
            emraise( thirdParty, "Unable to extract metadata from "
                "source %s (%p).", s.textual_id(sidPtr), &s );
        }
        sV_log2( "Metadata extracted for source %p.\n", &s );
        append_metadata( metadataPtr, chunk.run_no(), chunk.chunk_no() );
    }
    return *tIndexPtr;
}

}  // namespace sV

# endif  // H_STROMA_V_METADATA_STORE_H

