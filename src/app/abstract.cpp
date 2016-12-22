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

# include "app/abstract.hpp"
# include "app/mixins/root.hpp"
# include "detector_ids.h"
# include "utils.hpp"

# include <fstream>
# include <goo_versioning.h>

/**@defgroup app Appplications
 * @brief General infrastructure of applications.
 *
 * The abstract application classes provides a facility to store information
 * about supported algorithms, detector systems, logging, configurations and
 * so on.
 *
 * In sV library the most basic class is the sV::AbstractApplication
 * defining the major configuration facility. The library provides a few of
 * subclasses implementing additional functionality to be integrated with
 * ROOT or Geant4 or for functional needs like alignment or pipeline-like
 * event analysis of experimental/modelled data.
 * */

namespace sV {

//
// Abstract application
//////////////////////

# if 0
const char * _static_procName = nullptr;
void __static_custom_segfault_handler( int signum ) {
    if( !_static_procName ) {
        fprintf( stderr, "Couldn't intercept SEGFAULT as appname wasn't set yet.\n" );
        abort();
    }
    char buf[128]; 
    snprintf( buf, sizeof(buf), "gcore %d", getpid() );
    system(buf);
    snprintf( buf, sizeof(buf), "gdb -p %d", getpid() );
    system(buf);
    snprintf( buf, sizeof(buf), "kill -SIGABRT %d", getpid() );
    system(buf);
}
# endif

AbstractApplication::AbstractApplication( Config * cfg ) :
            _eStr(nullptr),
            _variablesMap( cfg ),
            _immediateExit(false),
            _verbosity(1),
            _ROOTAppFeatures(0x0) {
    // Initialize log dispatching to standard streams by
    // default:
    // (todo: may be it is useful to prevent it by getenv(NO_OUT)?)
    _lBuffer.add_target( &std::cout );
    _eBuffer.add_target( &std::cerr );

    namespace ga = ::goo::aux;

    # ifdef GOO_GDB_EXEC
    ga::iApp::add_environment_variable(
        ATTACH_GDB_ON_SEGFAULT_ENVVAR,
        "Whether to attach the gdb debugger to the process when SIGSEGV "
        "signal caught."
    );
    if( ::goo::aux::iApp::envvar_as_logical( ATTACH_GDB_ON_SEGFAULT_ENVVAR ) ) {
            ga::iApp::add_handler(
            ga::iApp::_SIGSEGV,
            ga::iApp::attach_gdb,
            "Attaches gdb to a process after SIGSEGV."
        );
    }
    # endif

    # ifdef GOO_GCORE_EXEC
    ga::iApp::add_environment_variable(
        PRODUCE_COREDUMP_ON_SEGFAULT_ENVVAR,
        "Whether to produce core dump when SIGSEGV signal received."
    );
    if( ::goo::aux::iApp::envvar_as_logical( PRODUCE_COREDUMP_ON_SEGFAULT_ENVVAR ) ) {
        ga::iApp::add_handler(
            ga::iApp::_SIGSEGV,
            ga::iApp::dump_core,
            "Creates a core.<pid> coredump file in CWD.",
            false
        );
    }
    # endif
}

void
AbstractApplication::_enable_ROOT_feature( uint8_t ftCode ) {
    _ROOTAppFeatures |= ftCode;
}

AbstractApplication::Config *
AbstractApplication::_V_construct_config_object( int argc, char * const argv [] ) const {
    po::options_description genericCfg,
                            rootAppCfg("CERN ROOT framework extras")
                            ;
    { genericCfg.add_options()
        ("help,h",
            "Prints this help message and exits.")
        ("config,c",
            po::value<std::string>()->default_value(StromaV_DEFAULT_CONFIG_FILE_PATH),
            "Sets the config file to command line options be interfereded with.")
        ("verbosity,v",
            po::value<int>()->default_value(1),
            "Sets verbosity level (from 0 -- silent, to 3 -- loquacious).")
        ("build-info",
            "Prints out current build information and exits.")
        ("logfile",
            po::value<std::string>()->default_value("stdout"),
            "Stream for logging. Use \"stdout\" for stdout.")
        ("errfile",
            po::value<std::string>()->default_value("stderr"),
            "Stream for error logging. Use \"stderr\" for stderr.")
        ("ascii-display,A",
            po::value<bool>()->default_value(false),
            "When enabled, can print to terminal some info updated in real-time. "
            "Not useful with loquacious verbosity. Not supported by all the "
            "sV applications.")
        ;
    }

    po::options_description opts;
    auto optsVect = _V_get_options();
    opts.add(genericCfg);
    if( _ROOTAppFeatures ) {
        mixins::RootApplication::append_ROOT_config_options( rootAppCfg, _ROOTAppFeatures );
        opts.add(rootAppCfg);
    }
    for( auto it = optsVect.begin(); optsVect.end() != it; ++it ) {
        opts.add(*it);
    }
    po::store(po::command_line_parser(argc, argv)
        .options(opts)
        .run(),
        *_variablesMap);
    // ...
    po::variables_map & vm = *_variablesMap;
    std::string confFilePath;

    _verbosity = vm["verbosity"].as<int>();
    if( _verbosity > 3 ) {
        std::cout << "Invalid verbosity level specified: " << (int) _verbosity;
        _verbosity = 3;
        std::cerr << ". Set to " << (int) _verbosity << "." << std::endl;
    }

    if( vm.count("config") ) {
        confFilePath = vm["config"].as<std::string>();
        if( verbosity() > 1 ) {
            std::cerr << "Using user's config file: " << confFilePath << std::endl;
        }
    } else {
        if( verbosity() > 0 ) {
            std::cerr << "Using default config file: " << confFilePath << std::endl;
        }
    }
    std::ifstream configFile( confFilePath, std::ifstream::in );
    if( ! configFile ) {
        emraise( fileNotReachable, "Couldn't open config file \"%s\".", confFilePath.c_str() );
    }
    po::store( po::parse_config_file( configFile, opts, /*allow_unregistered*/ true ), vm );
    configFile.close();

    try {
        po::notify( vm );
    } catch ( const std::exception & e ) {
        std::cerr << "variables_map::notify() Error: " << e.what() << std::endl;
    }
    
    if(vm.count("help")) {
        std::cout << opts << std::endl;
        std::cout << "Environment variables:" << std::endl;
        dump_envvars( std::cout );
        _immediateExit = true;
    }

    return _variablesMap;
}

void
AbstractApplication::_V_configure_application( const Config * cfg ) {
    _process_options( cfg );
}

std::ostream *
AbstractApplication::_V_acquire_stream() {
    if( cfg_option<std::string>("logfile") != "stdout" ) {
        _lBuffer.add_target( new std::ofstream(cfg_option<std::string>("logfile")),
                             false );
    }

    // TODO: Goo API must provide err stream acquizition method.
    _eStr = _V_acquire_errstream();

    return new std::ostream( &_lBuffer );
}

std::ostream *
AbstractApplication::_V_acquire_errstream() {
    if( cfg_option<std::string>("errfile") != "stderr" ) {
        _eBuffer.add_target( new std::ofstream(cfg_option<std::string>("errfile")),
                             false );
    }

    return new std::ostream( &_eBuffer );
}

std::vector<po::options_description>
AbstractApplication::_V_get_options() const {
    return std::vector<po::options_description>();
}

void
AbstractApplication::_process_options( const Config * vmPtr ) {
    const Config & vm = *vmPtr;

    if(vm.count("build-info")) {
        dump_build_info( std::cout );
        _immediateExit = true;
    }

    if( _ROOTAppFeatures ) {
        mixins::RootApplication::initialize_ROOT_system( _ROOTAppFeatures );
    }


    # if 0
    {   // Try dynamic_cast<RootApplication*>() and initialize ROOT's mixin:
        mixins::RootApplication * ra;
        if( NULL != (ra = dynamic_cast<mixins::RootApplication *>(this)) ) {
            // upon successful cast:
            const std::string & cmdArgs = vm["TApplication-args"].as<std::string>();
            if( !cmdArgs.empty() ) {
                // TODO: to be replaced by goo's ::goo::dict::Configuration::tokenize_string()
                // or ::goo::dict::Configuration::free_tokens()
                // when goo/appParameters branch will be merged to goo/master:
                ra->_t_argc = ::sV::aux::goo_XXX_tokenize_argstring(
                    cmdArgs,
                    ra->_t_argv );
                ra->create_TApplication(
                        &(ra->_t_argc),
                        ra->_t_argv,
                        nullptr,
                        0 );
            } else {
                ra->create_TApplication();
            }
        }
    }

    if(vmPtr->count("root.dynamic-path") && !_immediateExit ) {
        for( auto additionalPath : cfg_option<std::vector<std::string>>("root.dynamic-path") ) {
            if( !additionalPath.empty() ) {
                gSystem->AddDynamicPath( additionalPath.c_str() );
            }
        }
    }
    # endif

    _V_configure_concrete_app();

    # if 0

    # endif

    if( cfg_option<bool>( "ascii-display" ) ) {
        aux::ASCII_Display::enable_ASCII_display();
    }
}

void
AbstractApplication::dump_build_info( std::ostream & os ) const {
    const unsigned long features = 0
    # define set_bit_if_enabled( m ) | ( 1 << m )
    # ifdef             GEANT4_MC_MODEL
    set_bit_if_enabled( GEANT4_MC_MODEL )
    # endif
    # ifdef             G4_MDL_VIS
    set_bit_if_enabled( G4_MDL_VIS )
    # endif
    # ifdef             G4_MDL_GUI
    set_bit_if_enabled( G4_MDL_GUI )
    # endif
    ;

    os << "StromaV lib build info:" << std::endl
       << "                   version ... : " << STROMAV_VERSION_MAJOR
                                              << "." << STROMAV_VERSION_MINOR << std::endl
       << "                build type ... : " << STRINGIFY_MACRO_ARG(STROMAV_BUILD_TYPE) << std::endl
       << "              build system ... : " << STRINGIFY_MACRO_ARG(STROMAV_CMAKE_SYSTEM) << std::endl
       << "  default config file path ... : " << StromaV_DEFAULT_CONFIG_FILE_PATH << std::endl
       << "          featurs hex code ... : " << std::hex << "0x" << features << std::endl
       // TODO ... etc.
       ;
    
    char * bf;
    size_t bfLength;
    FILE * memfile = open_memstream( &bf, &bfLength );
    goo_build_info( memfile );
    ::fclose( memfile );
    os << bf;
    free(bf);
}

static const char _static_logPrefixes[][64] = {
    "[" ESC_BLDRED "EE" ESC_CLRCLEAR "] ",
    "[" ESC_BLDYELLOW "WW" ESC_CLRCLEAR "] ",
    "[" ESC_BLDBLUE "L1" ESC_CLRCLEAR "] ",
    "[" ESC_CLRCYAN "L2" ESC_CLRCLEAR "] ",
    "[" ESC_CLRBLUE "L3" ESC_CLRCLEAR "] ",
    "[" ESC_CLRCYAN "??" ESC_CLRCLEAR "] ",
};

const char *
AbstractApplication::_get_prefix_for_loglevel( int8_t l ) {
    switch(l) {
        case -2: return _static_logPrefixes[0];
        case -1: return _static_logPrefixes[1];
        case  1: return _static_logPrefixes[2];
        case  2: return _static_logPrefixes[3];
        case  3: return _static_logPrefixes[4];
        default: return _static_logPrefixes[5];
    };
}

void
AbstractApplication::message( int8_t level, const std::string msg, bool noprefix ) {
    if( level <= verbosity() ) {
        const char noprefixPrefix[] = "";
        const char * prefix = noprefix ? noprefixPrefix : _get_prefix_for_loglevel(level);
        std::ostream * os = &(level > 0 ? ls() : es());
        *os << prefix << msg;
        //if( level < 0 ) {
            os->flush();
        //}
    }
}

}  // namespace sV

