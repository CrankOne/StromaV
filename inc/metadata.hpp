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
# include <unordered_set>

# include <goo_exception.hpp>

namespace sV {

typedef sV_MetadataTypeIndex MetadataTypeIndex;
typedef sV_Metadata Metadata;

namespace aux {
template<typename EventIDT, typename SpecificMetadataT> class iEventSource;
}  // namespace aux
template<typename EventIDT> class MetadataDictionary;

namespace aux {
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
    /// Default ctr to make SWIG happy.
    iMetadataTypeBase() { _FORBIDDEN_CALL_; }
    /// Ctr.
    iMetadataTypeBase( const std::string & tnm ) : _typeName(tnm) {}
    /// Virtual dtr of abstract base class.
    virtual ~iMetadataTypeBase() {}

    /// Returns its type name.
    virtual const std::string & name() const
        { return _typeName; }

    /// Type ID getter. Will be automatically implemented by template
    /// descendant (virtual static method idiom).
    virtual MetadataTypeIndex get_index() const = 0;

    template<typename EventIDT> friend class sV::MetadataDictionary;
};  // class iMetadataTypeBase

template<typename EventIDT>
class iTemplatedEventIDMetdataType : public iMetadataTypeBase {
public:
    typedef EventIDT EventID;
    typedef MetadataDictionary<EventID> SpecificDictionary;
private:
    /// Back references to the dictionaries where this type was registered.
    std::unordered_set<const SpecificDictionary *> _dictionariesBackRefs;
protected:
    /// Aux method adding dict instance backref (invoked by dict upon
    /// register).
    void _add_dict_backref( const SpecificDictionary & dictRef ) {
        _dictionariesBackRefs.insert( &dictRef );
    }
    /// Aux method removing dict instance backref (invoked by dict upon
    /// type remove).
    void _remove_dict_backref( const SpecificDictionary & dictRef ) {
        _dictionariesBackRefs.erase( &dictRef );
    }
public:
    /// Default ctr to make SWIG happy.
    iTemplatedEventIDMetdataType() { _FORBIDDEN_CALL_; }

    iTemplatedEventIDMetdataType( const std::string & tnm ) : 
                                  iMetadataTypeBase( tnm ) {}

    friend class sV::MetadataDictionary<EventID>;
};  // iTemplatedEventIDMetdataType

}  // namespace aux

// Definition of this class is given below. Logically, it has to be nested
// class of MetadataDictionary but nested tamplates piss SWIG off so we've
// tried to calm him down this way.
template<typename EventIDT,
         typename SpecificMetadataT>
class iMetadataType;

/**@class MetadataDictionary
 * @brief Index of metadata constructors.
 *
 * Implementation of particular metadata interface object is not possible
 * without knowledge of event identification internals, but one could guess
 * here some major traits considering event ID as a template type. Once the
 * event identification type and routines are defined, instantiation of this
 * template becomes possible.
 *
 * This class is designed to handle the "metadata type" abstraction. It
 * constituates indexing and storaging singletone instances of particular
 * metadata descriptions in order to make them accessable at a runtime.
 * */
template<typename EventIDT>
class MetadataDictionary {
public:
    typedef EventIDT EventID;
    typedef aux::iTemplatedEventIDMetdataType<EventID>
            iSpecificEventIDMetdataType;
private:
    void _cast_typecheck( const Metadata & md,
                          MetadataTypeIndex toTypeIdx ) const;
public:
    /// Container for all known metadata types (composition).
    std::unordered_set<aux::iMetadataTypeBase *> _types;
    /// Metadata types indexed by name.
    std::unordered_map<std::string, aux::iMetadataTypeBase *> _namedIndex;
    /// Metadata types indexed by id.
    std::unordered_map<MetadataTypeIndex, aux::iMetadataTypeBase *>
                                                                _encodedIndex;
public:
    /// Registers metadata type.
    MetadataTypeIndex register_metadata_type(
                            aux::iTemplatedEventIDMetdataType<EventID> *md);

    /// Removes metadata type.
    void remove_metadata_type( const std::string & );

    /// Returns metadata type name based on its index.
    const std::string & metadata_type_name( MetadataTypeIndex idx ) const;

    /// Payload cast method for metadata instance.
    template<typename T> T & cast( Metadata & md ) const {
        typedef iMetadataType<EventID, T> ItsType;
        _cast_typecheck( md, ItsType::type_index() );
        return *reinterpret_cast<T*>(md.payload);
    }

    /// Payload cast method for metadata instance.
    template<typename T> const T & cast( const Metadata & md ) const {
        typedef iMetadataType<EventID, T> ItsType;
        _cast_typecheck( md, ItsType::type_index() );
        return *reinterpret_cast<const T*>(md.payload);
    }

    template<typename SpecificMetadataT>
    const iMetadataType<EventID, SpecificMetadataT> &
    get_metadata_type() const {
        _TODO_;  // TODO
    }
};  // class MetadataDictionary

template<typename EventIDT> void
MetadataDictionary<EventIDT>::_cast_typecheck( const Metadata & md,
                                        MetadataTypeIndex toTypeIdx ) const {
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

template<typename EventIDT> MetadataTypeIndex
MetadataDictionary<EventIDT>::register_metadata_type(
                            aux::iTemplatedEventIDMetdataType<EventID> * md ) {
    //iSpecificEventIDMetdataType * md = &mdRef;
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
        //dynamic_cast<iSpecificEventIDMetdataType*>(md)
        //                    ->_add_dict_backref( *this );
        _types.insert( md );
        MetadataTypeIndex newIdx = sV_generate_metadata_type_id( md );
        md->_set_type_index( newIdx );
        _encodedIndex.emplace( newIdx, md );
    }
}

template<typename EventIDT> void
MetadataDictionary<EventIDT>::remove_metadata_type(const std::string & tName) {
    auto it = _namedIndex.find( tName );
    if( _namedIndex.end() == it ) {
        sV_log3( "Metadata type dictionary (%p) has no index for type \"%s\" "
                 "while remove operation invoked.", this, tName.c_str() );
        return;
    } else {
        _types.erase( it->second );
        _encodedIndex.erase( it->second->get_index() );
        static_cast<iSpecificEventIDMetdataType*>(it->second)
                                                ->_remove_dict_backref( *this );
        // has to be invoked last:
        _namedIndex.erase( it );
    }
}

template<typename EventIDT> const std::string & 
MetadataDictionary<EventIDT>::metadata_type_name( MetadataTypeIndex idx ) const {
    auto it = _encodedIndex.find( idx );
    if( _encodedIndex.end() == it ) {
        emraise( noSuchKey, "Metadata dictionary %p: unknown metadata "
            "type index %zu", this, (size_t) idx );
    }
    return it->second->name();
}

}  // namespace sV

# ifndef SWIG
#   include "imetadata.itcc"
# endif

# endif  // H_STROMA_V_METADATA_DICTIONARY_H

