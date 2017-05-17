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

# ifndef H_STROMA_V_METADATA_TYPE_TRAITS_H
# define H_STROMA_V_METADATA_TYPE_TRAITS_H

# include "type_base.hpp"

namespace sV {

// FWD
namespace aux {
template<typename EventIDT,
         typename MetadataT,
         typename SourceIDT> struct RangeReadingMarkupEntry;
}  // namespace aux
template<typename T> class MetadataDictionary;
template<typename EventIDT, typename MetadataT, typename SourceIDT> class iSectionalEventSource;
template<typename EventIDT, typename MetadataT> class iTMetadataType;
template<typename EventIDT, typename MetadataT, typename SourceIDT> class iTCachedMetadataType;

template<typename EventIDT, typename MetadataT, typename SourceIDT> struct ITMetadataStore;
template<typename EventIDT, typename MetadataT, typename SourceIDT> struct ITDisposableSourceManager;
template<typename EventIDT, typename MetadataT, typename SourceIDT> struct ITEventQueryableStore;
template<typename EventIDT, typename MetadataT, typename SourceIDT> struct ITRangeQueryableStore;
template<typename EventIDT, typename MetadataT, typename SourceIDT> struct ITSetQueryableStore;

/**@class MetadataTypeTraits
 * @brief Struct summing up the type traits related to particular metadata.
 *
 * The MetadataTypeTraits class provides a helper shortcut for metadata
 * definitions.
 * In order to use it user code has to parameterize this class and then use
 * its scoped type definitions as interfaces.
 *
 * For example, lets consider a metadata type referencing n-th word
 * inside a text by position of its first character and last character both
 * encapsulated in `std::pair<size_t, size_t>`.
 *
 * @snippet md-test1.cpp Defining metadata traits
 *
 * The `MetadataTraits` type definition now refers to scope where all the needed
 * interfacing classes are defined. One may further refer to these nested
 * definitions to shorten the parent class names for descendants:
 *
 * @snippet md-test1.cpp Using the scoped metadata interface
 *
 * Explanatory examples may be found in unit tests.
 *
 * @see md-test-common.hpp
 * @see md-test-common.cpp
 * @see md-test1.cpp
 * @see md-test2.cpp
 *
 * @ingroup mdat
 */

/*
 * Generic MetadataTypeTraits<> template used for sectioned source.
 */
template<typename EventIDT,
         typename MetadataT,
         typename SourceIDT=void>
class MetadataTypeTraits {
public:
    # ifndef SWIG
    static_assert( !std::is_void<SourceIDT>::value,
                   "Class with SourceIDT=void has explicit specialization." );
    # endif
    /* Basic types */
    typedef EventIDT EventID;  ///< Event identifier type
    typedef MetadataT Metadata;  ///< Source-related metadata type
    typedef SourceIDT SourceID;  ///< Source identifier (for sectioned sources)
    /* Induced types */
    /// Metadata factory type (vctr)
    typedef MetadataDictionary<EventID> TypesDictionary;
    /// Required interface for metadata type
    typedef iTCachedMetadataType<EventID, Metadata, SourceID>   iMetadataType;
    /// Required interface for sectional event source type
    typedef iSectionalEventSource<EventID, Metadata, SourceID>  iEventSource;
    /* Induced store interfaces */
    /// Range event identifier pair (from, to) type used for ranged reading
    typedef aux::RangeReadingMarkupEntry<EventID, Metadata, SourceID>
            SubrangeMarkup;
    /// Basic metadata store interface type (querying interface), see \ref ITMetadataStore
    typedef ITMetadataStore<EventID, Metadata, SourceID>            iMetadataStore;
    /// See \ref ITDisposableSourceManager
    typedef ITDisposableSourceManager<EventID, Metadata, SourceID>  iDisposableSourceManager;
    /// See \ref ITEventQueryableStore
    typedef ITEventQueryableStore<EventID, Metadata, SourceID>      iEventQueryableStore;
    /// See \ref ITRangeQueryableStore
    typedef ITRangeQueryableStore<EventID, Metadata, SourceID>      iRangeQueryableStore;
    /// See \ref ITSetQueryableStore
    typedef ITSetQueryableStore<EventID, Metadata, SourceID>        iSetQueryableStore;

    /// A helper macro defining common types deduced from template parameters.
    /// May be used at the public section of class declarations. This macro
    /// has to be used for sectioned sources. For bulk sources use
    /// sV_METADATA_IMPORT_BULK_TRAITS.
    # define sV_METADATA_IMPORT_SECT_TRAITS( EIDT, MDTT, SIDT )             \
    /* Basic types */                                                       \
    typedef EIDT EventID;                                                   \
    typedef MDTT Metadata;                                                  \
    typedef SIDT SourceID;                                                  \
    typedef ::sV::MetadataTypeTraits<EventID, Metadata, SourceID> Traits;   \
    /* Induced types */                                                     \
    typedef typename Traits::TypesDictionary TypesDictionary;       \
    typedef typename Traits::iMetadataType iMetadataType;                   \
    typedef typename Traits::iEventSource iEventSource;                     \
    typedef typename Traits::iMetadataStore iMetadataStore;                 \
    typedef typename Traits::SubrangeMarkup SubrangeMarkup;                 \
    /* ... */

private:
    /// Metadata type identifier that has to be set upon construction. Note,
    /// that this static-template field need to be s
    static MetadataTypeIndex _typeIndex;
protected:
    static void _set_type_index( MetadataTypeIndex desiredIdx ) {
        if( _typeIndex ) {
            if( _typeIndex != desiredIdx ) {
                emraise( badState, "Metadata type traits %p already has type "
                    "index (%zu) while another assignment invoked with new "
                    "code %zu.",
                    &_typeIndex, (size_t) _typeIndex, (size_t) desiredIdx
                );
            } else {
                sV_log3( "Metadata type traits %p already has type index "
                        "(%zu) while some internal code invokes assignment of "
                        "same index.",
                        &_typeIndex, (size_t) _typeIndex );
            }
        } else {
            _typeIndex = desiredIdx;
            //sV_log2( "Index %zu assigned for metadata type traits "
            //         "\"%s\" (%p).\n", (size_t) desiredIdx, &_typeIndex );
        }
    }
public:
    static MetadataTypeIndex index() { return _typeIndex; }
    friend class MetadataDictionary<EventID>;
    friend class iTMetadataType<EventIDT, MetadataT>;
};

// Note that CLANG will probably generate a side instance for wrappers.
template<typename EventIDT,
         typename MetadataT,
         typename SourceIDT>
MetadataTypeIndex MetadataTypeTraits<EventIDT, MetadataT, SourceIDT>::_typeIndex = 0;

/*
 * Partial MetadataTypeTraits<> template used for bulk source.
 */
template<typename EventIDT,
         typename MetadataT>
struct MetadataTypeTraits<EventIDT, MetadataT, void> {
public:
    typedef EventIDT EventID;
    typedef MetadataT Metadata;
    typedef MetadataDictionary<EventID> TypesDictionary;
    typedef iBulkEventSource<EventID, Metadata> iEventSource;
    typedef iTMetadataType<EventID, Metadata> iMetadataType;

    /// A helper macro defining common types deduced from template parameters.
    /// May be used at the public section of class declarations. This macro
    /// has to be used for bulk sources. For sectioned sources use
    /// sV_METADATA_IMPORT_SECT_TRAITS.
    # define sV_METADATA_IMPORT_BULK_TRAITS( EIDT, MDTT )                   \
    /* Basic types */                                                       \
    typedef EIDT EventID;                                                   \
    typedef MDTT Metadata;                                                  \
    typedef ::sV::MetadataTypeTraits<EIDT, MDTT, void> Traits;              \
    /* Induced types */                                                     \
    typedef typename Traits::TypesDictionary TypesDictionary;               \
    typedef typename Traits::iEventSource iEventSource;                     \
    typedef typename Traits::iMetadataType iMetadataType;                   \
    /* ... */
private:
    /// Metadata type identifier that has to be set upon construction. Note,
    /// that this static-template field need to be s
    static MetadataTypeIndex _typeIndex;
protected:
    static void _set_type_index( MetadataTypeIndex desiredIdx ) {
        if( _typeIndex ) {
            if( _typeIndex != desiredIdx ) {
                emraise( badState, "Metadata type traits %p already has type "
                    "index (%zu) while another assignment invoked with new "
                    "code %zu.",
                    &_typeIndex, (size_t) _typeIndex, (size_t) desiredIdx
                );
            } else {
                sV_log3( "Metadata type traits %p already has type index "
                        "(%zu) while some internal code invokes assignment of "
                        "same index.",
                        &_typeIndex, (size_t) _typeIndex );
            }
        } else {
            _typeIndex = desiredIdx;
            //sV_log2( "Index %zu assigned for metadata type traits "
            //         "(%p).\n", (size_t) desiredIdx, &_typeIndex );
        }
    }
public:
    static MetadataTypeIndex index() { return _typeIndex; }
    friend class MetadataDictionary<EventID>;
    friend class iTMetadataType<EventIDT, MetadataT>;
};

// Note that CLANG will probably generate a side instance for wrappers.
template<typename EventIDT,
         typename MetadataT>
MetadataTypeIndex MetadataTypeTraits<EventIDT, MetadataT, void>::_typeIndex = 0;

# if 0
template<typename EventIDT, typename MetadataT> void
iTMetadataType<EventIDT, MetadataT>::_set_type_index(
                                            MetadataTypeIndex desiredIdx ) {
    MetadataTypeTraits<EventIDT, MetadataT>
                                            ::_set_type_index( desiredIdx );
}

template<typename EventIDT, typename MetadataT>
MetadataTypeIndex 
iTMetadataType<EventIDT, MetadataT>::get_index() const {
    return MetadataTypeTraits<EventIDT, MetadataT>::index();
}
# endif

}  // namespace sV

# endif  // H_STROMA_V_METADATA_TYPE_TRAITS_H

