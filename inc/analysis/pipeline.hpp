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

# include "sV_config.h"

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
 * The analysis pipeline is merely a container for a sequence of handlers
 * designed to process individual event.
 *
 * The handlers are represented by a
 * subclasses of iEventProcessor. The common functions of data sources are
 * summirized within iEventSequence base class.
 *
 * The pipeline receives the data from source (iEventSequence or per individual
 * events basis) and then performs sequentional operations defined in stacked
 * handlers. Handlers may or may not change the data object and accumulate
 * various side statistics.
 *
 * The pipeline can be configured at the run-time by dynamic registering data
 * source and handlers.
 *
 * Data source and handlers have to be configured outside of this class.
 *
 * The lifetime of data source and handlers are not maintained by this class.
 *
 * The particular responsibility of AnalysisPipeline is to track consistency
 * of passing data and accumulate some common statistics: number of events
 * read, processed by individual processors, number of events discriminated by
 * particular processors and so on.
 *
 * Considering the event representation level, the pipeline also ensures that
 * event data payload caches are also properly [un]packed during processing.
 *
 * @ingroup analysis
 * */
class AnalysisPipeline : public sV::AbstractApplication::ASCII_Entry {
public:
    typedef typename mixins::PBEventApp::UniEvent Event;
    typedef aux::iEventSequence iEventSequence;
    typedef aux::iEventProcessor iEventProcessor;
    typedef aux::iEventPayloadProcessorBase iEventPayloadProcessorBase;

    /// The entry referencing particular processor once being stacked up with
    /// others becomes a handler.
    class Handler {
    public:
        struct Statistics {
            size_t nConsidered
                 , nDiscriminated
                 , nAborted
                 ;
            Statistics() :  nConsidered(0), nDiscriminated(0), nAborted(0) {}
        };
    private:
        iEventProcessor & _processor;
        Statistics _stats;
        struct PayloadTraits {
            const std::type_info & TI;
            bool forcePack;
            // ...
            PayloadTraits( const std::type_info & TI_ ) :
                        TI(TI_), forcePack(false) {}
        } * _payloadTraits;
    public:
        const Statistics & stats() { return _stats; }
        bool payload_traits_available() const { return !!_payloadTraits; }
        PayloadTraits & payload_traits();
        Handler( iEventProcessor & processor_ );
        iEventProcessor & processor() { return _processor; }
        const iEventProcessor & processor() const { return _processor; }
        ~Handler();
    };
    typedef std::list<Handler> Chain;
protected:
    /// List of handlers.
    Chain _processorsChain;
private:
    std::set<void (*)()> _invalidators;
    std::set<void (*)(Event&)> _payloadPackers;

    size_t _nEventsAcquired;
    /// ASCII_Entry-related internal function.
    void _update_stat();
protected:
    void register_packing_functions( void(*invalidator)(),
                                     void(*packer)(Event&) );
    virtual int _process_chain( Event & );
    virtual void _finalize_event( Event & );
    virtual void _finalize_sequence( iEventSequence & );
public:
    AnalysisPipeline();
    virtual ~AnalysisPipeline() {}  // todo?

    /// Adds processor to processor chain.
    void push_back_processor( iEventProcessor & );

    /// Processor chain getter.
    const Chain & processors() const
        { return _processorsChain; }

    /// Evaluates pipeline on the single event. If event was denied,
    /// returns the ordering number of processor which did the discrimination
    /// starting from 1. 0 is returned if event passed.
    virtual int process( Event & );

    /// Evaluates pipeline on the sequence. If no errors occured, returns 0.
    virtual int process( iEventSequence & );

    template<typename EventClassT, typename PayloadT>
    friend class aux::iTEventPayloadProcessor;
};  // class AnalysisPipeline

namespace aux {
/**@class AnalysisApplication::iEventSequence
 *
 * Data format reader object representation.
 *
 * @ingroup analysis
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
 *
 * @ingroup analysis
 * */
class iEventProcessor {
public:
    typedef AnalysisPipeline::Event Event;
    /// Possible result flags and composite shortcuts that may be returned by
    /// procedure. Note, that flags itself may be counter-intuitive, so better
    /// stand for shortcuts (the have RC_ prefix).
    enum ProcessingResultFlags : int8_t {
        CONTINUE_PROCESSING = 0x1,  ///< when set, all the analysis has to be interrupt
        ABORT_CURRENT       = 0x2,  ///< processing of current event or payload has to be interrupt
        DISCRIMINATE        = 0x4,  ///< processing continues, but the current event/payload has to be considered as discriminated
        NOT_MODIFIED        = 0x10  ///< the event or data payload wasn't modified
        // shortcuts:
        /// For invasive processor task (e.g. reconstruction, applying calibration)
        , RC_CORRECTED                      = CONTINUE_PROCESSING
        /// For non-invasive processors (e.g. stats accumulators)
        , RC_ACCOUNTED                      = CONTINUE_PROCESSING | NOT_MODIFIED
        /// For invasive processor revealed cut condition (event still valid!)
        , RC_DISCRIMINATE_CORRECTED         = CONTINUE_PROCESSING | DISCRIMINATE
        /// For non-invasive cutter, to exclude event from physical analysis
        , RC_DISCRIMINATE                   = CONTINUE_PROCESSING | DISCRIMINATE  | NOT_MODIFIED
        /// May be useful for invasive data look-up procedures (xxx?)
        , RC_DONE                           = CONTINUE_PROCESSING | ABORT_CURRENT
        /// Useful for non-invasive data look-up procedures
        , RC_ABORT_CURRENT                  = CONTINUE_PROCESSING | ABORT_CURRENT | NOT_MODIFIED
        /// Invasive processor encountered invalid event that shouldn't be analysed further
        , RC_ABORT_DISCRIMINATE_CORRECTED   = CONTINUE_PROCESSING | DISCRIMINATE
        /// Non-invasive processor encountered invalid event that shouldn't be analysed further
        , RC_ABORT_DISCRIMINATE             = CONTINUE_PROCESSING | DISCRIMINATE  | NOT_MODIFIED
    };
    typedef int8_t ProcRes;

    /// Returns true if current processing has to be aborted.
    static bool consider_interim_result( ProcRes sub, ProcRes & current );
private:
    const std::string _pName;
protected:
    /// Should return 'false' if processing in chain should be aborted.
    virtual ProcRes _V_process_event( Event & ) = 0;
    /// Called after single event processed by all the processors.
    virtual ProcRes _V_finalize_event_processing( Event & ) { return RC_ACCOUNTED; }
    /// Called after all events read and source closed to cleanup statistics.
    virtual void _V_finalize() const {}
    /// Called after all events read and all processors finalized.
    virtual void _V_print_brief_summary( std::ostream & ) const {}
public:
    iEventProcessor( const std::string & pn ) : _pName(pn) {}
    virtual ~iEventProcessor(){}
    virtual ProcRes process_event( Event & e ) { return _V_process_event( e ); }
    virtual ProcRes finalize_event( Event & e )
                                { return _V_finalize_event_processing( e ); }
    virtual void print_brief_summary( std::ostream & os ) const
                                { _V_print_brief_summary( os ); }
    virtual void finalize() const { _V_finalize(); }
    const std::string & processor_name() const { return _pName; }
    virtual bool operator()( Event & e ) { return process_event( e ); }
    friend class ::sV::AnalysisPipeline;
};

class iEventPayloadProcessorBase : public iEventProcessor {
public:
    typedef AnalysisPipeline::Event Event;
    iEventPayloadProcessorBase( const std::string & pn ) :
                                                        iEventProcessor(pn) {}
protected:
    /// Must be overriden by payload processor.
    virtual void register_hooks( AnalysisPipeline * ) = 0;

    /// Has to return C++ RTTI identifier of payload type.
    virtual const std::type_info & _V_payload_type_info() const = 0;
public:
    /// Returns C++ RTTI identifier of payload type.
    const std::type_info & payload_type_info() const {
        return _V_payload_type_info(); }

    friend class ::sV::AnalysisPipeline;
};  // class AnalysisPipeline::iEventPayloadProcessorBase

/**@class AnalysisApplication::iTEventPayloadProcessor
 *
 * This template base for processors incapsulates (re)caching logic for
 * concrete data payload instances inside event messages. Since gprotobuf3
 * does not provide any extension/inheritance mechanics, we have to implement
 * some helper code here.
 *
 * @ingroup analysis
 * */
template<typename EventClassT,
         typename PayloadT>
class iTEventPayloadProcessor : public iEventPayloadProcessorBase {
public:
    typedef AnalysisPipeline::Event Event;
private:
    /// When set, forces event finalizing procedure to perform payload packing
    /// at the end. Usually set for certain instance by pipeline on
    /// push_back_processor() to force payload pack prior to further processing
    /// of an entire event.
    mutable bool _forcePayloadPack;
protected:
    /// Reentrant static field. Should be initialized to NULL by analysis
    /// application at the beginning of each new event.
    static PayloadT * _reentrantPayloadPtr;
    /// Has to return true if data of PayloadT is precent at the given event.
    /// Usually done with ->has_experimental()/->has_simulated()/etc.
    static bool (*has_payload)(const Event &);
    /// Will be called if current event has payload of required type.
    static void (*unpack_payload)( Event & );
    /// Will be called at the end of event processing pipeline.
    static void (*pack_payload)( Event & );
    /// Must be called at the beginning of each new event by management class.
    static void nullate_cache() { _reentrantPayloadPtr = nullptr; }

    /// Should return 'false' if processing in chain has to be aborted.
    virtual ProcRes _V_process_event( Event & uEvent ) override {
        if( has_payload(uEvent) ) {
            if( !_reentrantPayloadPtr ) {
                assert(unpack_payload);
                unpack_payload( uEvent );
            }
            ProcRes rs = _V_process_event_payload( *_reentrantPayloadPtr );
            if( _forcePayloadPack && !(ABORT_CURRENT & rs) ) {
                pack_payload( uEvent );
            }
            return rs;
        }
        return iEventProcessor::RC_ACCOUNTED;
    }
    /// Helper method registering pack/unpack caching functions. Invoked by
    /// pipeline
    virtual void register_hooks( AnalysisPipeline * ppl ) final {
        assert( pack_payload );
        ppl->register_packing_functions( nullate_cache,
                                         pack_payload );
    }
    /// Returns RTTI type info for payload type.
    virtual const std::type_info & _V_payload_type_info() const final {
        return typeid(PayloadT); }
    /// One has to implement all the payload processing here.
    virtual ProcRes _V_process_event_payload( PayloadT & ) = 0;

    iTEventPayloadProcessor( const std::string & pn ) :
                            iEventPayloadProcessorBase(pn) {}

    friend class ::sV::AnalysisPipeline;
};  // class iTEventPayloadProcessor

template<typename EventClassT,
         typename PayloadT>
PayloadT * iTEventPayloadProcessor<EventClassT, PayloadT>
                                            ::_reentrantPayloadPtr = nullptr;

template<typename EventClassT,
         typename PayloadT>
bool (*iTEventPayloadProcessor<EventClassT, PayloadT>::has_payload)
                               ( const AnalysisPipeline::Event & ) = nullptr;

template<typename EventClassT,
         typename PayloadT>
void (*iTEventPayloadProcessor<EventClassT, PayloadT>::unpack_payload)
                                    ( AnalysisPipeline::Event & ) = nullptr;

template<typename EventClassT,
         typename PayloadT>
void (*iTEventPayloadProcessor<EventClassT, PayloadT>::pack_payload)
                                    ( AnalysisPipeline::Event & ) = nullptr;


//
// Specialization for experimental payload
/////////////////////////////////////////

template<typename PayloadT>
class iTExperimentalEventPayloadProcessor :
        public iTEventPayloadProcessor<events::ExperimentalEvent, PayloadT> {
public:
    typedef AnalysisPipeline::Event Event;
    typedef iTEventPayloadProcessor<events::ExperimentalEvent, PayloadT> Parent;
private:
    static bool _has_payload( const Event & uEvent ) {
        if( !uEvent.has_experimental() ) {
            return false;
        }
        # ifndef NDEBUG
        if( ! uEvent.experimental()
                       .payload()
                       .Is<PayloadT>() ) {
            sV_logw( "Malformed experimental event message payload "
                         "\"%s\". Payload ignored.",
                         uEvent.experimental()
                                  .payload().GetTypeName().c_str());
            return false;
        }
        # endif
        return true;
    }
    /// Will be called if current event has payload of required type.
    static void _unpack_payload( Event & uEvent ) {
        Parent::_reentrantPayloadPtr = new PayloadT();
        uEvent.mutable_experimental()
                ->mutable_payload()
                ->UnpackTo(Parent::_reentrantPayloadPtr);
    }
    /// Will be called at the end of event processing pipeline.
    static void _pack_payload( Event & uEvent ) {
        uEvent.mutable_experimental()
                 ->mutable_payload()
                 ->PackFrom(*Parent::_reentrantPayloadPtr);
        delete Parent::_reentrantPayloadPtr;
        Parent::_reentrantPayloadPtr = nullptr;
    }
protected:
    iTExperimentalEventPayloadProcessor( const std::string & pn ) :
                            Parent(pn) {
        if( !Parent::has_payload ) {
            Parent::has_payload = _has_payload;
        }
        if( !Parent::unpack_payload ) {
            Parent::unpack_payload = _unpack_payload;
        }
        if( !Parent::pack_payload ) {
            Parent::pack_payload = _pack_payload;
        }
    }
};  // class iExperimentalEventPayloadProcessor

//
// Specialization for simulated payload:
/////////////////////////////////////////

template<typename PayloadT>
class iTSimulatedEventPayloadProcessor :
        public iTEventPayloadProcessor<events::SimulatedEvent, PayloadT> {
public:
    typedef AnalysisPipeline::Event Event;
    typedef iTEventPayloadProcessor<events::SimulatedEvent, PayloadT> Parent;
private:
    static bool _has_payload( const Event & uEvent ) {
        if( !uEvent.has_simulated() ) {
            return false;
        }
        # ifndef NDEBUG
        if( ! uEvent.simulated()
                    .payload()
                    .Is<PayloadT>() ) {
            sV_logw( "Malformed simulated event message payload "
                         "\"%s\". Payload ignored.",
                         uEvent.simulated()
                                .payload().GetTypeName().c_str() );
            return false;
        }
        # endif
        return true;
    }
    /// Will be called if current event has payload of required type.
    static void _unpack_payload( Event & uEvent ) {
        Parent::_reentrantPayloadPtr = new PayloadT();
        uEvent.mutable_simulated()
                ->mutable_payload()
                ->UnpackTo(Parent::_reentrantPayloadPtr);
    }
    /// Will be called at the end of event processing pipeline.
    static void _pack_payload( Event & uEvent ) {
        uEvent.mutable_experimental()
                ->mutable_payload()
                ->PackFrom(*Parent::_reentrantPayloadPtr);
        delete Parent::_reentrantPayloadPtr;
        Parent::_reentrantPayloadPtr = nullptr;
    }
protected:
    iTSimulatedEventPayloadProcessor( const std::string & pn ) :
                            Parent(pn) {
        if( !Parent::has_payload ) {
            Parent::has_payload = _has_payload;
        }
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

