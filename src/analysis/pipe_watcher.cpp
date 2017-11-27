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

# include "sV_config.h"

# ifdef RPC_PROTOCOLS

# include "analysis/pipe_watcher.hpp"
# include "app/mixins/protobuf.hpp"
# include "analysis/pipeline.hpp"

namespace sV {
namespace aux {

void
iPipelineWatcher::track_pipeline( AnalysisPipeline * ) {
    _TODO_  // TODO
}

void
iPipelineWatcher::release_pipeline( AnalysisPipeline * ) {
    _TODO_  // TODO
}

void
iPipelineWatcher::update_structure( AnalysisPipeline * ppl ) {
    sV::reports::PipelineReport & R = *_repPtr;
    // wipe out all previously stored information about processors in pipeline
    R.clear_processorstates();
    // fill information about each processor at the pipeline using handler
    // wrapper
    for( const AnalysisPipeline::Handler & h : ppl->_processorsChain ) {
        sV::reports::ProcessorState & ps = *R.add_processorstates();
        if( h.payload_traits_available() ) {
            if( h.payload_traits().forcePack ) {
                ps.set_type( reports::ProcessorState::typedPayload );
            } else {
                ps.set_type( reports::ProcessorState::payloadPlain );
            }
        } else if( h.junction_available() ) {
            ps.set_type( reports::ProcessorState::fj );
            _TODO_  // TODO
        } else {
            ps.set_type( reports::ProcessorState::ordinary );
        }
        ps.set_name( h.processor().processor_name() );
        ps.set_rtticlassid( typeid(h.processor()).name() );
        // ...
    }
}

void
iPipelineWatcher::update_stats( AnalysisPipeline * ) {
    sV::reports::PipelineReport & R = *_repPtr;
    # ifndef NDEBUG
    if( R.processorstates_size() != ppl->_processorsChain ) {
        emraise(asertFailed, "iPipelineWatcher::update_stats() indicates "
            "changed pipeline structure (processor number: %d kept vs %d "
            "real).", R.processorstates_size(), ppl->_processorsChain );
    }
    // ... other checks
    # endif
    _TODO_  // TODO: f/j support
    // ...
    uint32_t np = 0;
    for( const AnalysisPipeline::Handler & h : ppl->_processorsChain ) {
        sV::reports::ProcessorState & ps = *R.mutable_processorstates( np );
        ps.set_nconsidered( h.stats().nConsidered );
        ps.set_ndiscriminated( h.stats().nDiscriminated );
        ps.set_aborted( h.stats().nAborted );
        if( h.payload_traits_available() ) {
            ps.set_npacked( h.payload_traits().nPacked );
        }
        if( h.junction_available() ) {
            _TODO_  // TODO: f/j support
        }
        // ...
        ++np;
    }
}

iPipelineWatcher::iPipelineWatcher() : _repPtr(sV_MSG_NEW(sV::reports::PipelineReport)) {
}

}  // namespace aux
}  // namespace sV

# endif  // RPC_PROTOCOLS

