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

# ifndef H_STROMA_V_METADATA_DICTIONARY_H
# define H_STROMA_V_METADATA_DICTIONARY_H

# include "metadata.h"
# include "app/app.h"

# include <typeinfo>
# include <list>
# include <unordered_map>

# include <goo_exception.hpp>

namespace sV {

typedef sV_MetadataTypeIndex MetadataTypeIndex;
typedef sV_Metadata Metadata;


template<typename EventIDT, typename SpecificMetadataT> class iEventSource;
//template<typename EventIDT> class MetadataDictionary;

/**@class iMetadataTypeBase
 * @brief Base for metadata templates.
 *
 * Metadata type describes how to utilize the metadata and can be referred with
 * string exprassion (metadata type name). This trait describes its major
 * interfacing logic.
 * */
class iMetadataTypeBase {
private:
    /// Constant attribute, set by ctr.
    const std::string _typeName;
protected:
    /// For internal use --- automatically implemented by template descendant
    /// and invoked by MetadataDictionary implementation.
    virtual void _set_type_index( MetadataTypeIndex ) = 0;
public:
    /// Ctr.
    iMetadataTypeBase( const std::string & tnm ) : _typeName(tnm) {}
    /// Returns its type name.
    virtual const std::string & name() const
        { return _typeName; }

    /// Type ID getter. Will be automatically implemented by template
    /// descendant.
    virtual MetadataTypeIndex get_index() = 0;

    template<typename EventIDT> friend class MetadataDictionary;
};  // class iMetadataTypeBase


/**@class MetadataDictionary
 * @brief Index of metadata constructors.
 *
 * Implementation of particular metadata interfaceobject is not possible
 * without knowledge of event identification internals, but one could guess
 * here some major traits considering event ID as a template type.
 * */
template<typename EventIDT>
class MetadataDictionary {
public:
    typedef EventIDT EventID;
    template<typename SpecificMetadataT> class iMetadataType;
private:
    void _cast_typecheck( const Metadata & md, MetadataTypeIndex toTypeIdx ) const {
        if( md.typeIndex != toTypeIdx ) {
            sV_loge("Metadata %p has type 0x%x (%s) while type cast to 0x%x (%s) "
                    "was requested.", md.payload,
                    (uint16_t) md.typeIndex,
                    metadata_type_name( md.typeIndex ).c_str(),
                    (uint16_t) toTypeIdx,
                    metadata_type_name( toTypeIdx ).c_str() );
            throw std::bad_cast();
        }
    }
public:
    /// Container for all known metadata types (composition).
    std::list<iMetadataTypeBase *> _types;
    /// Metadata types indexed by name.
    std::unordered_map<std::string,       iMetadataTypeBase *> _namedIndex;
    /// Metadata types indexed by id.
    std::unordered_map<MetadataTypeIndex, iMetadataTypeBase *> _encodedIndex;
public:
    /// Registers metadata.
    MetadataTypeIndex register_metadata( iMetadataTypeBase * md ) {        
        auto insertionResult = _namedIndex.emplace( md->name(), md );
        if( !insertionResult.second ) {
            if( insertionResult.first->second == md ) {
                sV_log3( "Metadata type \"%s\" (%p) already known to metadata "
                         "types dictionary %p. Ignored.\n", md->name().c_str(),
                         md, this );
            } else {
                emraise( badState,
                         "Metadata type \"%s\" (%p) can not be inserted to  "
                         "types dictionary %p. Name not unique. Another type: "
                         "%p.", md->name().c_str(), md, this,
                         insertionResult.first->second );
            }
        } else {
            _types.push_back( md );
            MetadataTypeIndex newIdx = _types.size();
            md->_set_type_index( newIdx );
            _encodedIndex.emplace( newIdx, md );
        }
    }

    /// Returns metadata type name based on its index.
    const std::string & metadata_type_name( MetadataTypeIndex idx ) const {
        auto it = _encodedIndex.find( idx );
        if( _encodedIndex.end() == it ) {
            emraise( noSuchKey, "Metadata dictionary %p: unknown metadata "
                "type index %u", this, (unsigned int) idx );
        }
        return it->second->name();
    }

    /// Payload cast method for metadata instance.
    template<typename T> T & cast( Metadata & md ) const {
        typedef MetadataDictionary<EventID>::iMetadataType<T> ItsType;
        _cast_typecheck( md, ItsType::type_index() );
        return *reinterpret_cast<T*>(md.payload);
    }

    /// Payload cast method for metadata instance.
    template<typename T> const T & cast( const Metadata & md ) const {
        typedef MetadataDictionary<EventID>::iMetadataType<T> ItsType;
        _cast_typecheck( md, ItsType::type_index() );
        return *reinterpret_cast<const T*>(md.payload);
    }
};  // class MetadataDictionary


/**@class iMetadataType
 * @brief Metadata type intefrace for certain event identificator.
 *
 * Provides interfacing functions for implementing operations with metadata of
 * certain type.
 * */
template<typename EventIDT>
template<typename SpecificMetadataT>
class MetadataDictionary<EventIDT>::iMetadataType : public iMetadataTypeBase {
public:
    typedef EventIDT                                EventID;
    typedef SpecificMetadataT                       SpecificMetadata;
    typedef MetadataDictionary<EventID>             SpecificDictionary;
    typedef iEventSource<EventID, SpecificMetadata> DataSource;
private:
    /// Metadata type identifier that has to be set upon construction. Note,
    /// that this static-template field need to be s
    static MetadataTypeIndex _typeIndex;
protected:
    virtual void _set_type_index( MetadataTypeIndex desiredIdx ) final {
        if( _typeIndex ) {
            if( _typeIndex != desiredIdx ) {
                emraise( badState, "Metadata type %p already has type index "
                    "(%u) while another assignment invoked with new code %u.",
                    this, (unsigned int) _typeIndex, (unsigned int) desiredIdx
                );
            } else {
                sV_log3( "Metadata type %p already has type index (%u) while "
                        "some internal code invokes assignment of same index.",
                        this, (unsigned int) _typeIndex );
            }
        }
    }
    /// Obtains metadata from provided source (IF).
    virtual bool _V_acquire_metadata( DataSource & ) = 0;
    /// Stores given metadata at destination handle provided by dict (IF).
    virtual bool _V_store_metadata( const SpecificMetadata & mdInstance,
                            SpecificDictionary & dictInstance ) = 0;
    /// Restores metadata corresponding to provided event identifier (IF).
    virtual bool _V_restore_metadata_for( SpecificMetadata & mdDestInstance,
                               const SpecificDictionary & dictInstance,
                               const EventID & eid ) = 0;
public:
    /// Obtains metadata from provided source.
    bool acquire_metadata( DataSource & s ) { return _V_acquire_metadata(s); }
    /// Stores given metadata at destination handle provided by dict.
    bool store_metadata( const SpecificMetadata & mdInstance,
                         SpecificDictionary & dictInstance ) {
        return _V_store_metadata(mdInstance, dictInstance); }
    /// Restores metadata corresponding to provided event identifier.
    bool restore_metadata_for( SpecificMetadata & mdDestInstance,
                               const SpecificDictionary & dictInstance,
                               const EventID & eid ) {
        return _V_restore_metadata_for(mdDestInstance, dictInstance, eid); }
    /// Returns encoded type.
    static MetadataTypeIndex type_index() { return _typeIndex; }

    /// Returns encoded type. Same as static method type_index().
    virtual MetadataTypeIndex get_index() const final { return type_index(); }
};  // class iMetadata

template<typename EventIDT>
template<typename SpecificMetadataT>
MetadataTypeIndex MetadataDictionary<EventIDT> \
        ::iMetadataType<SpecificMetadataT>
        ::_typeIndex = 0;

}  // namespace sV

# endif  // H_STROMA_V_METADATA_DICTIONARY_H

