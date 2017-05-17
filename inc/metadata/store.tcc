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

# include "traits.tcc"

namespace sV {

template<typename EventIDT,
         typename MetadataT,
         typename SourceIDT>
class iSectionalEventSource;

template<typename EventIDT,
         typename MetadataT>
class iBulkEventSource;

namespace aux {

/// Aux struct describing interval of events that can be read from a particular
/// sectioned source. Optionally, can refer to metadata and existing source
/// instance (otherwise, these fields are set to null).
/// @ingroup mdat
template<typename EventIDT,
         typename MetadataT,
         typename SourceIDT>
struct RangeReadingMarkupEntry {
    EventIDT from, to;
    SourceIDT sid;
    MetadataT * mdPtr;
    iSectionalEventSource<EventIDT, MetadataT, SourceIDT> * srcPtr;
};

}  // namespace aux

/** \addtogroup mdat 
 * @{
 */

/**@class ITMetadataStore
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
         typename MetadataT,
         typename SourceIDT>
struct ITMetadataStore {
    sV_METADATA_IMPORT_SECT_TRAITS(EventIDT, MetadataT, SourceIDT);

    ITMetadataStore() {}
    virtual ~ITMetadataStore() {}

    /// (IF) Returns pointer to metadata instance corresponding to specific
    /// sectional source.
    virtual Metadata * get_metadata_for( const SourceID & sid ) const = 0;

    /// (IF) Has to save metadata associated with provided source ID.
    virtual void put_metadata( const SourceID & sid,
                               const Metadata & mdRef ) = 0;

    /// Removes metadata entry from store.
    virtual void erase_metadata_for( const SourceID & sid ) = 0;
};  // class ITMetadataStore


/**@class ITDisposableSourceManager
 * @brief A template interface mixin defining methods for retaining source
 * instances.
 *
 * Adds source instances management methods to basic metadata store interface.
 * Has to be used as a mixin.
 * */
template<typename EventIDT,
         typename MetadataT,
         typename SourceIDT>
struct ITDisposableSourceManager : virtual public ITMetadataStore<EventIDT,
                                                               MetadataT,
                                                               SourceIDT> {
    sV_METADATA_IMPORT_SECT_TRAITS(EventIDT, MetadataT, SourceIDT);

    /// (IF) Has to return an instance of sectional data source
    /// corresponding to particular source ID or return null pointer if store
    /// instance does not support such function.
    /// @see ITMetadataStore::free_source()
    virtual iEventSource * source( const SourceID & ) = 0;

    /// (IF) Has to free previously acquired event source by ptr, if needed.
    /// (todo: move to another class?)
    virtual void free_source( iEventSource * srcPtr ) = 0;
};  // class iSectionalSourceManager


/**@class ITEventQueryableStore
 * @brief A template interface mixin defining methods for querying source
 *        identifier by event ID.
 *
 * This interface declares a pure abstract method for source ID lookup by
 * given event ID.
 * */
template<typename EventIDT,
         typename MetadataT,
         typename SourceIDT>
struct ITEventQueryableStore : virtual public ITMetadataStore<EventIDT,
                                                           MetadataT,
                                                           SourceIDT> {
    sV_METADATA_IMPORT_SECT_TRAITS(EventIDT, MetadataT, SourceIDT);

    /// (IF) Has to return an identifier of sectional source containing the
    /// event with specific ID.
    virtual bool source_id_for( const EventID &, SourceID & ) const = 0;
};


/**@class ITRangeQueryableStore
 * @brief A template interface mixin defining methods for querying set of
 *        source identifiers by event IDs range.
 *
 * This interface declares a pure abstract method for source IDs lookup by
 * given range of event ID.
 * */
template<typename EventIDT,
         typename MetadataT,
         typename SourceIDT>
struct ITRangeQueryableStore : virtual public ITMetadataStore<EventIDT,
                                                           MetadataT,
                                                           SourceIDT>{
    sV_METADATA_IMPORT_SECT_TRAITS(EventIDT, MetadataT, SourceIDT);

    /// (IF) Has to perform filling of source identifiers list corresponding
    /// to specific range of event identifiers.
    virtual void collect_source_ids_for_range(
                                const EventID & from,
                                const EventID & to,
                                std::list<SubrangeMarkup> & ) const = 0;
};  // class iRangeQueryableStore


/**@class ITSetQueryableStore
 * @brief A template interface mixin defining methods for querying set of
 *        source identifiers by list of arbitrary event IDs.
 *
 * This interface declares a pure abstract method for source IDs lookup by
 * given set of event ID (given in random order).
 * */
template<typename EventIDT,
         typename MetadataT,
         typename SourceIDT>
struct ITSetQueryableStore : virtual public ITMetadataStore<EventIDT,
                                                         MetadataT,
                                                         SourceIDT> {
    sV_METADATA_IMPORT_SECT_TRAITS(EventIDT, MetadataT, SourceIDT);

    /// (IF) Has to perform filling of source identifiers list corresponding
    /// to list of specific event identifiers.
    virtual void collect_source_ids_for_set(
                                const std::list<EventID> &,
                                std::list<SubrangeMarkup> & ) const = 0;
};  // class iRangeQueryableStore

/** @} */

}  // namespace sV

# endif  // H_STROMA_V_METADATA_STORE_H

