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

# ifndef H_STROMA_V_ANALYSIS_TESTING_PROCESSOR_H
# define H_STROMA_V_ANALYSIS_TESTING_PROCESSOR_H

# include "config.h"

# if defined(RPC_PROTOCOLS) && defined(ANALYSIS_ROUTINES)

# include "app/analysis.hpp"
# include "uevent.hpp"

namespace sV {
namespace dprocessors {
namespace aux {

class TestingProcessor : public AnalysisPipeline::iEventProcessor {
public:
    typedef AnalysisPipeline::iEventProcessor Parent;
    typedef AnalysisApplication::Event Event;
protected:
    virtual bool _V_process_event( Event * ) override;
public:
    TestingProcessor( const std::string & pn ) :
                AnalysisPipeline::iEventProcessor( pn ) {}
};  // class TestingProcessor

}  // namespace aux
}  // namespace dprocessors
}  // namespace sV

# endif  // defined(RPC_PROTOCOLS) && defined(ANALYSIS_ROUTINES)

# endif  // H_STROMA_V_ANALYSIS_TESTING_PROCESSOR_H

