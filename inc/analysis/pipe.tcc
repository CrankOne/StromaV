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

# ifndef H_STROMA_V_PIPELINE_H
# define H_STROMA_V_PIPELINE_H

# include <vector>

namespace sV {

/**@brief Pipeline handler template wrapping the processing functor.
 * @class PipelineHandler
 *
 * The MessageT type has no any special restrictions.
 * The ProcessorT type has to be callable.
 * The ProcResT type has to be copy-constructible.
 * */
template< typename MessageT
        , typename ProcessorT
        , typename ProcResT>
class PipelineHandler {
public:
    typedef MessageT Message;
    typedef ProcessorT Processor;
    typedef ProcResT ProcRes;
private:
    Processor * _p;
public:
    PipelineHandler( Processor * p ) : _p(p) {}
    ProcRes process( Message & m ) { return (*_p)( m ); }
};  // class PipelineHandler

namespace aux {
template<typename T>
using STLAllocatedVector = std::vector<T>;
}

/**@brief Strightforward pipeline template primitive.
 * @class Pipeline
 *
 * This is basic implementation of pipeline that performs sequential invokation
 * of processing atoms stored at ordered container.
 *
 * The PipelineProcResT type has to be compy-constructible.
 *
 * Other template
 * parameters are similar to PipelineHandler's template parameters with
 * identical requirements.
 * */
template< typename MessageT
        , typename ProcessorT
        , typename ProcResT
        , typename PipelineProcResT
        , template<typename T> class TChainT=aux::STLAllocatedVector >
class Pipeline : public TChainT< PipelineHandler< MessageT
                                                , ProcessorT
                                                , ProcResT
                                                >
                               > {
public:
    /// Message type (e.g. physical event).
    typedef MessageT   Message;
    /// Processor type (must be callable --- function or functor).
    typedef ProcessorT Processor;
    /// Result type, returning by handler call.
    typedef ProcResT   ProcRes;
    /// Result type, that shall be returned by pipeline invokation.
    typedef PipelineProcResT PipelineProcRes;
    ///
    typedef PipelineHandler<MessageT, ProcessorT, ProcResT> Handler;
    ///
    typedef TChainT<Handler> Chain;
    /// Base source interface that has to be implemented.
    struct ISource {
        virtual Message * next( ) = 0;
    };  // class ISource
    /// Interface making a decision, whether to continue processing on few
    /// stages.
    /// Invokation of pop_result() is guaranteed at the ending of processing
    /// loop.
    struct IArbiter {
        /// Shall return true if result returned by current handler allows
        /// further propagation.
        virtual bool consider_handler_result( ProcRes ) = 0;
        /// Shall return result considering current state and reset state.
        virtual PipelineProcRes pop_result() = 0;
        /// Whether to retrieve next message from source and start
        /// event propagation.
        virtual bool next_message() = 0;
    };  // class IArbiter
protected:
    IArbiter * _a;
public:
    /// Ctr. Requires an arbiter instance to act.
    Pipeline( IArbiter * a ) : _a(a) {}
    /// Virtual dtr (trivial).
    virtual ~Pipeline() {}
    /// Run pipeline evaluation on source.
    virtual PipelineProcRes process( ISource & );
    /// Run pipeline evaluation on single message.
    virtual PipelineProcRes process( Message & );
};  // class Pipeline


template< typename MessageT
        , typename ProcessorT
        , typename ProcResT
        , typename PipelineProcResT
        , template<typename> class TChainT> PipelineProcResT
Pipeline< MessageT
        , ProcessorT
        , ProcResT
        , PipelineProcResT
        , TChainT >::process( ISource & src ) {
    Message * msg;
    while(!!(msg = src.next())) {
        for( Handler & h : *static_cast<Chain*>(this) ) {
            if( ! _a->consider_handler_result( h.process(*msg) ) ) {
                break;
            }
        }
        if( ! _a->next_message() ) {
            break;
        }
    }
    return _a->pop_result();
}

template< typename MessageT
        , typename ProcessorT
        , typename ProcResT
        , typename PipelineProcResT
        , template<typename> class TChainT > PipelineProcResT
Pipeline< MessageT
        , ProcessorT
        , ProcResT
        , PipelineProcResT
        , TChainT >::process( Message & msg ) {
    for( Handler & h : *static_cast<Chain*>(this) ) {
        if( ! _a->consider_handler_result( h.process(msg) ) ) {
            break;
        }
    }
    return _a->pop_result();
}


# if 0
template< typename MessageT
        , typename ProcessorT
        , typename ProcresT> void
Pipeline::process( ISource & ) {
    // Check if we actually have something to do
    if( _handlers.empty() ) {
        sV_loge( "No processors specified --- has nothing to do for "
                 "pipeline %p.\n", this );
        return -1;
    }
    // The temporary sources stack keeping internal state. Has to be empty upon
    // finishing processing.
    std::stack< std::pair<ISource *, Chain::iterator> > sourcesStack;
    // First in stack will refer to original source.
    sourcesStack.push( std::make_pair( &src, _processorsChain.begin() ) );
    // Begin main processing loop. Will run upon sources stack is non-empty AND
    // arbiter did not explicitly interrupt it.
    IArbiter::EvalStatus eStatus = IArbiter::continueEval;
    PipelineProcRes globalProcRC = _a->default_global_result();
    while( !sourcesStack.empty()
        && IArbiter::AbortProcessing != eStatus ) {
        // Reference pointing to the current event source.
        iEventSequence & cSrc = *sourcesStack.top().first;
        // Iterator pointing to the current handler in chain.
        Chain::iterator procStart =  sourcesStack.top().second;
        sourcesStack.pop();
        // Begin of loop iterating messages source.
        for( Message * msg = cSrc.initialize()
           ; cSrc.is_good()
           ; cSrc.next(msg) ) {
            Chain::iterator procIt;
            // Begin of loop iterating the handlers chain.
            for( procIt = procStart
               ; procIt != _processorsChain.end()
               ; ++procIt ) {
                // Process message with current handler and consider result.
                eStatus = _a->consider( procIt->process(msg)
                                      , globalProcRC );
                if( IArbiter::merged == eStatus ) {
                    // Fork was filled and junction has merged events. It
                    // means that we have to proceed with events that were put
                    // in "junction" queue as if it is an event source.
                    sourcesStack.push(
                        std::make_pair( procIt->junction_ptr(), ++procIt) );
                }
                if( IArbiter::propagate != eStatus ) {
                    // We have to stop propagating current message through the
                    // processors chain.
                    break;
                }
            }
            if( IArbiter::merged == eStatus ) {
                // We have to take next (newly-created) source from internal
                // queue and proceed with it.
                break;
            }
        }
    }
    return 0;
}
# endif

}  // namespace sV

# endif  // H_STROMA_V_PIPELINE_H

