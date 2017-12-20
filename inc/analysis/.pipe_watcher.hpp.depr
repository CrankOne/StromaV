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

# ifndef H_STROMA_V_ANALYSIS_PIPELINE_TRACKING_H
# define H_STROMA_V_ANALYSIS_PIPELINE_TRACKING_H

# include "sV_config.h"

# ifdef RPC_PROTOCOLS

// A trick to disable shadowing warnings in aut-generated GPB
// headers.
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wshadow"
# include "pipeline_reports.pb.h"
# pragma GCC diagnostic pop

namespace sV {

// FWD (see analysis/pipeline.hpp)
class AnalysisPipeline;

namespace aux {

/**@brief Pipeline tracking class implementation making reports in protobuf
 *        messages.
 * @class PBPipelineReporter
 *
 * Interfacing container class triggering periodical updates. Useful for
 * tracking pipeline progress.
 *
 * @ingroup analysis
 **/
class iPipelineWatcher {
private:
    sV::reports::PipelineReport * _repPtr;
public:
    iPipelineWatcher();
    virtual void track_pipeline( AnalysisPipeline * );
    virtual void release_pipeline( AnalysisPipeline * );
    virtual void update_stats( AnalysisPipeline * );
    virtual void update_structure( AnalysisPipeline * );
};  // class PBPipelineReporter

}  // namespace aux
}  // namespace sV

# endif  // RPC_PROTOCOLS
# endif  // H_STROMA_V_ANALYSIS_PIPELINE_TRACKING_H


