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

# ifndef H_STROMA_V_MULTICASTING_DATA_PROCESSOR_H
# define H_STROMA_V_MULTICASTING_DATA_PROCESSOR_H

# include "sV_config.h"

# if defined(RPC_PROTOCOLS) && defined(ANALYSIS_ROUTINES)

# if 0

# include "app/analysis.hpp"
# include "uevent.hpp"

# include <time.h>

namespace sV {
namespace dprocessors {

/**@class Benchmarking
 * @brief Aux processor measuring time elapsed for events treatment.
 *
 * This class contains a set of precise timers measuring time elapsed for:
 *  - Single event processing time
 *  - Overall events processing time (as sum of all previous single events times)
 *  - Overall time from first event 
 * The single event processing time will be acquired in assumption that this
 * processor goes first in chain. I.e. it has to be placed right after the
 * data source in processing pipeline.
 */
class Benchmarking : public sV::aux::iEventProcessor,
                     public sV::AbstractApplication::ASCII_Entry {
public:
    typedef sV::aux::iEventProcessor Parent;
    typedef mixins::PBEventApp::UniEvent Event;
private:
    bool _initialEventSubmitted;
    struct timespec _start,
                    _latestEventStart
                    ;
    mutable long _dur_latestEventProcessing,
                  _dur_sumProcessing,
                  _dur_overall,
                  _n_events
                  ;
protected:
    virtual ProcRes _V_process_event( Event & ) override;
    virtual void _V_finalize_event_processing( Event & ) override;
    virtual void _V_print_brief_summary( std::ostream & ) const override;
    void _update_stat();
public:
    Benchmarking( const std::string & pn );
    Benchmarking( const goo::dict::Dictionary & );
    virtual ~Benchmarking();

    static void timer_start( struct timespec & );
    static long timer_end(const struct timespec & start_time);
    void finalize() const;
};  // class Benchmarking

}  // namespace dprocessors
}  // namespace sV
# endif

# endif  // defined(RPC_PROTOCOLS) && defined(ANALYSIS_ROUTINES)
# endif  // H_STROMA_V_MULTICASTING_DATA_PROCESSOR_H


