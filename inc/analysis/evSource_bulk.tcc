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

# ifndef H_STROMA_V_BULK_EVENT_SOURCE_H
# define H_STROMA_V_BULK_EVENT_SOURCE_H

# include "evSource_RA.tcc"
# include "metadata/dictionary.tcc"

namespace sV {

/**@class iBulkEventSource
 * @brief Interface class for unique event source supporting metadata for
 *        random access.
 * 
 * Template class for event sequences supporting random access. All the 
 * `event_read_*` functions can return false upon non-critical failure as well
 * as indicate a critical error by throwing an exception.
 *
 * This class is designed as an intermediate representation of somewhat ideal
 * "unique" event source that supports metadata indexing on its own scope. It
 * can be further used as a container or manager dispatching queries among
 * multiple minor event source instances implemented by its sibling ---
 * iEventSource which provides additional identification mechanics for
 * multiple source instances of the same type.
 *
 * Adjoint metadata template base class is called iTMetadataType.
 * */
template<typename EventIDT,
         typename SpecificMetadataT>
class iBulkEventSource :
            public aux::iRandomAccessEventSource<EventIDT, SpecificMetadataT> {
public:
    typedef EventIDT EventID;
    typedef SpecificMetadataT SpecificMetadata;
    typedef iTMetadataType<EventID, SpecificMetadata> iSpecificMetadataType;
    typedef aux::iRandomAccessEventSource<EventIDT, SpecificMetadataT> Parent;
    typedef typename Parent::Event Event;
protected:
    virtual const SpecificMetadata * _V_acquire_my_metadata() final {
        const iSpecificMetadataType & mdt = 
                    Parent
                    ::metadata_types_dict()
                    .template get_metadata_type<SpecificMetadata>();
        return &( mdt.acquire_metadata( *this ) );
    }

    iBulkEventSource() : aux::iEventSequence( 0x0 ), 
                         Parent( 0x0 ) {}

    iBulkEventSource( MetadataDictionary<EventID> & mdtDictRef ) :
                aux::iEventSequence( 0x0 ),
                Parent( mdtDictRef, 0x0 ) {}
public:
    virtual ~iBulkEventSource() {}
};  // class iBulkEventSource

}  // namespace sV

# endif  // H_STROMA_V_BULK_EVENT_SOURCE_H

