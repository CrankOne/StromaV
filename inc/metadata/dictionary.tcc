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

# ifndef H_STROMA_V_METADATA_TYPES_DICTIONARY_H
# define H_STROMA_V_METADATA_TYPES_DICTIONARY_H

# include "type_base.hpp"
# include "type.tcc"

namespace sV {

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
 *
 * This class also provides a special template method for querying the metadata
 * type instances by their C++ type.
 * */
template<typename EventIDT>
class MetadataDictionary {
public:
    typedef EventIDT EventID;
    typedef sV::aux::iTemplatedEventIDMetadataType<EventID>
            iSpecificEventIDMetdataType;
    typedef MetadataDictionary<EventIDT> Self;
private:
    void _cast_typecheck( const C_Metadata & md,
                          MetadataTypeIndex toTypeIdx ) const;
public:
    /// Container for all known metadata types (composition).
    std::unordered_set<iSpecificEventIDMetdataType *> _types;
    /// Metadata types indexed by name.
    std::unordered_map<std::string, iSpecificEventIDMetdataType *> _namedIndex;
    /// Metadata types indexed by id.
    std::unordered_map<MetadataTypeIndex, iSpecificEventIDMetdataType *>
                                                                _encodedIndex;
public:
    MetadataDictionary() {}

    /// Registers metadata type. Front-end interfacing function. Need to be
    /// called with metadata type instances before providing any operations.
    MetadataTypeIndex register_metadata_type(
                                    iSpecificEventIDMetdataType & md);

    /// Removes metadata type.
    void remove_metadata_type( const std::string & );

    /// Returns metadata type name based on its index.
    const std::string & metadata_type_name( MetadataTypeIndex idx ) const;

    /// Returns reference to metadata type by its name.
    const iSpecificEventIDMetdataType & metadata_type(
                                            const std::string & tnm ) const {
        auto it = _namedIndex.find( tnm );
        if( _namedIndex.end() == it ) {
            emraise( noSuchKey, "Metadata types dictionary %p has no metadata "
                    "type \"%s\" registered.", this, tnm.c_str() );
        }
        return *(it->second);
    }

    /// Payload cast method for metadata instance.
    template<typename T> T & cast( C_Metadata & md ) const {
        typedef iTMetadataType<EventID, T> ItsType;
        _cast_typecheck( md, ItsType::type_index() );
        return *reinterpret_cast<T*>(md.payload);
    }

    /// Payload cast method for metadata instance.
    template<typename T> const T & cast( const C_Metadata & md ) const {
        typedef iTMetadataType<EventID, T> ItsType;
        _cast_typecheck( md, ItsType::type_index() );
        return *reinterpret_cast<const T*>(md.payload);
    }

    template<typename SpecificMetadataT>
    const iTMetadataType<EventID, SpecificMetadataT> &
    get_metadata_type() const {
        // This method will be called with specific metadata objec C++-type
        // as a template argument. It has to return an instance of
        // corresponding iTMetadataType --- just found instance of the
        // iTMetadataType<EventID, SpecificMetadataT>. Here we involve a
        // idiomatic "virtual static method".
        auto it = _encodedIndex.find(
                    iTMetadataType<EventID, SpecificMetadataT>::type_index() );
        if( _encodedIndex.end() == it ) {
            emraise( notFound, "Metadata types dictionary %p has no "
                     " type registered with ID %#x.", this,
                     iTMetadataType<EventID, SpecificMetadataT>::type_index() );
        }
        return static_cast<const iTMetadataType<EventID, SpecificMetadataT> &>(
                                                                *(it->second));
    }

    template<typename SpecificMetadataT>
    iTMetadataType<EventID, SpecificMetadataT> &
    get_metadata_type() {
        return const_cast<iTMetadataType<EventID, SpecificMetadataT> &>(
                static_cast<const Self *>(this)
                ->get_metadata_type<SpecificMetadataT>());
    }
};  // class MetadataDictionary

template<typename EventIDT> void
MetadataDictionary<EventIDT>::_cast_typecheck( const C_Metadata & md,
                                        MetadataTypeIndex toTypeIdx ) const {
    if( md.typeIndex != toTypeIdx ) {
        sV_loge("Metadata %p has type %#x (%s) while type cast to %#x (%s) "
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
                                        iSpecificEventIDMetdataType & mdt ) {
    sV_log3( "Attempt to register metadata type \"%s\" (%p) "
             "at dictionary %p.\n", mdt.name().c_str(), &mdt, this );
    auto insertionResult = _namedIndex.emplace( mdt.name(), &mdt );
    if( !insertionResult.second ) {
        if( insertionResult.first->second == &mdt ) {
            sV_log3( "Metadata type \"%s\" (%p) already known to metadata "
                     "types dictionary %p. Ignored.\n", mdt.name().c_str(),
                     &mdt, this );
            return mdt.get_index();
        } else {
            emraise( badState,
                     "Metadata type \"%s\" (%p) can not be inserted to  "
                     "types dictionary %p. Name not unique. Another type: "
                     "%p.", mdt.name().c_str(), &mdt, this,
                     insertionResult.first->second );
        }
    } else {
        _types.insert( &mdt );
        MetadataTypeIndex newIdx = sV_generate_metadata_type_id( &mdt );
        mdt._set_type_index( newIdx );
        _encodedIndex.emplace( newIdx, &mdt );
        sV_log2( "Metadata type %s (%p) registered at dictionary %p "
                 "with index %#x.\n",
                 mdt.name().c_str(), &mdt, this, (int) newIdx );
        return newIdx;
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

# endif  // H_STROMA_V_METADATA_TYPES_DICTIONARY_H

