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

# ifndef H_STROMA_V_EVENT_SOURCE_H
# define H_STROMA_V_EVENT_SOURCE_H

# include "pipeline.hpp"
# include "identifiable_ev_source.tcc"

namespace sV {

/**@class iEventSource
 * @brief Interface base class for identifiable event sources.
 *
 * This template interface is designed for description of multiple sources with
 * unique content usually corresponding to artifacts. E.g. for
 * experimental statistics distributed among multiple homogenious files, each
 * file might be represented by instance of descendant class.
 * */
template<typename EventIDT,
         typename SpecificMetadataT,
         typename SourceIDT>
class iEventSource : public iUniqueEventSource<EventIDT, SpecificMetadataT>,
                     public mixins::iIdentifiableEventSource<SourceIDT> {
public:
    typedef EventIDT EventID;
    typedef SpecificMetadataT SpecificMetadata;
    typedef iCachedMetadataType<EventID, SpecificMetadata, SourceIDT>
            iSpecificMetadataType;
public:
    iEventSource() {}
    virtual ~iEventSource() {}

    virtual const SpecificMetadata & metadata(
                        sV::MetadataDictionary<EventIDT> & mDict ) override {
        if( !_mDatCache ) {
            const iSpecificMetadataType & mdt = 
                        mDict.template get_metadata_type<SpecificMetadata>();
            _mDatCache = &( mdt.acquire_metadata_for( *this ) );
        }
        return *_mDatCache;
    }
};  // class iEventSource

}  // namespace sV

# endif  // H_STROMA_V_EVENT_SOURCE_H
