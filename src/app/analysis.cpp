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

void
AnalysisApplication::push_back_processor( const std::string & name ) {
    auto p = find_processor( name )();
    sV_log3( "Adding processor %s:%p.\n", name.c_str(), p );
    AnalysisPipeline::push_back_processor( p );
}

void
AnalysisApplication::_finalize_event( Event * ep ) {
    AnalysisPipeline::_finalize_event( ep );
    notify_ascii_display();
}

// Interfacing application methods implementation

AnalysisApplication::AnalysisApplication( Config * vm ) :
            sV::AbstractApplication( vm ),
            mixins::PBEventApp(vm) {
    _enable_ROOT_feature( mixins::RootApplication::enableCommonFile );
    // Inject custom stacktrace info acauizition into boost exception construction
    // procedures; TODO: doesn't work; may be here `handler' means "how to treat" the
    // exception, not "hook for user code to throw one"?)
    //init_boost_exception_handler();

    // XXX (dev)
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

std::vector<po::options_description>
AnalysisApplication::_V_get_options() const {
    std::vector<po::options_description> res = Parent::_V_get_options();
    po::options_description analysisAppCfg;
    { analysisAppCfg.add_options()
        ("input-file,i",
            po::value<std::string>(),
            "Input file --- an actual data source.")
        ("input-format,F",
            po::value<std::string>()->default_value("unset"),
            "Sets input file format.")
        ("processor,p",
            po::value<std::vector<std::string>>(),
            "Pushes processor in chain, one by one, in order.")
        ("list-src-formats",
            "Prints a list of available input file data formats.")
        ("list-processors",
            "Prints a list of available treatment routines.")
        ("max-events-to-read,n",
            po::value<size_t>()->default_value(0),
            "Number of events to read; (set zero to read all available).")
        ;
    } res.push_back(analysisAppCfg);
    if( _suppOpts ) {
        for( auto it = _suppOpts->begin(); it != _suppOpts->end(); ++it ) {
            res.push_back( (*it)() );
        }
    }
    return res;
}

void
AnalysisApplication::_V_configure_concrete_app() {
    po::variables_map & vm = co();
    if( vm.count("list-src-formats") ) {
        list_readers( std::cout );
        _immediateExit = true;
    }
    if( vm.count("list-processors") ) {
        list_processors( std::cout );
        _immediateExit = true;
    }
    if( !do_immediate_exit() && _readersDict && cfg_option<std::string>("input-format") != "unset" ) {
        _evSeq = find_reader( cfg_option<std::string>("input-format") )();
    }
    if( !do_immediate_exit() && _procsDict ) {
        auto procNamesVect = cfg_option<std::vector<std::string>>("processor");
        for( auto it  = procNamesVect.begin();
                  it != procNamesVect.end(); ++it) {
            push_back_processor( *it );
        }
    }
    //if( !cfg_option<std::string>("root-file").empty() ) {
    //    new TFile( cfg_option<std::string>("root-file").c_str(), "RECREATE" );
    //}
}

}  // namespace sV

# endif  // RPC_PROTOCOLS

