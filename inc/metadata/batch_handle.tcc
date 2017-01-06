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
    private:
        Self & _handleRef;
        ReadingMarkup * _markup;
    protected:
        ProxyRangeSequence( Self & self, ReadingMarkup * markupPtr ) :
                    aux::iEventSequence( aux::iEventSequence::randomAccess ),
                    _handleRef(self), _markup(markupPtr) {}
        ~ProxyRangeSequence() {
            delete _markup;
        }
        virtual bool _V_is_good() override {
            _TODO_  // TODO
        }
        virtual void _V_next_event( Event *& ) override {
            _TODO_  // TODO
        }
        virtual Event * _V_initialize_reading() override {
            _TODO_  // TODO
        }
        virtual void _V_finalize_reading() override {
            _TODO_  // TODO
        }
        friend class BatchEventsHandle<EventIDT, MetadataT, SourceIDT>;
    };
private:
    iMetadataType & _mdt;
    void _assert_mdt_non_empty() {
        if( _mdt.stores().empty() ) {
            emraise( badState, "No stores associated with cached metadata "
                     "type \"%s\" (id:%#x, ptr:%p). Unable to retreive "
                     "cached metadata.", _mdt.name().c_str(),
                     _mdt.type_index(), &_mdt );
        }
    }
    Event _reentrantSingleEvent;
public:
    BatchEventsHandle( iMetadataType & mdt ) :
                                                    _mdt(mdt) {}
    virtual ~BatchEventsHandle() {}

    /// Note that returned event ptr refers to internal reentrant instance and
    /// will be re-written on next invokation of this method. Lifetime of
    /// this instance is restricted by lifetime of owning iCachedMetadataType
    /// instance.
    virtual Event * event_read_single( const EventID & eid ) override {
        # if 1
        _TODO_  // TODO
        # else
        _assert_mdt_non_empty();
        SourceID sid;
        bool found = false;
        for( auto storePtr : _mdt.stores() ) {
            if( (found = storePtr->source_id_for( eid, sid )) ) {
                break;
            }
        }
        if( !found ) {
            emraise( noSuchKey, "Unable to locate source containing event "
                                "with specified ID." );
        }
        iEventSource * evSourcePtr = nullptr;
        iMetadataStore * owningStore = nullptr;
        for( auto storePtr : _mdt.stores() ) {
            if( nullptr != (evSourcePtr = storePtr->source(sid)) ) {
                owningStore = storePtr;
                break;
            }
        }
        if( !owningStore ) {
            emraise( badState, "Event with specified ID located, but none of "
                "available metadata caching store instances can provide "
                "accessible handle to source." );
        }
        // todo: may optimize it a little by using direct querying of metadata
        // here:
        //Event * eventReadPtr = evSourcePtr->_md_event_read_single( eid );
        Event * eventReadPtr = evSourcePtr->event_read_single( eid );
        _reentrantSingleEvent.CopyFrom( *eventReadPtr );
        owningStore->free_source(evSourcePtr);
        return &_reentrantSingleEvent;
        # endif
    }

    virtual std::unique_ptr<aux::iEventSequence> event_read_range(
                                const EventID & lower,
                                const EventID & upper ) override {
        # if 1
        _TODO_  // TODO
        # else
        _assert_mdt_non_empty();
        std::list<SourceID> sids;
        for( auto storePtr : _mdt.stores() ) {
            storePtr->collect_source_ids_for_range( lower, upper, sids );
        }

        // Keeps ranges corresponding to particular sources.
        auto rmuPtr = new typename ProxyRangeSequence::ReadingMarkup();

        for( const auto & sid : sids ) {
            SubrangeMarkup entry;
            _mdt.get_subrange( lower, upper, sid, entry );
            rmuPtr->push_back( entry );
        }

        return std::unique_ptr<aux::iEventSequence>(
                                    new ProxyRangeSequence( *this, rmuPtr ));
        # endif
    }

    virtual std::unique_ptr<aux::iEventSequence> event_read_list(
                                const std::list<EventID> & list ) override {
        _assert_mdt_non_empty();
        _TODO_  // TODO
    }
    
};  // class BatchEventsHandle

}  // namespace aux

}  // namespace sV

# endif  // H_STROMA_V_METADATA_BATCH_HANDLE_H

