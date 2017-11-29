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

# ifndef H_STROMA_V_PIPELINING_MANIFOLD_H
# define H_STROMA_V_PIPELINING_MANIFOLD_H

namespace sV {

namespace aux {

/// Possible result flags and composite shortcuts that may be returned by
/// procedure. Note, that flags itself may be counter-intuitive, so better
/// stand for shortcuts (the have RC_ prefix). Priority of treatment:
/// 1. Global abort --- stop processing, gently finalize all the processing
/// handlers (flush buffers, build plots, close files, etc).
/// 2. Event abort  --- no further treatment of current shall be performed.
/// No modification have to be considered.
/// 3. Modification --- shall propagate modified flag to caller.
/// 4. Discriminate --- shall pull out the event/sub-event/sample. Arbiter,
/// depending of its modification, may keep process this event.
/// Note: all modification will be refused upon abort/discrimination.
enum ManifoldProcessingResultFlags : int8_t {
    CONTINUE_PROCESSING = 0x1,  ///< when set, all the analysis has to be interrupted
    ABORT_CURRENT       = 0x2,  ///< processing of current event or payload has to be interrupted
    DISCRIMINATE        = 0x4,  ///< processing continues, but the current event/payload has to be considered as discriminated
    NOT_MODIFIED        = 0x8,  ///< the event or data payload wasn't modified
    JUNCTION_DONE       = 0x10  ///< returned ONLY by f/j handlers. Has special meaning.
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

template< typename MessageT>
struct iManifoldProcessor {
public:
    typedef MessageT Message;
public:
    virtual ManifoldProcessingResultFlags operator()( Message & ) = 0;
};

//template<typename MessageT, typename ManifoldResultT>
//using ManifoldTraits = PipelineTraitsT< MessageT
//                                      , iManifoldProcessor<MessageT>
//                                      , ManifoldProcessingResultFlags
//                                      , ManifoldResultT>
//                                      ;

template< typename MessageT
        , typename ManifoldResultT >
struct ManifoldTraits : protected PipelineTraitsT< MessageT
                                                 , iManifoldProcessor<MessageT>
                                                 , ManifoldProcessingResultFlags
                                                 , ManifoldResultT > {
    /// Self traits typedef.
    typedef ManifoldTraits<MessageT, ManifoldResultT> Self;
    /// Parent (hidden) typedef.
    typedef PipelineTraitsT< MessageT
                           , iManifoldProcessor<MessageT>
                           , ManifoldProcessingResultFlags
                           , PipelineProcResT
                           , ManifoldResultT > Parent;
    /// Message type (e.g. physical event).
    typedef typename Parent::Message   Message;
    /// Processor type (must be callable --- function or functor).
    typedef typename Parent::Processor Processor;
    /// Result type, returning by handler call.
    typedef typename Parent::ProcRes   ProcRes;
    /// Result type, that shall be returned by pipeline invokation.
    typedef typename Parent::PipelineProcRes PipelineProcRes;
    /// Concrete handler interfacing type.
    typedef aux::PipelineHandler<Self> Handler;
    /// Concrete chain interfacing type (parent for this class).
    typedef typename Parent::Chain Chain;
    /// Base source interface that has to be implemented.
    typedef typename Parent::ISource ISource;
    /// Interface making a decision, whether to continue processing on few
    /// stages.
    /// Invokation of pop_result() is guaranteed at the ending of processing
    /// loop.
    struct IArbiter : public Parent::IArbiter {
    private:
        bool _doAbort
           , _doSkip
           , _forkFilled
           ;
    public:
        /// Causes transitions
        virtual bool consider_handler_result( ManifoldProcessingResultFlags fs ) override {
            _doAbort = !(CONTINUE_PROCESSING & fs);
            _forkFilled = JUNCTION_DONE & fs;
            return _doAbort || (ABORT_CURRENT & fs);
        }
        virtual PipelineProcRes pop_result() override {
            // ... TODO: what?
        }
        virtual bool next_message() override {
            return !_doAbort;
        }
        virtual bool is_fork_filled() override {
            return _forkFilled;
        }
    };  // class IArbiter
};

}  // namespace aux

/**@brief Assembly of pipelines with elaborated composition management.
 * @class Manifold
 *
 * The Manifold class manages assembly of pipelines, performing fork/junction
 * (de-)multiplexing, changing the event types and automated insertion of
 * auxilliary handlers.
 * */
template< typename MessageT
        , typename ResultT>
class Manifold : public Pipeline< aux::ManifoldTraits< MessageT
                                                     , ResultT
                                                     >
                                > {
public:
    typedef aux::ManifoldTraits<MessageT, ResultT> Traits;
    typedef typename Traits::Message   Message;
    typedef typename Traits::Processor Processor;
    typedef typename Traits::ProcRes   ProcRes;
    typedef typename Traits::PipelineProcRes PipelineProcRes;
    typedef typename Traits::Handler   Handler;
    typedef typename Traits::Chain     Chain;
    typedef typename Traits::ISource   ISource;
    typedef typename Traits::IArbiter  IArbiter;
public:
    /// Manifold overloads the source processing method to support fork/junction
    /// processing.
    virtual PipelineProcRes process( ISource & ) override;
    /// Single message will be processed as a (temporary) messages source
    /// containing only one event.
    virtual PipelineProcRes process( Message & ) override {
        _TODO_  // TODO
    }
};  // class ForkProcessor


template< typename MessageT
        , typename ResultT> ResultT
Manifold::process( ISource & src ) {
    // Check if we actually have something to do
    if( Chain::empty() ) {
        emraise( badState
               , "No processors specified --- has nothing to do for "
                 "manifold %p.\n"
               , this );
    }
    IArbiter & a = *arbiter_ptr();
    // The temporary sources stack keeping internal state. Has to be empty upon
    // finishing processing.
    std::stack< std::pair<ISource *, Chain::iterator> > sourcesStack;
    // First in stack will refer to original source.
    sourcesStack.push( std::make_pair( &src, _processorsChain.begin() ) );
    // Begin main processing loop. Will run upon sources stack is non-empty AND
    // arbiter did not explicitly interrupt it.
    while( !sourcesStack.empty() ) {
        // Reference pointing to the current event source.
        iEventSequence & cSrc = *sourcesStack.top().first;
        // Iterator pointing to the current handler in chain.
        Chain::iterator procStart =  sourcesStack.top().second;
        sourcesStack.pop();
        // Begin of loop iterating messages source.
        Message * msg;
        while(!!(msg = cSrc.next())) {
            Chain::iterator procIt;
            // Begin of loop iterating the handlers chain.
            for( procIt = procStart
               ; procIt != _processorsChain.end()
               ; ++procIt ) {
                // Process message with current handler and consider result.
                if( mnfldArbiter.consider_handler_result( procIt->process( msg ) ) ) {
                    // consider_handler_result() returned true, what means we
                    // can propagate further along the handlers chain.
                    continue;
                }
                // consider_handler_result() returned false, that means we have
                // to interrupt the propagation, but if we have filled the
                // f/j handler, it must be put on top of sources stack.
                if( mnfldArbiter.fork_filled() ) {
                    // Fork was filled and junction has merged events. It
                    // means that we have to proceed with events that were put
                    // in "junction" queue as if it is an event source.
                    sourcesStack.push(
                        std::make_pair( procIt->junction_ptr(), ++procIt) );
                }
                break;
            }
        }
        if( mnfldArbiter.next_message() ) {
            // We have to take next (newly-created) source from internal
            // queue and proceed with it.
            break;
        }
    }
    return a.pop_result();
}

}  // namespace sV

# endif  // H_STROMA_V_PIPELINING_MANIFOLD_H


