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

# ifndef H_STROMA_V_BASIC_PIPELINE_H
# define H_STROMA_V_BASIC_PIPELINE_H

# include <vector>
# include <goo/goo_exception.hpp>

namespace sV {

namespace aux {
template<typename T>
using STLAllocatedVector = std::vector<T>;

/**@brief Pipeline handler template wrapping the processing functor.
 * @class PipelineHandler
 *
 * The MessageT type has no any special restrictions.
 * The ProcessorT type has to be callable.
 * The ProcResT type has to be copy-constructible.
 * */
template< typename PipelineTraitsT>
class PipelineHandler {
public:
    typedef typename PipelineTraitsT::Message Message;
    typedef typename PipelineTraitsT::Processor Processor;
    typedef typename PipelineTraitsT::ProcRes ProcRes;
private:
    Processor * _p;
public:
    PipelineHandler( Processor * p ) : _p(p) {}
    ProcRes process( Message & m ) { return (*_p)( m ); }
    Processor & processor() { return *_p; }
    const Processor & processor() const { return *_p; }
};  // class PipelineHandler
}  // namespace aux

template< typename MessageT
        , typename ProcessorT
        , typename ProcResT
        , typename PipelineProcResT
        , template<typename T> class TChainT=aux::STLAllocatedVector >
struct PipelineTraits {
    /// Traits typedef.
    typedef PipelineTraits<MessageT, ProcessorT, ProcResT, PipelineProcResT, TChainT> Self;
    /// Message type (e.g. physical event).
    typedef MessageT   Message;
    /// Processor type (must be callable --- function or functor).
    typedef ProcessorT Processor;
    /// Result type, returning by handler call.
    typedef ProcResT   ProcRes;
    /// Result type, that shall be returned by pipeline invokation.
    typedef PipelineProcResT PipelineProcRes;
    /// Concrete handler interfacing type.
    typedef aux::PipelineHandler<Self> Handler;
    /// Concrete chain interfacing type (parent for this class).
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
};

/**@brief Strightforward pipeline template primitive.
 * @class Pipeline
 *
 * This is the basic implementation of pipeline that performs sequential
 * invokation of processing atoms stored at ordered container, guided by
 * arbitering class.
 *
 * The PipelineProcResT type has to be compy-constructible.
 *
 * Other template
 * parameters are similar to PipelineHandler's template parameters with
 * identical requirements.
 * */
template< typename PipelineTraitsT >
class Pipeline : public PipelineTraitsT::Chain {
public:
    typedef typename PipelineTraitsT::Message   Message;
    typedef typename PipelineTraitsT::Processor Processor;
    typedef typename PipelineTraitsT::ProcRes   ProcRes;
    typedef typename PipelineTraitsT::PipelineProcRes PipelineProcRes;
    typedef typename PipelineTraitsT::Handler   Handler;
    typedef typename PipelineTraitsT::Chain     Chain;
    typedef typename PipelineTraitsT::ISource   ISource;
    typedef typename PipelineTraitsT::IArbiter  IArbiter;
private:
    IArbiter * _a;
protected:
    IArbiter * arbiter_ptr() { return _a; }
    const IArbiter * arbiter_ptr() const { return _a; }
public:
    /// Ctr. Requires an arbiter instance to act.
    Pipeline( IArbiter * a ) : _a(a) {}
    /// Virtual dtr (trivial).
    virtual ~Pipeline() {}
    /// Run pipeline evaluation on source.
    virtual PipelineProcRes process( ISource & src ) {
        if( ! arbiter_ptr() ) {
            emraise( badState, "Arbiter object pointer is not set for "
                    "pipeline instance %p while process() was invoked.",
                    this );
        }
        IArbiter & a = *arbiter_ptr();
        Message * msg;
        while(!!(msg = src.next())) {
            for( Handler & h : *static_cast<Chain*>(this) ) {
                if( ! a.consider_handler_result( h.process(*msg) ) ) {
                    break;
                }
            }
            if( ! a.next_message() ) {
                break;
            }
        }
        return a.pop_result();
    }
    /// Run pipeline evaluation on single message.
    virtual PipelineProcRes process( Message & msg ) {
        if( ! arbiter_ptr() ) {
            emraise( badState, "Arbiter object pointer is not set for "
                    "pipeline instance %p while process() was invoked.",
                    this );
        }
        IArbiter & a = *arbiter_ptr();
        for( Handler & h : *static_cast<Chain*>(this) ) {
            if( ! a.consider_handler_result( h.process(msg) ) ) {
                break;
            }
        }
        return a.pop_result();
    }
    /// Shortcut for inserting processor at the back of pipeline.
    virtual void push_back( Processor * p ) {
        Chain::push_back( Handler(p) );
    }
    /// Call operator for single message --- to use pipeline as a processor.
    PipelineProcRes operator()( Message & msg ) {
        return process( msg );
    }
};  // class Pipeline

}  // namespace sV

# endif  // H_STROMA_V_BASIC_PIPELINE_H

