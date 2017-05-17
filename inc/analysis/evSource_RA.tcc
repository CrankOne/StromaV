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

/**@struct IRandomAccessEventStream
 * @brief An interface for random access to events stream.
 *
 * Note, that the class implementing this interface has also to provide
 * implementation of a query result --- a descendant of the iEventSequence
 * for methods returning multiple events. These events may or may not be
 * located in memory at once.
 *
 * This struct is a pure interface.
 *
 * @ingroup analysis
 * @ingroup mdat
 *
 * @see iEventSequence
 */
template<typename EventIDT>
struct IRandomAccessEventStream {
    virtual aux::iEventSequence::Event * event_read_single(
                                    const EventIDT & eid ) = 0;

    virtual std::unique_ptr<aux::iEventSequence> event_read_range(
                                    const EventIDT & lower,
                                    const EventIDT & upper ) = 0;
    virtual std::unique_ptr<aux::iEventSequence> event_read_list(
                                    const std::list<EventIDT> & list ) = 0;

    virtual ~IRandomAccessEventStream() {}
};

namespace aux {

/**@class iRandomAccessEventSource
 * @brief An interim interface providing random access mthods.
 *
 * User code has to define particular metadata application routines.
 * @see IRandomAccessEventStream
 *
 * @ingroup analysis
 * @ingroup mdat
 * */
template<typename EventIDT,
         typename MetadataT>
class iRandomAccessEventSource : virtual public iEventSequence,
                                 public IRandomAccessEventStream<EventIDT> {
public:
    typedef EventIDT EventID;
    typedef MetadataT Metadata;
    typedef iTMetadataType<EventID, Metadata> iSpecificMetadataType;
    typedef iEventSequence::Event Event;
private:
    bool _ownsDict;
    MetadataDictionary<EventIDT> * _mdtDictPtr;
    const Metadata * _raMDatCache;

    iRandomAccessEventSource(
                MetadataDictionary<EventID> * mdtDictPtr,
                bool ownsDict,
                iEventSequence::Features_t fts ) :
                        iEventSequence( fts | iEventSequence::randomAccess ),
                        _ownsDict( ownsDict ),
                        _mdtDictPtr( mdtDictPtr ),
                        _raMDatCache(nullptr) {}
protected:
    virtual const Metadata * _V_acquire_my_metadata() = 0;

    /// This abstract metod has to read the event using metadata instance (IF).
    virtual Event * _V_md_event_read_single( const Metadata & md,
                                             const EventIDT & eid ) = 0;

    /// (IF) This abstract method has to read range of events using metadata
    /// instance.
    virtual std::unique_ptr<aux::iEventSequence> _V_md_event_read_range(
                                        const Metadata & md,
                                        const EventIDT & eidFrom,
                                        const EventIDT & eidTo ) = 0;
    /// (IF) Has to read a list of events in given order.
    virtual std::unique_ptr<aux::iEventSequence> _V_md_event_read_list(
                                    const Metadata & md,
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
    virtual Event * event_read_single( const EventID & eid ) override {
        return _V_md_event_read_single( metadata(), eid ); }

    virtual std::unique_ptr<iEventSequence> event_read_range(
                                const EventID & lower,
                                const EventID & upper ) override {
        return _V_md_event_read_range( metadata(), lower, upper ); }

    virtual std::unique_ptr<iEventSequence> event_read_list(
                                const std::list<EventID> & list ) override {
        return _V_md_event_read_list(metadata(), list); }

    MetadataDictionary<EventIDT> & metadata_types_dict() { return *_mdtDictPtr; }

    /// Obtain (fetch cached or build new) metadata for itself.
    virtual const Metadata & metadata() {
        if( !_raMDatCache ) {
            _raMDatCache = _V_acquire_my_metadata();
        }
        return *_raMDatCache;
    }
};  // iRandomAccessEventSource

}  // namespace aux

}  // namespace sV

# endif  // H_STROMA_V_RANDOM_ACCESS_EVENT_SOURCE_H

