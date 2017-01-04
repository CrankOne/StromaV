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

# ifndef H_STROMA_V_RANDOM_ACCESS_EVENT_SOURCE_H
# define H_STROMA_V_RANDOM_ACCESS_EVENT_SOURCE_H

# include "pipeline.hpp"
# include "metadata/type.tcc"

namespace sV {

namespace aux {

/**@class iRandomAccessEventSource
 * @brief An interim interface providing random access mthods.
 *
 * User code has to define particular metadata application routines.
 * */
template<typename EventIDT,
         typename SpecificMetadataT>
class iRandomAccessEventSource : virtual public iEventSequence {
public:
    typedef EventIDT EventID;
    typedef SpecificMetadataT SpecificMetadata;
    typedef iMetadataType<EventID, SpecificMetadata> iSpecificMetadataType;
    typedef iEventSequence::Event Event;
private:
    bool _ownsDict;
    MetadataDictionary<EventIDT> * _mdtDictPtr;
    const SpecificMetadata * _raMDatCache;

    iRandomAccessEventSource(
                MetadataDictionary<EventID> * mdtDictPtr,
                bool ownsDict,
                iEventSequence::Features_t fts ) :
                        iEventSequence( fts | iEventSequence::randomAccess ),
                        _ownsDict( ownsDict ),
                        _mdtDictPtr( mdtDictPtr ),
                        _raMDatCache(nullptr) {}
protected:
    virtual const SpecificMetadata * _V_acquire_my_metadata() = 0;

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
    iRandomAccessEventSource( iEventSequence::Features_t fts ) :
                        iRandomAccessEventSource(
                                    new MetadataDictionary<EventID>(),
                                    true,
                                    fts ) {}
    iRandomAccessEventSource( MetadataDictionary<EventID> & mdtDictRef,
                              iEventSequence::Features_t fts ) :
                        iRandomAccessEventSource(
                                    &mdtDictRef,
                                    false,
                                    fts ) {}
public:
    virtual ~iRandomAccessEventSource() {
        if( _ownsDict && _mdtDictPtr ) {
            delete _mdtDictPtr;
        }
    }
    virtual Event * event_read_single( const EventID & eid ) {
        return _V_md_event_read_single( metadata(), eid ); }

    virtual std::unique_ptr<iEventSequence> event_read_range(
                                    const EventID & lower,
                                    const EventID & upper ) {
        return _V_md_event_read_range( metadata(), lower, upper ); }

    virtual std::unique_ptr<iEventSequence> event_read_list(
                                    const std::list<EventID> & list ) {
        return _V_md_event_read_list(metadata(), list); }

    MetadataDictionary<EventIDT> & metadata_types_dict() { return *_mdtDictPtr; }

    /// Obtain (fetch cached or build new) metadata for itself.
    virtual const SpecificMetadata & metadata() {
        if( !_raMDatCache ) {
            _raMDatCache = _V_acquire_my_metadata();
        }
        return *_raMDatCache;
    }
};  // iRandomAccessEventSource

}  // namespace aux

}  // namespace sV

# endif  // H_STROMA_V_RANDOM_ACCESS_EVENT_SOURCE_H

