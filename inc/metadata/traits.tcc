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
template<typename T> class MetadataDictionary;
template<typename EventIDT, typename MetadataT, typename SourceIDT> class iSectionalEventSource;
template<typename EventIDT, typename MetadataT> class iMetadataType;
template<typename EventIDT, typename MetadataT, typename SourceIDT> class iCachedMetadataType;

/**???
 * Generic MetadataTypeTraits<> template used for sectioned source.
 */
template<typename EventIDT,
         typename SpecificMetadataT,
         typename SourceIDT=void>
class MetadataTypeTraits {
public:
    # ifndef SWIG
    static_assert( !std::is_void<SourceIDT>::value,
                   "Class with SourceIDT=void has explicit specialization." );
    # endif
    typedef EventIDT EventID;
    typedef SpecificMetadataT Metadata;
    typedef MetadataDictionary<EventID> MetadataTypesDictionary;
    typedef SourceIDT SourceID;
    typedef iSectionalEventSource<EventID, Metadata, SourceID> iEventSource;
    typedef iCachedMetadataType<EventID, Metadata, SourceID>
            iSpecificMetadataType;
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
            sV_log2( "Index %zu assigned for metadata type traits "
                     "\"%s\" (%p).\n", (size_t) desiredIdx, &_typeIndex );
        }
    }
public:
    static MetadataTypeIndex index() { return _typeIndex; }
    friend class MetadataDictionary<EventID>;
    friend class iMetadataType<EventIDT, SpecificMetadataT>;
};

// Note that CLANG will probably generate a side instance for wrappers.
template<typename EventIDT,
         typename SpecificMetadataT,
         typename SourceIDT>
MetadataTypeIndex MetadataTypeTraits<EventIDT, SpecificMetadataT, SourceIDT>::_typeIndex = 0;

/**???
 * Partial MetadataTypeTraits<> template used for batch source.
 */
template<typename EventIDT,
         typename SpecificMetadataT>
struct MetadataTypeTraits<EventIDT, SpecificMetadataT, void> {
public:
    typedef EventIDT EventID;
    typedef SpecificMetadataT SpecificMetadata;
    typedef MetadataDictionary<EventID> MetadataTypesDictionary;
    typedef iBatchEventSource<EventID, SpecificMetadata> iEventSource;
    typedef iMetadataType<EventID, SpecificMetadata> iSpecificMetadataType;
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
            sV_log2( "Index %zu assigned for metadata type traits "
                     "\"%s\" (%p).\n", (size_t) desiredIdx, &_typeIndex );
        }
    }
public:
    static MetadataTypeIndex index() { return _typeIndex; }
    friend class MetadataDictionary<EventID>;
    friend class iMetadataType<EventIDT, SpecificMetadataT>;
};

// Note that CLANG will probably generate a side instance for wrappers.
template<typename EventIDT,
         typename SpecificMetadataT>
MetadataTypeIndex MetadataTypeTraits<EventIDT, SpecificMetadataT, void>::_typeIndex = 0;

# if 0
template<typename EventIDT, typename SpecificMetadataT> void
iMetadataType<EventIDT, SpecificMetadataT>::_set_type_index(
                                            MetadataTypeIndex desiredIdx ) {
    MetadataTypeTraits<EventIDT, SpecificMetadataT>
                                            ::_set_type_index( desiredIdx );
}

template<typename EventIDT, typename SpecificMetadataT>
MetadataTypeIndex 
iMetadataType<EventIDT, SpecificMetadataT>::get_index() const {
    return MetadataTypeTraits<EventIDT, SpecificMetadataT>::index();
}
# endif

}  // namespace sV

# endif  // H_STROMA_V_METADATA_TYPE_TRAITS_H

