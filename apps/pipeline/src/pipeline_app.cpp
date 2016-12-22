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

# include "pipeline_app.hpp"

namespace sV {

int
App::_V_run() {
    if( do_immediate_exit() ) return EXIT_FAILURE;
    // Check if we actually have something to do
    if( _processorsChain.empty() ) {
        sV_logw( "No processors specified --- has nothing to do.\n" );
    }
    AnalysisPipeline::iEventSequence & evseq
                            = get_evseq<AnalysisPipeline::iEventSequence&>();

    int rc = this->AnalysisPipeline::process( &evseq );

    evseq.print_brief_summary( goo::app<App>().ls() );
    for( auto it  = _processorsChain.begin();
              it != _processorsChain.end(); ++it ) {
        (**it).print_brief_summary( goo::app<App>().ls() );
    } 

    return EXIT_SUCCESS ? rc == 0 : EXIT_FAILURE;
}

}  // namespace sV


