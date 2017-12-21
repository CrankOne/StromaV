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

# include "pipeT/pipet.tcc"

# include "app/mixins/protobuf.hpp"
# include "uevent.hpp"
//# include "analysis/pipe_watcher.hpp"

# include <unordered_map>
# include <unordered_set>

namespace sV {

typedef pipet::Pipe<mixins::PBEventApp::UniEvent> AnalysisPipeline;

namespace aux {

/**@class AnalysisApplication::iEventSequence
 *
 * Data format reader object representation.
 *
 * @ingroup analysis
 * */
class iEventSequence : public pipet::interfaces::Source<mixins::PBEventApp::UniEvent> {
public:
    typedef mixins::PBEventApp::UniEvent Event;
    typedef uint8_t Features_t;
    enum Features : Features_t
    {
        plain        = 0x0,
        randomAccess = 0x1,
        identifiable = 0x2,
    };
private:
    uint8_t _features;
    Event * _ePtr;
protected:
    virtual bool _V_is_good() = 0;
    virtual void _V_next_event( Event *& ) = 0;
    /// Has to return a pointer to a valid event. Can invoke _V_next_event()
    /// internally.
    virtual Event * _V_initialize_reading() = 0;
    virtual void _V_finalize_reading() = 0;
    virtual void _V_print_brief_summary( std::ostream & ) const {}
public:
    iEventSequence( Features_t fts );
    virtual ~iEventSequence(){}

    /// Returns true if source initialized and next event can be read.
    virtual bool is_good() { return _V_is_good(); }

    /// Set-ups reading procedure.
    virtual Event * initialize_reading() { return _ePtr = _V_initialize_reading(); }

    /// Performs reading next event into provided reentrant instance.
    virtual void next_event( Event *& e ) { _V_next_event( e ); }

    /// Invoked after event reading done (e.g. to clean-up file descriptor).
    virtual void finalize_reading() {   _V_finalize_reading(); }

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

    virtual Event * get() override {
        if( !_ePtr ) {
            initialize_reading();
            if( is_good() ) {
                return _ePtr;
            }
            return nullptr;
        }
        if( is_good() ) {
            next_event(_ePtr);
            return _ePtr;
        } else {
            finalize_reading();
            return _ePtr = nullptr;
        }
    }
};

/**@class AnalysisApplication::iEventProcessor
 *
 * Event processing handler class interface. This class claims the basic logic
 * for pipeline processing handler used in sV.
 *
 * @ingroup analysis
 * */
template<typename EventT=mixins::PBEventApp::UniEvent>
class iEventProcessor {
public:
    typedef EventT Event;
    typedef pipet::PipeRC ProcRes;
private:
    const std::string _pName;
protected:
    /// Should return 'false' if processing in chain should be aborted.
    virtual ProcRes _V_process_event( Event & ) = 0;
public:
    iEventProcessor( const std::string & pn ) : _pName(pn) {}
    virtual ~iEventProcessor(){}

    virtual ProcRes operator()( Event & e ) {
        return _V_process_event( e );
    }
    const std::string & name() const { return _pName; }
};


template< typename ContainerMessageT
        , typename SubMessageT>
class SubPipeProcessor : public iEventProcessor<ContainerMessageT>
                       , public pipet::Pipe<SubMessageT> {
public:
    typedef ContainerMessageT ContainerMessage;
    typedef SubMessageT SubMessage;
    typedef iEventProcessor<ContainerMessageT> Parent;
protected:
    virtual SubMessage * _unpack( ContainerMessage ) = 0;
    virtual void _pack( const SubMessage & sMsg, ContainerMessage * ) = 0;
public:
    SubPipeProcessor( const std::string & nm ) : Parent(nm) {}
};

# if 0

class iStatefulProcessor {
protected:
    void _V_finalize(  );
};
# endif

# if 0

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
        assert(has_payload);
        if( has_payload(uEvent) ) {
            if( !_reentrantPayloadPtr ) {
                assert(unpack_payload);
                unpack_payload( uEvent );
            }
            ProcRes rs = _V_process_event_payload( *_reentrantPayloadPtr );
            if( _forcePayloadPack
                    && !(ABORT_CURRENT & rs)
                    &&  (CONTINUE_PROCESSING & rs) ) {
                // Pack payload if forced:
                pack_payload( uEvent );
            }
            return rs;
        }
        return iEventProcessor::RC_ACCOUNTED;
    }
    /// Helper method registering pack/unpack caching functions. Invoked by
    /// pipeline
    virtual void register_hooks( AnalysisPipeline * ppl ) /*final*/ {
        assert( pack_payload );
        ppl->register_packing_functions( nullate_cache,
                                         pack_payload );
    }
    /// Returns RTTI type info for payload type.
    virtual const std::type_info & _V_payload_type_info() const /*final*/ {
        return typeid(PayloadT); }
    /// One has to implement all the payload processing here.
    virtual ProcRes _V_process_event_payload( PayloadT & ) = 0;

    iTEventPayloadProcessor( const std::string & pn ) :
                            iEventPayloadProcessorBase(pn),
                            _forcePayloadPack(false) {}

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
        # ifndef NDEBUG
        if( Parent::_reentrantPayloadPtr ) {
            emraise(dbgBadArchitect, "Event payload (\"experimental\") caching logic "
                "violated: substitutive unpacking." );
        }
        # endif  // NDEBUG
        Parent::_reentrantPayloadPtr = new PayloadT();
        uEvent.mutable_experimental()
                ->mutable_payload()
                ->UnpackTo(Parent::_reentrantPayloadPtr);
    }
    /// Will be called at the end of event processing pipeline.
    static void _pack_payload( Event & uEvent ) {
        # ifndef NDEBUG
        if( !Parent::_reentrantPayloadPtr ) {
            sV_logw( "(debug) event payload (\"experimental\") caching logic "
                "violated: redundant packing routine invokation.\n" );
            return;
        }
        # endif // NDEBUG
        uEvent.mutable_experimental()
                 ->mutable_payload()
                 ->PackFrom(*Parent::_reentrantPayloadPtr);
        delete Parent::_reentrantPayloadPtr;
        Parent::_reentrantPayloadPtr = nullptr;
        //std::cout << "======================================" <<
        //    "Bank!!1!!"
        //    << std::endl;
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

/**@brief A trivial arbitration class.
 * @class DefaultArbiter
 *
 * This default arbiter implements a default pipeline steering with a single
 * parameter: whether or not proceed with events that were marked as
 * "discriminated" by processor classes. If abortDiscriminated was set to
 * true in ctr, the DISCRIMINATED flag in return code shall be effecitvely
 * equivalent to ABORT_CURRENT.
 *
 * Note, that processors may force cache packing routines to be evaluated in
 * any way (all this packing stuff is merely a fine setting for optimization
 * sake).
 **/
class DefaultArbiter : public iArbiter {
private:
    bool _abortDiscriminated;
protected:
    /// The global and local abort flags will considered in a natural way. The
    /// discrimination flag will not cause immediate stop.
    virtual AnalysisPipeline::EvalStatus _V_consider_rc(
                ProcRes sub, ProcRes & current ) override;
    /// If the abortion flags were set, the false will be returned
    /// unconditionally. If not, the result corresponds to !NOT_MODIFIED flag.
    virtual bool _V_do_pack( ProcRes g ) const override;
public:
    DefaultArbiter( bool abortDiscriminated=false ) :
                            _abortDiscriminated(abortDiscriminated) {}
    ~DefaultArbiter() {}
    bool abort_discriminated() const { return _abortDiscriminated; }
};  // DefaultArbiter
# endif

}  // namespace aux
}  // namespace sV

namespace pipet {
namespace aux {

template< typename MessageT
        > struct CallableTraits<sV::aux::iEventProcessor<MessageT>, MessageT, false> {
    typedef sV::aux::iEventProcessor<MessageT> Callable;
    typedef MessageT Message;
    constexpr static bool isFunction = false;
    typedef typename pipet::PipeRC CallableResult;
    typedef sV::aux::iEventProcessor<MessageT> & CallableRef;
};  // CallableTraits (sV's iEventProcessor)

}  // namespace aux
}  // namespace pipet

# endif  // RPC_PROTOCOLS
# endif  //H_STROMA_V_ANALYSIS_PIPELINE_H

