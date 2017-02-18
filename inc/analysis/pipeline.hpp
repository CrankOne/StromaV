/*
 * Copyright (c) 2016 Renat R. Dusaev <crank@qcrypt.org>
 * Author: Renat R. Dusaev <crank@qcrypt.org>
 * Author: Bogdan Vasilishin <togetherwith@gmail.com>
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

# ifndef H_STROMA_V_ANALYSIS_PIPELINE_H
# define H_STROMA_V_ANALYSIS_PIPELINE_H

# include "../config.h"

# ifdef RPC_PROTOCOLS

# include "app/mixins/protobuf.hpp"
# include "uevent.hpp"

# include <unordered_map>
# include <unordered_set>

namespace sV {

// FWD
namespace aux {
/// Data format reader object representation.
class iEventSequence;
/// Event processor reader object representation.
class iEventProcessor;
/// Processor stub helper for concrete event processors.
template<typename EventClassT, typename PayloadT>
            class iTEventPayloadProcessor;
/// Aux interfacing class implementing pushing hooks.
class iEventPayloadProcessorBase;
}  // namespace aux

/**@class AnalysisPipeline
 * @brief Representation for data analysis pipeline.
 *
 * The pipeline receives the data from object defined as a data sequence and
 * then performs sequentional operations defined in handlers. Handlers may or
 * may not change the data object and accumulate statistics.
 *
 * The pipeline can be configured at the run-time by dynamic registering data
 * source and handlers.
 *
 * Data source and handlers have to be configured outside of this class.
 *
 * The lifetime of data source and handlers are not maintained by instance of
 * this class.
 * */
class AnalysisPipeline {
public:
    typedef typename mixins::PBEventApp::UniEvent Event;
    typedef aux::iEventSequence iEventSequence;
    typedef aux::iEventProcessor iEventProcessor;
    typedef aux::iEventPayloadProcessorBase iEventPayloadProcessorBase;
protected:
    /// Pointer to data source.
    iEventSequence * _evSeq;
    /// List of handlers.
    std::list<iEventProcessor *> _processorsChain;

private:
    std::set<void (*)()>         _invalidators;
    std::set<void (*)(Event*)> _payloadPackers;
protected:
    void register_packing_functions( void(*invalidator)(),
                                     void(*packer)(Event*) );
    virtual int _process_chain( Event * );
    virtual void _finalize_event( Event * );
    virtual void _finalize_sequence( iEventSequence * );
public:
    AnalysisPipeline();
    virtual ~AnalysisPipeline() {}  // todo?

    /// Adds processor to processor chain.
    void push_back_processor( iEventProcessor * );

    /// Processor chain getter.
    std::list<iEventProcessor *> & processors()
        { return _processorsChain; }

    /// Current event sequence getter.
    iEventSequence * event_sequence()
        { return _evSeq; }

    /// Evaluates pipeline on the single event. If event was denied,
    /// returns the ordering number of processor which did the discrimination
    /// starting from 1. 0 is returned if event passed.
    virtual int process( Event * );

    /// Evaluates pipeline on the sequence. If no errors occured, returns 0.
    virtual int process( iEventSequence * );

    template<typename EventClassT, typename PayloadT>
    friend class aux::iTEventPayloadProcessor;
};  // class AnalysisPipeline

namespace aux {
/**@class AnalysisApplication::iEventSequence
 *
 * Data format reader object representation.
 * */
class iEventSequence {
public:
    typedef AnalysisPipeline::Event Event;
    typedef uint8_t Features_t;
    enum Features
    # ifndef SWIG
        : Features_t
    # endif
    {
        randomAccess = 0x1,
        identifiable = 0x2,
    };
private:
    uint8_t _features;
protected:
    virtual bool _V_is_good() = 0;
    virtual void _V_next_event( Event *& ) = 0;
    /// Has to return a pointer to a valid event. Can invoke _V_next_event()
    /// internally.
    virtual Event * _V_initialize_reading() = 0;
    virtual void _V_finalize_reading() = 0;
    virtual void _V_print_brief_summary( std::ostream & ) const {}
    iEventSequence( Features_t fts );
public:
    virtual ~iEventSequence(){}

    /// Returns true if source initialized and next event can be read.
    virtual bool is_good() {     return _V_is_good(); }

    /// Set-ups reading procedure.
    virtual Event * initialize_reading() { return _V_initialize_reading(); }

    /// Performs reading next event into provided reentrant instance.
    virtual void next_event( Event *& e ) {         _V_next_event( e ); }

    /// Invoked after event reading done (e.g. to clean-up file descriptor).
    virtual void finalize_reading( ) {   _V_finalize_reading(); }

    /// Prints out brief statistics of current state.
    virtual void print_brief_summary( std::ostream & os ) const
                                    { _V_print_brief_summary( os ); }

    /// Returns features mask.
    Features_t features() const { return _features; }

    /// Returns true if descendant implements iRandomAccessEventSource.
    bool does_provide_random_access() const
                { return _features & randomAccess; }

    /// Returns true if descendant implements iIdentifiableEventSource.
    bool is_identifiable() const
                { return _features & identifiable; }

    friend class ::sV::AnalysisPipeline;
};

/**@class AnalysisApplication::iEventProcessor
 *
 * Event processing handler class interface. This class claims the basic logic.
 * */
class iEventProcessor {
public:
    typedef AnalysisPipeline::Event Event;
    enum ProcessorFeatures : uint8_t {
        desiresMetadata = 0x1,  ///< Set for proc-s which may work without mdat.
        requiresMetadata  = 0x3,  ///< Set for processors which do require mdat.
        supportsExperimental = 0x4,
        supportsSimulated = 0x8,
    };
private:
    const std::string _pName;
    const uint8_t _pFeatures;
protected:
    /// Should return 'false' if processing in chain should be aborted.
    virtual bool _V_process_event( Event * ) = 0;
    /// Called after single event processed by all the processors.
    virtual void _V_finalize_event_processing( Event * ) {}
    /// Called after all events read and source closed to cleanup statistics.
    virtual void _V_finalize() const {}
    /// Called after all events read and all processors finalized.
    virtual void _V_print_brief_summary( std::ostream & ) const {}

    // This ctr is introduced only to avoid many of the repeatative
    // initializations and should be never called directly. If you've got
    // the linkage error referring to this ctr, check the foremost descentant
    // of the inheritance chain started from iEventProcessor. This descentant
    // must call the public ctr with all its arguments explicitly in its ctr.
    // This is how virtual inheritance works in C++.
    /// Shortcut ctr for virtual inheritance.
    iEventProcessor();  // Must not be implemented.
public:
    iEventProcessor( const std::string & pn, uint8_t pFts ) :
                    _pName(pn), _pFeatures(pFts) {}
    virtual ~iEventProcessor(){}
    virtual bool process_event( Event * e ) { return _V_process_event( e ); }
    virtual void finalize_event( Event * e )
                                    { _V_finalize_event_processing( e ); }
    virtual void print_brief_summary( std::ostream & os ) const
                                    { _V_print_brief_summary( os ); }
    virtual void finalize() const { _V_finalize(); }
    const std::string & processor_name() const { return _pName; }

    virtual bool operator()( Event * e ) { return process_event( e ); }

    friend class ::sV::AnalysisPipeline;
};

class iEventPayloadProcessorBase : public virtual iEventProcessor {
public:
    typedef AnalysisPipeline::Event Event;
    iEventPayloadProcessorBase( const std::string & pn ) :
                                                        iEventProcessor(pn) {}
protected:
    /// Must be overriden by payload processor.
    virtual void register_hooks( AnalysisPipeline * ) = 0;

    friend class ::sV::AnalysisPipeline;
};  // class AnalysisPipeline::iEventPayloadProcessorBase

/**@class iEventMetadataNeedProcessor
 * @brief Aux interfacing processor class requiring knowledge of the metadata
 *        associated to the current event source. */
template<typename MetadataT>
class iEventMetadataNeedProcessor : public virtual iEventProcessor {
protected:
    const MetadataT * _mdatPtr;
public:
    iEventMetadataNeedProcessor( const std::string & pn ) :
                                            _mdatPtr(nullptr) {}
    virtual ~iEventMetadataNeedProcessor(){}

    bool is_metadata_set() const { return _mdatPtr; }
    void metadata( const MetadataT & mdatRef ) { _mdatPtr = &mdatRef; }
    void reset_metadata() { _mdatPtr = nullptr; }
    const MetadataT & metadata() const {
        if( !is_metadata_set() ) {
            emraise( badState, "Metadata not set for event processor "
                "instance %p when it was required.", this );
        }
        return *_mdatPtr;
    }
};

/**@class AnalysisApplication::iTEventPayloadProcessor
 *
 * This template base for processors incapsulates (re)caching logic for
 * concrete data payload instances inside event messages. Since gprotobuf3
 * does not provide any extension/inheritance mechanics, we have to implement
 * some helper code here.
 * */
template<typename EventClassT,
         typename PayloadT>
class iTEventPayloadProcessor : public iEventPayloadProcessorBase {
public:
    typedef AnalysisPipeline::Event Event;
protected:
    /// Reentrant static field. Should be initialized to NULL by analysis
    /// application at the beginning of each new event.
    static PayloadT * _reentrantPayloadPtr;
public:
    /// Must be called at the beginning of each new event by management class.
    static void nullate_cache() { _reentrantPayloadPtr = nullptr; }
protected:
    /// Will be called if current event has payload of required type.
    static void (*unpack_payload)( Event * );
    /// Will be called at the end of event processing pipeline.
    static void (*pack_payload)( Event * );
protected:
    iTEventPayloadProcessor( const std::string & pn ) :
                            iEventPayloadProcessorBase(pn) {}

    /// Should return 'false' if processing in chain has to be aborted.
    virtual bool _V_process_event( Event * uEventPtr ) override {
        if( !_reentrantPayloadPtr ) {
            assert(unpack_payload);
            unpack_payload( uEventPtr );
        }
        return _V_process_event_payload( _reentrantPayloadPtr );
    }

    virtual void register_hooks( AnalysisPipeline * ppl ) final {
        assert( pack_payload );
        ppl->register_packing_functions( nullate_cache,
                                         pack_payload );
    }

    /// One has to implement all the payload processing here.
    virtual bool _V_process_event_payload( PayloadT * ) = 0;
public:
    /// Helper method registering pack/unpack caching functions. Invoked by
    /// pipeline
    friend class ::sV::AnalysisPipeline;
};  // class iTEventPayloadProcessor

template<typename EventClassT,
         typename PayloadT>
PayloadT * iTEventPayloadProcessor<EventClassT, PayloadT>
                                            ::_reentrantPayloadPtr = nullptr;

template<typename EventClassT,
         typename PayloadT>
void (*iTEventPayloadProcessor<EventClassT, PayloadT>::unpack_payload)
                                    ( AnalysisPipeline::Event * ) = nullptr;

template<typename EventClassT,
         typename PayloadT>
void (*iTEventPayloadProcessor<EventClassT, PayloadT>::pack_payload)
                                    ( AnalysisPipeline::Event * ) = nullptr;


template<typename PayloadT>
class iTExperimentalEventPayloadProcessor :
        public iTEventPayloadProcessor<events::ExperimentalEvent, PayloadT> {
public:
    typedef AnalysisPipeline::Event Event;
    typedef iTEventPayloadProcessor<events::ExperimentalEvent, PayloadT> Parent;
private:
    /// Will be called if current event has payload of required type.
    static void _unpack_payload( Event * uEventPtr ) {
        Parent::_reentrantPayloadPtr = new PayloadT();
        uEventPtr->mutable_experimental()
                 ->mutable_payload()
                 ->UnpackTo(Parent::_reentrantPayloadPtr);
    }
    /// Will be called at the end of event processing pipeline.
    static void _pack_payload( Event * uEventPtr ) {
        uEventPtr->mutable_experimental()
                 ->mutable_payload()
                 ->PackFrom(*Parent::_reentrantPayloadPtr);
        delete Parent::_reentrantPayloadPtr;
        Parent::_reentrantPayloadPtr = nullptr;
    }
protected:
    iTExperimentalEventPayloadProcessor( const std::string & pn ) :
                            Parent(pn) {
        if( !Parent::unpack_payload ) {
            Parent::unpack_payload = _unpack_payload;
        }
        if( !Parent::pack_payload ) {
            Parent::pack_payload = _pack_payload;
        }
    }
};  // class iExperimentalEventPayloadProcessor

}  // namespace aux

}  // namespace sV

# endif  // RPC_PROTOCOLS
# endif  //H_STROMA_V_ANALYSIS_PIPELINE_H

