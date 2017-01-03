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

# include "c_types.h"
# include "app/app.h"
//# include "identifiable_ev_source.tcc"

# include <typeinfo>
# include <list>
# include <unordered_map>
# include <unordered_set>

# include <goo_exception.hpp>

namespace sV {

typedef sV_MetadataTypeIndex MetadataTypeIndex;
typedef sV_Metadata C_Metadata;
template<typename EventIDT, typename SpecificMetadataT>
        class iBatchEventSource;
template<typename EventIDT>
        class MetadataDictionary;

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
    /// Ctr.
    iMetadataTypeBase( const std::string & tnm ) : _typeName(tnm) {}
    /// Virtual dtr of abstract base class.
    virtual ~iMetadataTypeBase() {}

    /// Returns its type name.
    virtual const std::string & name() const
        { return _typeName; }

    /// Type ID getter. Will be automatically implemented by template
    /// descendant.
    virtual MetadataTypeIndex get_index() const = 0;

    template<typename EventIDT> friend class sV::MetadataDictionary;
};  // class iMetadataTypeBase


template<typename EventIDT>
class iTemplatedEventIDMetadataType : public iMetadataTypeBase {
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
    iTemplatedEventIDMetadataType( const std::string & tnm ) : 
                                  iMetadataTypeBase( tnm ) {}

    friend class sV::MetadataDictionary<EventID>;
};  // iTemplatedEventIDMetadataType

}  // namespace aux

}  // namespace sV

# endif  // H_STROMA_V_METADATA_DICTIONARY_H

