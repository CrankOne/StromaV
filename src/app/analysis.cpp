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

# include "app/analysis.hpp"

# ifdef RPC_PROTOCOLS

# include "event.pb.h"
# include "app/mixins/root.hpp"

# include <TFile.h>

/**@defgroup analysis Analysis
 *
 * @brief This group provides various analysis routines for processing
 * experimental or modelled data.
 * */

namespace sV {

//void
//AnalysisApplication::push_back_processor( const std::string & name ) {
//    auto p = find_processor( name )();
//    sV_log3( "Adding processor %s:%p.\n", name.c_str(), p );
//    AnalysisPipeline::push_back_processor( p );
//}

void
AnalysisApplication::_finalize_event( Event * ep ) {
    AnalysisPipeline::_finalize_event( ep );
    notify_ascii_display();
}

// Interfacing application methods implementation

AnalysisApplication::AnalysisApplication( Config * vm ) :
            sV::AbstractApplication( vm ),
            mixins::PBEventApp(vm) {
    # if 0 // XXX (dev)
    // Inject custom stacktrace info acauizition into boost exception construction
    // procedures; TODO: doesn't work; may be here `handler' means "how to treat" the
    // exception, not "hook for user code to throw one"?)
    //init_boost_exception_handler();
    {
        namespace ga = ::goo::aux;
        # ifdef GOO_GDB_EXEC
        ga::iApp::add_handler(
                ga::iApp::_SIGABRT,
                ga::iApp::attach_gdb,
                "Attaches gdb to a process on on SIGABRT."
            );
        # endif
        # ifdef GOO_GCORE_EXEC
        ga::iApp::add_handler(
                ga::iApp::_SIGABRT,
                ga::iApp::dump_core,
                "Creates a core.<pid> coredump file on SIGABRT.",
                false
            );
        # endif
    }
    # endif

    _enable_ROOT_feature( mixins::RootApplication::enableCommonFile );

    vm->insertion_proxy()
        .p<std::string>('i', "input-file",
            "Input file --- an actual data source.") //.required_argument()? 
        .p<std::string>('F', "input-format",
            "Sets input file format.",
            "unset" )
        .list<std::string>('p', "processor",
            "Pushes processor in chain, one by one, in order.")
        .flag("list-src-formats",
            "Prints a list of available input file data formats.")
        .flag("list-processors",
            "Prints a list of available treatment routines.")
        .p<size_t>('n', "max-events-to-read",
            "Number of events to read; (set zero to read all available).",
            0 )
        ;
}

AnalysisApplication::~AnalysisApplication() {
    if( _evSeq ) {
        delete _evSeq;
        _evSeq = nullptr;
    }
    for( auto it  = _processorsChain.begin();
              it != _processorsChain.end(); ++it ) {
        delete *it;
    }
    _processorsChain.clear();
    if(gFile) {
        gFile->Write();
    }
}

# if 0
std::vector<goo::dict::Dictionary>
AnalysisApplication::_V_get_options() const {
    std::vector<goo::dict::Dictionary> res = Parent::_V_get_options();
    goo::dict::Dictionary analysisAppCfg("analysis", "Options related to analysis application.");
    { analysisAppCfg.insertion_proxy()
        .p<std::string>('i', "input-file",
            "Input file --- an actual data source.") //.required_argument()? 
        .p<std::string>('F', "input-format",
            "Sets input file format.",
            "unset" )
        .list<std::string>('p', "processor",
            "Pushes processor in chain, one by one, in order.")
        .flag("list-src-formats",
            "Prints a list of available input file data formats.")
        .flag("list-processors",
            "Prints a list of available treatment routines.")
        .p<size_t>('n', "max-events-to-read",
            "Number of events to read; (set zero to read all available).",
            0 )
        ;
    } res.push_back(analysisAppCfg);
    if( _suppOpts ) {
        for( auto it = _suppOpts->begin(); it != _suppOpts->end(); ++it ) {
            res.push_back( (*it)() );
        }
    }
    return res;
}
# endif

void
AnalysisApplication::_V_configure_concrete_app() {
    if( co()["list-src-formats"].as<bool>() ) {
        list_readers( std::cout );
        _immediateExit = true;
    }
    if( co()["list-processors"].as<bool>() ) {
        list_processors( std::cout );
        _immediateExit = true;
    }
    if( !do_immediate_exit()
                && _readersDict
                && cfg_option<std::string>("input-format") != "unset" ) {
        _evSeq = find_reader( cfg_option<std::string>("analysis.input-format") )();
    }
    if( !do_immediate_exit() && _procsDict ) {
        auto procNamesVect = co()["processor"].as_list_of<std::string>();
        for( auto it  = procNamesVect.begin();
                  it != procNamesVect.end(); ++it) {
            AnalysisPipeline::push_back_processor( find_processor(*it)() );
        }
    }
}

}  // namespace sV

# endif  // RPC_PROTOCOLS

