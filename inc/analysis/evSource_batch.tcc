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

# ifndef H_STROMA_V_BATCH_EVENT_SOURCE_H
# define H_STROMA_V_BATCH_EVENT_SOURCE_H

# include "evSource_RA.tcc"
# include "metadata/dictionary.tcc"

namespace sV {

/**@class iBatchEventSource
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
 * Adjoint metadata template base class is called iMetadataType.
 * */
template<typename EventIDT,
         typename SpecificMetadataT>
class iBatchEventSource :
            public aux::iRandomAccessEventSource<EventIDT, SpecificMetadataT> {
public:
    typedef EventIDT EventID;
    typedef SpecificMetadataT SpecificMetadata;
    typedef iMetadataType<EventID, SpecificMetadata> iSpecificMetadataType;
    typedef aux::iRandomAccessEventSource<EventIDT, SpecificMetadataT> Parent;
    typedef typename Parent::Event Event;
private:
    SpecificMetadata * _raMDatCache;
protected:
    /// This abstract metod has to read the event using metadata instance (IF).
    virtual Event * _V_md_event_read_single( const SpecificMetadata & md,
                                             const EventIDT & eid ) = 0;

    /// This abstract method has to read range of events using metadata
    /// instance (IF).
    virtual std::unique_ptr<aux::iEventSequence> _V_md_event_read_range(
                                        const SpecificMetadata & md,
                                        const EventIDT & eidFrom,
                                        const EventIDT & eidTo ) = 0;

    virtual std::unique_ptr<aux::iEventSequence> _V_md_event_read_list(
                                    const SpecificMetadata & md,
                                    const std::list<EventID> & eidsList ) = 0;

    /// Random access read event based on provided metadata
    /// information.
    virtual Event * _V_event_read_single( const EventIDT & eid ) override {
        return _V_md_event_read_single( metadata(), eid );
    }

    /// Read events in some ID range.
    virtual std::unique_ptr<aux::iEventSequence> _V_event_read_range(
                                        const EventIDT & eidFrom,
                                        const EventIDT & eidTo ) override {
        return _V_md_event_read_range( metadata(), eidFrom, eidTo );
    }
    /// Read events specified by set of indexes.
    virtual std::unique_ptr<aux::iEventSequence> _V_event_read_list(
                                const std::list<EventID> & list ) override {
        return _V_md_event_read_list( metadata(), list );
    }

    iBatchEventSource() :
                Parent( 0x0 ),
                _raMDatCache(nullptr) {}

    iBatchEventSource( MetadataDictionary<EventID> & mdtDictRef ) :
                Parent( mdtDictRef, 0x0 ),
                _raMDatCache(nullptr) {}
public:
    virtual ~iBatchEventSource() {
        if( _raMDatCache ) {
            delete _raMDatCache;
        }
    }

    /// Obtain (fetch cached or build new) metadata for itself.
    virtual const SpecificMetadata & metadata() {
        if( !_raMDatCache ) {
            const iSpecificMetadataType & mdt = 
                        Parent
                        ::metadata_types_dict()
                        .template get_metadata_type<SpecificMetadata>();
            _raMDatCache = &( mdt.acquire_metadata( *this ) );
        }
        return *_raMDatCache;
    }
};  // class iBatchEventSource

}  // namespace sV

# endif  // H_STROMA_V_BATCH_EVENT_SOURCE_H

