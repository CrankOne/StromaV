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

# ifndef H_STROMA_V_METADATA_TYPE_IFACE_H
# define H_STROMA_V_METADATA_TYPE_IFACE_H

# include "type_base.hpp"

namespace sV {

/**@class iMetadataType
 * @brief Metadata type intefrace for certain event identificator.
 *
 * The metadata type abstraction is responsible for acquizition the metadata
 * instances. It's implementations has to define acquizition from various
 * sources.
 *
 * Provides interfacing functions for operations with metadata of certain type.
 * 
 * This is a "template singletone" --- object of each particular template
 * specification has to be instantiated only once for each parent metadata
 * dictionary.
 * */
template<typename EventIDT, typename SpecificMetadataT>
class iMetadataType :
            public sV::aux::iTemplatedEventIDMetadataType<EventIDT> {
public:
    typedef EventIDT                                EventID;
    typedef SpecificMetadataT                       SpecificMetadata;
    typedef iBatchEventSource<EventID, SpecificMetadata> DataSource;
    typedef MetadataDictionary<EventID>             SpecificDictionary;
    typedef sV::aux::iTemplatedEventIDMetadataType<EventID> Parent;
    //typedef typename SpecificDictionary::iSpecificEventIDMetdataType iSpecificEventIDMetdataType;
public:
    /// Metadata type identifier that has to be set upon construction. Note,
    /// that this static-template field need to be s
    static MetadataTypeIndex _typeIndex;
protected:
    virtual void _set_type_index( MetadataTypeIndex desiredIdx ) final {
        if( _typeIndex ) {
            if( _typeIndex != desiredIdx ) {
                emraise( badState, "Metadata type %p already has type index "
                    "(%zu) while another assignment invoked with new code %zu.",
                    this, (size_t) _typeIndex, (size_t) desiredIdx
                );
            } else {
                sV_log3( "Metadata type %p already has type index (%zu) while "
                        "some internal code invokes assignment of same index.",
                        this, (size_t) _typeIndex );
            }
        }
        _typeIndex = desiredIdx;
        sV_log2( "Index %zu assigned for metadata type instance "
                 "\"%s\" (%p).\n", (size_t) desiredIdx,
                 this->name().c_str(), this );
        sV_log2( "XXX idx var addr: %p\n", &_typeIndex );
    }
    /// (IM) Obtains metadata from provided source. The particular
    /// implementation can include direct acquizition or look-up
    /// among side cache resource (monolithic).
    virtual SpecificMetadata & _V_acquire_metadata( DataSource & ) const = 0;
public:
    iMetadataType( const std::string & tnm ) : Parent( tnm ) {}

    /// Obtains metadata for provided source.
    SpecificMetadata & acquire_metadata( DataSource & s ) const {
                return _V_acquire_metadata(s); }

    /// Returns encoded type.
    static MetadataTypeIndex type_index() {
        sV_log2( "XXX ===> idx var addr: %p\n", &_typeIndex );
        return _typeIndex; }

    /// Returns encoded type. Same as static method type_index().
    virtual MetadataTypeIndex get_index() const final { return type_index(); }

    friend class MetadataDictionary<EventID>;
};  // class iMetadataType

template<typename EventIDT, typename SpecificMetadataT>
MetadataTypeIndex iMetadataType<EventIDT, SpecificMetadataT>::_typeIndex = 0;

}  // namespace sV

# endif  // H_STROMA_V_METADATA_TYPE_IFACE_H
