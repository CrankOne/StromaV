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

# include "analysis/pipeline.hpp"

# ifdef RPC_PROTOCOLS

namespace sV {

AnalysisPipeline::Handler::Handler( iEventProcessor & processor_ ) :
            _processor(processor_),
            _payloadTraits(nullptr) {
    bzero( &_stats, sizeof(Statistics) );
    auto payloadProcPtr = dynamic_cast<iEventPayloadProcessorBase*>( &_processor );
    if( payloadProcPtr ) {
        _payloadTraits = new PayloadTraits( payloadProcPtr->payload_type_info() );
    }
}

AnalysisPipeline::Handler::PayloadTraits &
AnalysisPipeline::Handler::payload_traits() {
    if( ! payload_traits_available() ) {
        emraise( badState, "Handler of processors %p has no payload traits.",
            &_processor );
    }
    return *_payloadTraits;
}

AnalysisPipeline::Handler::~Handler() {
    if( _payloadTraits ) {
        delete _payloadTraits;
    }
}

AnalysisPipeline::AnalysisPipeline() :
            ASCII_Entry( goo::aux::iApp::exists() ?
                        &goo::app<AbstractApplication>() : nullptr, 1 ),
                _nEventsAcquired(0) {}


void
AnalysisPipeline::_update_stat() {
    ++_nEventsAcquired;
    // ...
}

void
AnalysisPipeline::push_back_processor( iEventProcessor & proc ) {
    Handler handler(proc);
    if( !_processorsChain.empty() ) {
        // We have to force payload packing of the previous processor in cases:
        //  - When previous processor has unpacked payload cache and current
        // has not (probably, current processor will deal with an entire
        // event).
        //  - When payload types of both caching processors differ.
        Handler & prevHandler = _processorsChain.back();
        // We shall not force to pack anything, if previous processor does not
        // cache payload.
        if( prevHandler.payload_traits_available() && (
                (!handler.payload_traits_available())
             || (prevHandler.payload_traits().TI != handler.payload_traits().TI)
            )) {
            prevHandler.payload_traits().forcePack = true;
        }
    }
    if( handler.payload_traits_available() ) {
        static_cast<iEventPayloadProcessorBase&>(proc).register_hooks( this );
    }
    _processorsChain.push_back( handler );
    
    sV_log3( "Processor %p now handles event pipeline.\n", &proc );
}

void
AnalysisPipeline::register_packing_functions( void(*invalidator)(),
                                              void(*packer)(Event&) ) {
    if( _invalidators.end() == _invalidators.find( invalidator ) ) {
        _invalidators.insert( invalidator );
    }
    if( _payloadPackers.end() == _payloadPackers.find( packer ) ) {
        _payloadPackers.insert( packer );
    }
}

int
AnalysisPipeline::_process_chain( Event & event ) {
    int n = 0;
    for( auto it  = _processorsChain.begin();
              it != _processorsChain.end(); ++it, n++ ) {
        AnalysisPipeline::iEventProcessor::ProcRes result = it->processor()( event );
        if(    (result & AnalysisPipeline::iEventProcessor::ABORT_CURRENT)
           || !(result & AnalysisPipeline::iEventProcessor::CONTINUE_PROCESSING)) {
            break;
        }
    }
    return n;
}

void
AnalysisPipeline::_finalize_event( Event & event ) {
    for( auto it  = _processorsChain.begin();
              it != _processorsChain.end(); ++it ) {
        AnalysisPipeline::iEventProcessor::ProcRes result = it->processor().finalize_event( event );
        if(    (result & AnalysisPipeline::iEventProcessor::ABORT_CURRENT)
           || !(result & AnalysisPipeline::iEventProcessor::CONTINUE_PROCESSING)) {
            break;
        }
    }
    for( auto & packer : _payloadPackers ) {
        packer( event );
    }
    for( auto & nullate : _invalidators ) {
        nullate();
    }
    _update_stat();
}

void
AnalysisPipeline::_finalize_sequence(
                                AnalysisPipeline::iEventSequence & evSeq ) {
    evSeq.finalize_reading();
    for( auto it  = _processorsChain.begin();
              it != _processorsChain.end(); ++it ) {
        it->processor().finalize();
    }
}

int
AnalysisPipeline::process( AnalysisPipeline::Event & event ) {
    int n = _process_chain( event );
    _finalize_event( event );
    return n;
}

int
AnalysisPipeline::process( AnalysisPipeline::iEventSequence & evSeq ) {
    // Check if we actually have something to do
    if( _processorsChain.empty() ) {
        sV_loge( "No processors specified --- has nothing to do for "
                     "pipeline %p.\n", this );
        return -1;
    }
    size_t nEventsProcessed = 0;
    for( auto evPtr = evSeq.initialize_reading();
         evSeq.is_good();
         evSeq.next_event( evPtr ), ++nEventsProcessed ) {
        #ifndef NDEBUG
        if( !evPtr ) {
            emraise( dbgBadArchitect, "Event sequnce returned a NULL event pointer!" );
        }
        #endif
        this->process( *evPtr );
    }
    sV_log2( "Pipeline %p depleted the source %p with %zu events. Finalizing...\n",
            this, &evSeq, nEventsProcessed );
    _finalize_sequence( evSeq );
    return 0;
}

namespace aux {

/** Accepts subprocess result as a first argument and reference to global as a
 * second. The usual usage implies consideration of result returned by
 * processing event data subsection (e.g. particular detector).
 * TODO: usage snippet (may be taken from any existing implementation)
 * */
bool
iEventProcessor::consider_interim_result( ProcRes local, ProcRes & global ) {
    bool stopCurrent = false;
    if( !(CONTINUE_PROCESSING & local) ) {
        stopCurrent = true;
        global &= ~CONTINUE_PROCESSING;  // unset global 'continue' flag.
    }
    if( ABORT_CURRENT & local ) {
        stopCurrent = true;
    }
    if( DISCRIMINATE & local ) {
        global |= DISCRIMINATE;
    }
    if( !(NOT_MODIFIED & local) ) {
        global &= ~NOT_MODIFIED;  // unset global 'not modified' flag.
    }
    return stopCurrent;
}

//
// iEventSequence impl

iEventSequence::iEventSequence( Features_t fts ) : _features(fts) {}

}  // namespace aux

}  // namespace sV

# endif  // RPC_PROTOCOLS


