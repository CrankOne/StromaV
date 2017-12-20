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

# include "analysis/processors/benchmark.hpp"

# if defined(RPC_PROTOCOLS) && defined(ANALYSIS_ROUTINES)

# if 0

# include <iomanip>

namespace sV {
namespace dprocessors {

Benchmarking::Benchmarking( const std::string & pn ) : Parent(pn),
                        sV::AbstractApplication::ASCII_Entry( goo::aux::iApp::exists() ?
                            &goo::app<AbstractApplication>() : nullptr, 1 ),
                        _initialEventSubmitted(false),
                        _dur_sumProcessing(0),
                        _n_events(0) {
}

Benchmarking::Benchmarking( const goo::dict::Dictionary & ) :
                        Benchmarking("benchmarking") {}

Benchmarking::~Benchmarking() {}

aux::iEventProcessor::ProcRes
Benchmarking::_V_process_event( Event & ) {
    if( !_initialEventSubmitted ) {
        timer_start( _start );
        _initialEventSubmitted = true;
    }
    timer_start( _latestEventStart );
    ++_n_events;
    return iEventProcessor::RC_ACCOUNTED;
}

aux::iEventProcessor::ProcRes
Benchmarking::_V_finalize_event_processing( Event & ) {
    _dur_sumProcessing +=
        (_dur_latestEventProcessing = timer_end( _latestEventStart ) );
    _update_stat();
    return iEventProcessor::RC_ACCOUNTED;
}

void
Benchmarking::_V_print_brief_summary( std::ostream & os ) const {
    if( !_initialEventSubmitted ) return;
    os << ESC_CLRGREEN "Benchmarking" ESC_CLRCLEAR ":" << std::endl
       << std::fixed << std::setprecision(2)
       << "  events considered .......... : " << _n_events << std::endl
       << "  overall duration, sec ...... : " << double(_dur_overall)*1e-9 << std::endl
       << "    avrg usec/event .......... : " << double(_dur_overall)/(1000.*_n_events) << std::endl
       << "  proc. elapsed dur, sec ..... : " << double(_dur_sumProcessing)*1e-9 << std::endl
       << "    avrg usec/event proc. .... : " << double(_dur_sumProcessing)/(1000.*_n_events) << std::endl
       ;
}

void
Benchmarking::finalize() const {
    _dur_overall = timer_end( _start );
}

void
Benchmarking::_update_stat() {
    if( !can_acquire_display_buffer() ) return;
    if( !_initialEventSubmitted ) return;

    char ** lines = my_ascii_display_buffer();
    assert( lines[0] && !lines[1] );

    _dur_overall = timer_end( _start );

    double overallPerf = (1e9*_n_events)/double(_dur_overall),
           procPerf = (1e9*_n_events)/double(_dur_sumProcessing)
           ;

    snprintf( lines[0], ::sV::aux::ASCII_Display::LineLength,
        "events#/sec %.2f, events#proc./sec %.2f, ev.decoding lag %.2f%%",
        overallPerf, procPerf, 100*(procPerf-overallPerf)/procPerf );
}

void
Benchmarking::timer_start( struct timespec & start_time ) {
    clock_gettime(CLOCK_MONOTONIC, &start_time);
}

long
Benchmarking::timer_end(const struct timespec & start_time) {
    struct timespec end_time;
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    long diffInNanos = (end_time.tv_sec  - start_time.tv_sec )*1e9
                     + (end_time.tv_nsec - start_time.tv_nsec)
                     ;
    return diffInNanos;
}

StromaV_ANALYSIS_PROCESSOR_DEFINE( Benchmarking, "benchmarking" ) {
    return goo::dict::Dictionary( NULL,
            "TODO" );
}

}  // namespace dprocessors
}  // namespace sV

# endif

# endif  // defined(RPC_PROTOCOLS) && defined(ANALYSIS_ROUTINES)

