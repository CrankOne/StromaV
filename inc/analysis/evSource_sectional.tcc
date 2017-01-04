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

# ifndef H_STROMA_V_SECTIONAL_EVENT_SOURCE_H
# define H_STROMA_V_SECTIONAL_EVENT_SOURCE_H

# include "evSource_RA.tcc"
# include "evSource_identifiable.tcc"
# include "metadata/dictionary.tcc"
# include "metadata/store.tcc"

namespace sV {

/**@class iSectionalEventSource
 * @brief Interface base class for identifiable event sources.
 *
 * This template interface is designed for description of multiple sources with
 * unique content usually corresponding to artifacts. For instance the
 * experimental statistics are often distributed among multiple homogenious
 * files. Each file might be represented by instance of descendant class.
 *
 * Such artifacts usually supports random access at their own scope.
 * For non-sectional data sources with random access but without internal
 * sectioning, the similar template class called
 * iBatchEventSource<...> was designed.
 *
 * Adjoint metadata template base class is called iCachedMetadataType.
 * */
template<typename EventIDT,
         typename SpecificMetadataT,
         typename SourceIDT>
class iSectionalEventSource :
            public aux::iRandomAccessEventSource<EventIDT, SpecificMetadataT>,
            public mixins::iIdentifiableEventSource<SourceIDT> {
public:
    typedef EventIDT EventID;
    typedef SpecificMetadataT SpecificMetadata;
    typedef iCachedMetadataType<EventID, SpecificMetadata, SourceIDT>
            iSpecificMetadataType;
    typedef aux::iRandomAccessEventSource<EventIDT, SpecificMetadataT>
            Parent;
protected:
    virtual const SpecificMetadata * _V_acquire_my_metadata() final {
        const iSpecificMetadataType & mdt =
                    static_cast<const iSpecificMetadataType &> (
                    Parent::metadata_types_dict().template get_metadata_type<SpecificMetadata>() );
        return &( mdt.acquire_metadata_for( *this ) );
    }
public:
    iSectionalEventSource(aux::iEventSequence::Features_t fts = 0x0) :
            aux::iEventSequence( fts
                               | aux::iEventSequence::identifiable ),
            aux::iRandomAccessEventSource<EventIDT, SpecificMetadataT>( fts ),
            mixins::iIdentifiableEventSource<SourceIDT>( fts ) {}

    iSectionalEventSource( SourceIDT & id,
                           aux::iEventSequence::Features_t fts=0x0 ) :
            aux::iEventSequence( fts
                               | aux::iEventSequence::identifiable ),
            aux::iRandomAccessEventSource<EventIDT, SpecificMetadataT>( fts ),
            mixins::iIdentifiableEventSource<SourceIDT>( fts, id ) {}

    iSectionalEventSource( MetadataDictionary<EventIDT> & mtDict,
                           aux::iEventSequence::Features_t fts=0x0 ) :
            aux::iEventSequence( fts
                               | aux::iEventSequence::identifiable ),
            aux::iRandomAccessEventSource<EventIDT, SpecificMetadataT>( mtDict, fts ),
            mixins::iIdentifiableEventSource<SourceIDT>( fts ) {}

    iSectionalEventSource( SourceIDT & id,
                           MetadataDictionary<EventIDT> & mtDict,
                           aux::iEventSequence::Features_t fts=0x0 ) :
            aux::iEventSequence( fts
                               | aux::iEventSequence::identifiable ),
            aux::iRandomAccessEventSource<EventIDT, SpecificMetadataT>( mtDict, fts ),
            mixins::iIdentifiableEventSource<SourceIDT>( id, fts ) {}

    virtual ~iSectionalEventSource() {}
};  // class iSectionalEventSource

}  // namespace sV

# endif // H_STROMA_V_SECTIONAL_EVENT_SOURCE_H

