/*
 * Copyright (c) 2017 Renat R. Dusaev <crank@qcrypt.org>
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

# ifndef H_STROMA_V_METADATA_BATCH_HANDLE_H
# define H_STROMA_V_METADATA_BATCH_HANDLE_H

namespace sV {

# include "store.tcc"
# include "analysis/evSource_RA.tcc"
# include <cstdint>

namespace aux {

/**@class BatchEventsHandle
 * @brief Auxilliary class representing a subset all of iSectionalEventSource
 *        instances of particular type.
 * 
 * This interim class is usually instantiated by the internals of
 * iCachedMetadataType in order to provide unified look-up mechanics among
 * available stores by providing a proxy instance referring to metadata type's
 * stores list. User can consider it as a one humongous data source containing
 * all available events physically distributed among multiple artifacts. User
 * probably will never instantiate it directly but rather obtaine a reference
 * to it via the `batch_handle()` method of corresponding metadata type.
 *
 * @see IRandomAccessEventStream
 */
template<typename EventIDT,
         typename MetadataT,
         typename SourceIDT>
class BatchEventsHandle : public IRandomAccessEventStream<EventIDT> {
public:
    sV_METADATA_IMPORT_SECT_TRAITS( EventIDT, MetadataT, SourceIDT );
    
    # if 0
    typedef typename Traits::iSpecificMetadataStore MetadataStore;
    typedef typename Traits::iSpecificMetadataType iSpecificCachedMetadataType;
    typedef typename Traits::iEventSource iEventSource;
    typedef typename Traits::SubrangeMarkup SubrangeMarkup;
    # endif

    typedef sV::events::Event Event;

    typedef BatchEventsHandle<EventID, Metadata, SourceID> Self;
protected:
    /// Internal helper class routing events iteration in range.
    class ProxyRangeSequence : aux::iEventSequence {
    public:
        typedef std::list<SubrangeMarkup> ReadingMarkup;
        typedef typename Traits::iDisposableSourceManager Manager;
        typedef std::list<Manager *> Managers;
    private:
        iEventSource * _cEvSrc;
        std::unique_ptr<iEventSequence> _rangeReadingSrc;
        Managers & _mngrs;
        typename Managers::iterator _evSrcOwnerIt;

        ReadingMarkup * _markup;
        typename ReadingMarkup::iterator _rmuIt;
    protected:
        Event * _dispose_source() {
            _evSrcOwnerIt = _mngrs.end();
            // TODO: use existing source if available in _rmuIt
            for( auto mngrIt = _mngrs.begin();
                 _mngrs.end() != mngrIt; ++mngrIt ) {
                if( (_cEvSrc = (*mngrIt)->source(_rmuIt->sid) ) ) {
                    _evSrcOwnerIt = mngrIt;
                    // TODO: use acquired MD instance if available in _rmuIt
                    _rangeReadingSrc = _cEvSrc->event_read_range(
                                                    _rmuIt->from, _rmuIt->to );
                    return _rangeReadingSrc->initialize_reading();
                }
            }
            emraise( badState, "Proxy event source instance could not "
                "acquire disposable event source among %zu stores.",
                _mngrs.size() );
        }

        Event * _dispose_next_source() {
            if( _cEvSrc ) {
                // Free current source.
                (*_evSrcOwnerIt)->free_source(_cEvSrc);
                _cEvSrc = nullptr;
                _evSrcOwnerIt = _mngrs.end();
            }
            ++_rmuIt;
            if( _rmuIt != _markup->end() ) {
                return _dispose_source();
            } else {
                return nullptr;
            }
        }

        ProxyRangeSequence( Event & evRef,
                            Managers & mngrs,
                            ReadingMarkup * markupPtr ) :
                    aux::iEventSequence( 0x0 ),
                    _cEvSrc(nullptr),
                    _mngrs(mngrs),
                    _evSrcOwnerIt(mngrs.end()),
                    _markup(markupPtr),
                    _rmuIt(markupPtr->end()) {}
        ~ProxyRangeSequence() {
            delete _markup;
        }

        virtual bool _V_is_good() override {
            return _rmuIt != _markup->end();
        }

        virtual void _V_next_event( Event *& evPtrRef ) override {
            _rangeReadingSrc->next_event( evPtrRef );
            if( !_rangeReadingSrc->is_good() ) {  // sic!
                _rangeReadingSrc->finalize_reading();
                evPtrRef = _dispose_next_source();
            }
        }

        virtual Event * _V_initialize_reading() override {
            _rmuIt = _markup->begin();
            return _dispose_source();
        }

        virtual void _V_finalize_reading() override {
            if( _cEvSrc ) {
                // Free current source.
                (*_evSrcOwnerIt)->free_source(_cEvSrc);
                _cEvSrc = nullptr;
                _evSrcOwnerIt = _mngrs.end();
            }
        }
        friend class BatchEventsHandle<EventIDT, MetadataT, SourceIDT>;
    };
private:
    iMetadataType & _mdt;
    Event _reentrantSingleEvent;
public:
    BatchEventsHandle( iMetadataType & mdt ) : _mdt(mdt) {}
    virtual ~BatchEventsHandle() {}

    /// Note that returned event ptr refers to internal reentrant instance and
    /// will be re-written on next invokation of this method. Lifetime of
    /// this instance is restricted by lifetime of owning iCachedMetadataType
    /// instance.
    virtual Event * event_read_single( const EventID & eid ) override {
        if( _mdt._singleEventQueryables.empty() ) {
            emraise( badState, "No stores associated with cached metadata "
                     "type \"%s\" (id:%#x, ptr:%p) which could perform single "
                     "event look-up. Unable to retreive source ID.",
                     _mdt.name().c_str(),
                     _mdt.type_index(), &_mdt );
        }
        if( _mdt._dspSrcMngrs.empty() ) {
            emraise( badState, "No stores associated with cached metadata "
                     "type \"%s\" (id:%#x, ptr:%p) which could perform "
                     "proxying of events reading (creation of disposable event "
                     "source). Unable to retreive event by ID.",
                     _mdt.name().c_str(),
                     _mdt.type_index(), &_mdt );
        }

        SourceID sid;
        bool found = false;
        for( auto storePtr : _mdt._singleEventQueryables ) {
            if( (found = storePtr->source_id_for( eid, sid )) ) {
                break;
            }
        }
        if( !found ) {
            emraise( noSuchKey, "Unable to find source containing event "
                                "with specified ID." );
        }
        iEventSource * evSourcePtr = nullptr;
        typename Traits::iDisposableSourceManager * owningStore = nullptr;
        for( auto storePtr : _mdt._dspSrcMngrs ) {
            if( nullptr != (evSourcePtr = storePtr->source(sid)) ) {
                owningStore = storePtr;
                break;
            }
        }
        if( !owningStore ) {
            emraise( badState, "Event with specified ID located, but none of "
                "available metadata caching store instances were able to "
                " provide disposable streaming instance." );
        }
        // todo: may optimize it a little by using direct querying of metadata
        // here:
        //Event * eventReadPtr = evSourcePtr->_md_event_read_single( eid );
        Event * eventReadPtr = evSourcePtr->event_read_single( eid );
        _reentrantSingleEvent.CopyFrom( *eventReadPtr );
        owningStore->free_source(evSourcePtr);
        return &_reentrantSingleEvent;
    }

    virtual std::unique_ptr<aux::iEventSequence> event_read_range(
                                const EventID & lower,
                                const EventID & upper ) override {
        if( _mdt._rangeQueryables.empty() ) {
            emraise( badState, "No stores associated with cached metadata "
                     "type \"%s\" (id:%#x, ptr:%p) which could perform "
                     "querying of events sub-ranges appropriate to particular "
                     "sectioned source instances. Unable to retreive events "
                     "sub-ranges.",
                     _mdt.name().c_str(),
                     _mdt.type_index(), &_mdt );
        }
        if( _mdt._dspSrcMngrs.empty() ) {
            emraise( badState, "No stores associated with cached metadata "
                     "type \"%s\" (id:%#x, ptr:%p) which could perform "
                     "proxying of events reading (creation of disposable event "
                     "source). Unable to retreive events by ID range.",
                     _mdt.name().c_str(),
                     _mdt.type_index(), &_mdt );
        }
        // Keeps ranges corresponding to particular sources.
        auto rmuPtr = new typename ProxyRangeSequence::ReadingMarkup();

        for( auto storePtr : _mdt._rangeQueryables ) {
            storePtr->collect_source_ids_for_range( lower, upper, *rmuPtr );
        }

        return std::unique_ptr<aux::iEventSequence>(
                new ProxyRangeSequence( _reentrantSingleEvent,
                                        _mdt._dspSrcMngrs,
                                        rmuPtr ));
    }

    virtual std::unique_ptr<aux::iEventSequence> event_read_list(
                                const std::list<EventID> & list ) override { 
        _TODO_  // TODO
    }
    
};  // class BatchEventsHandle

}  // namespace aux

}  // namespace sV

# endif  // H_STROMA_V_METADATA_BATCH_HANDLE_H

