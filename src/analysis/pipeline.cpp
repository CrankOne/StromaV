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

AnalysisPipeline::AnalysisPipeline() : _evSeq(nullptr) {}

void
AnalysisPipeline::push_back_processor( iEventProcessor * proc ) {
    if( !proc ) {
        emraise( nullPtr, "Can't add a processor --- null pointer." );
    }
    _processorsChain.push_back( proc );
    auto payloadProcPtr = dynamic_cast<iEventPayloadProcessorBase*>( proc );
    if( payloadProcPtr ) {
        payloadProcPtr->register_hooks( this );
    }
    sV_log3( "Processor %p now handles event pipeline.\n", proc );
}

void
AnalysisPipeline::register_packing_functions( void(*invalidator)(),
                                              void(*packer)(Event*) ) {
    if( _invalidators.end() == _invalidators.find( invalidator ) ) {
        _invalidators.insert( invalidator );
    }
    if( _payloadPackers.end() == _payloadPackers.find( packer ) ) {
        _payloadPackers.insert( packer );
    }
}

int
AnalysisPipeline::_process_chain( Event * evPtr ) {
    int n = 0;
    for( auto it  = _processorsChain.begin();
              it != _processorsChain.end(); ++it, n++ ) {
        if(!(**it)( evPtr )) {
            // Processor has to return false to break the loop.
            break;
        }
    }
    return n;
}

void
AnalysisPipeline::_finalize_event( Event * evPtr ) {
    for( auto it  = _processorsChain.begin();
              it != _processorsChain.end(); ++it ) {
        (**it).finalize_event( evPtr );
    }
    for( auto & packer : _payloadPackers ) {
        packer( evPtr );
    }
    for( auto & nullate : _invalidators ) {
        nullate();
    }
}

void
AnalysisPipeline::_finalize_sequence(
                                AnalysisPipeline::iEventSequence * evSeqPtr ) {
    evSeqPtr->finalize_reading();
    for( auto it  = _processorsChain.begin();
              it != _processorsChain.end(); ++it ) {
        (**it).finalize();
    }
}

int
AnalysisPipeline::process( AnalysisPipeline::Event * evPtr ) {
    int n = _process_chain( evPtr );
    _finalize_event( evPtr );
    return n;
}

int
AnalysisPipeline::process( AnalysisPipeline::iEventSequence * evSeqPtr ) {
    assert( evSeqPtr );
    _evSeq = evSeqPtr;

    // Check if we actually have something to do
    if( _processorsChain.empty() ) {
        sV_logw( "No processors specified --- has nothing to do for "
                     "pipeline %p.\n", this );
    }
    //AnalysisPipeline::iEventSequence & evseq
    //                        = get_evseq<AnalysisPipeline::iEventSequence&>();
    for( auto evPtr = evSeqPtr->initialize_reading();
         evSeqPtr->is_good();
         evSeqPtr->next_event( evPtr ) ) {
        this->process( evPtr );
    }
    _finalize_sequence( evSeqPtr );

    _evSeq = nullptr;

    return 0;
}

namespace aux {

//
// iEventSequence impl

iEventSequence::iEventSequence( Features_t fts ) : _features(fts) {}

}  // namespace aux

}  // namespace sV

# endif  // RPC_PROTOCOLS


