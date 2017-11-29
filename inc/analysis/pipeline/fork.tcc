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

namespace sV {

class IForkDispatcher {
public:
    // ...
};  // class IForkDispatcher

/**@brief Processor that performs multiplexing incoming messages among few
 *        child pipelines.
 * @class ForkProcessor
 *
 * The forking processor dispatches incoming messages among two or more child
 * pipelines. To do so, it does return "skip" eval result code (of type
 * Traits::ProcRes) until corresponding Junction ...  TODO
 * */
template<typename TraitsT>
class ForkProcessor : public TraitsT::Processor,
                      public std::set<Pipeline<TraitsT>*> {
public:
    typedef typename PipelineTraitsT::Message   Message;
    typedef typename PipelineTraitsT::Processor Processor;
    typedef typename PipelineTraitsT::ProcRes   ProcRes;
    typedef typename PipelineTraitsT::PipelineProcRes PipelineProcRes;
    typedef typename PipelineTraitsT::Handler   Handler;
    typedef typename PipelineTraitsT::Chain     Chain;
    typedef typename PipelineTraitsT::ISource   ISource;
    typedef typename PipelineTraitsT::IArbiter  IArbiter;
    /// Pipeline pointers container type.
    typedef std::set<Pipeline<TraitsT>*>        Pipelines;
public:
    ForkProcessor();
    virtual ProcRes operator()(Message * msg) {
        _dispatcher->dispatch_message( msg );
    }
    bool is_full() const;  // TODO
};  // class ForkProcessor

}  // namespace sV

# endif  // H_STROMA_V_PIPELINE_FORK_H

