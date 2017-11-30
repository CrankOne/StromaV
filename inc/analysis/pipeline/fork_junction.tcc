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

# ifndef H_STROMA_V_PIPELINE_FORK_H
# define H_STROMA_V_PIPELINE_FORK_H

# include "analysis/pipeline/manifold.tcc"

namespace sV {

namespace aux {

/**@brief Axilliary class performing common operations with set of parallel
 *        pipes.
 * @class IDispatcher
 *
 * This class defines how to perform required operations with particular
 * container type. This behaviour is clamper by ForkJoin pipeline node.
 * */
template< typename MessageT
        , typename ContainerT>
class IDispatcher {
protected:
    /// Shall put the message into one of the given manifolds.
    virtual void _V_dispatch( ContainerT &, MessageT & ) = 0;
    /// Shall return true when no more messages may be recieved.
    virtual bool _V_is_complete() const = 0;
    /// Shall suspend execution until message(s) may be retrieved and
    /// return overall result.
    virtual aux::ManifoldProcessingResultFlags _V_wait() = 0;
    /// Shall return completed messages, one by one.
    virtual MessageT * next_message( ContainerT & ) = 0;
    friend class ForkJoin<Message>;
};  // class IDispatcher
}  // namespace aux


/**@brief A processor that manages multiple parallel pipelines manifolds.
 * @class ForkJoin
 *
 * The forking processor (F/J node) dispatches incoming messages among multiple
 * child pipelines (sub-manifolds). The special behaviour of this node is that
 * it acquires few messagess until it will be filled, causing outern pipeline
 * to abort propagation of current message. Once the F/J is becomes ready to
 * return processed message(s), the outern manifold (pipeline) has to proceed
 * from F/J node as if it is a message source until it will be depleted.
 *
 * The complicated behaviour of F/J node brings significant benefit once one
 * need to run few data processing pipelines in parallel (i.e, using multiple
 * computational threads).
 *
 * The special arbitering class is defined here to handle two special cases:
 *  1. When given messages are dispatched among the child manifolds and next
 *     message has to be put into F/J node.
 *  2. When messages could be extracted from F/J node.
 * */
template< typename MessageT
        , typename ContainerT >
class ForkJoin : public aux::iManifoldProcessor<MessageT>
               , public typename aux::iManifoldProcessor<MessageT>::Traits::ISource {
public:
    typedef MessageT Message;
    typedef ContainerT Container;
    typedef aux::IDispatcher<MessageT, Container> IDispatcher;
private:
    Container & _container;
    iDispatcher & _dispatch;
public:
    ForkProcessor( Container & container
                 , const iDispatcher & dsp ) : _container( container ) 
                                             , _dispatcher( dsp ) {}

    virtual aux::ManifoldProcessingResultFlags operator()(Message & msg) override {
        Traits::ProcRes pr = aux::ABORT_CURRENT;
        _dispatcher._V_dispatch( *this, msg );
        if( _dispatcher._V_is_complete() ) {
            pr = _dispatcher.wait();
            return pr | aux::JUNCTION_DONE;
        }
        return pr;
    }

    virtual Message * next() override {
        return _dispatcher.next_message( _container );
    }
};  // class ForkProcessor

}  // namespace sV

# endif  // H_STROMA_V_PIPELINE_FORK_H

