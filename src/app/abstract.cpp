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
# include "yaml-adapter.hpp"
# include "detector_ids.h"
# include "utils.hpp"

# include <fstream>
# include <goo_versioning.h>
# include <dlfcn.h>
# include <dirent.h>

# include <yaml.h>

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
            _appCfg( cfg ),
            _configuration( "sV-config", "StromaV app-managed config" ),
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
    // This is a pre-initialization step that assembles the common
    // configuration dictionaries from descendants specifications. It can cause
    // immediate exit if some particular flags were provided (help, build-info,
    // etc.).
    Config & conf = *_appCfg;
    conf.insertion_proxy()
        .flag( 'h', "help",
                "Prints help message and exits." )
        .flag( "build-info",
                "Prints build configuration info and exits." )
        .p<std::string>( 'c', "config",
                "Path to .yml-configuration.", StromaV_DEFAULT_CONFIG_FILE_PATH )
        .list<std::string>( 'O', "override-opt",
                "Overrides config entry in common config options set." )
        .p<int>('v', "verbosity",
                "Sets common verbosity level. [0..3] values are allowed.", 1 )
        .p<std::string>( "stdout",
                "Redirects internal logging messages to stream. Use '-' for "
                "stdout (default).", "-")
        .p<std::string>( "stderr",
                "Redirects internal error logging messages to stream. Use '-' "
                "for stderr (default).", "-")
        .list<std::string>( 'l', "load-module",
                "Dynamically loads a third-party library. An interfacing "
                "option to set-up sV plugins.")
        .flag( 'A', "ascii-display",
                "When enabled, can print to terminal some info updated in "
                "real-time. Not useful with loquacious verbosity. Not "
                "supported by all the sV applications.")
        ;
    auto optsVect = _V_get_options();
    for( auto p : optsVect ) {
        conf.append_section( p );
    }

    if( conf.extract( argc, argv, true ) < 0) {
        _immediateExit = true;
    }
    
    if( conf["h"].as<bool>() ) {
        conf.usage_text( std::cout, argv[0] );
        std::cout << "Environment variables:" << std::endl;
        dump_envvars( std::cout );
        _immediateExit = true;
        return _appCfg;
    }

    if( conf["build-info"].as<bool>() ) {
        dump_build_info( std::cout );
        _immediateExit = true;
    }

    _verbosity = conf["verbosity"].as<int>();
    if( _verbosity > 3 ) {
        std::cerr << "Invalid verbosity level specified: " << (int) _verbosity;
        _verbosity = 3;
        std::cerr << ". Level " << (int) _verbosity << " was set." << std::endl;
    }

    const auto & ll = conf["load-module"].as_list_of<std::string>();
    for( const auto & dlPath : ll ) {
        void * handle = dlopen( dlPath.c_str(), RTLD_NOW );
        if( !handle ) {
            std::cerr << "Failed to load \"" << dlPath << "\"" << std::endl;
        } else {
            std::cout << "Shared object file \"" << dlPath 
                      << "\" loaded." << std::endl;
        }
    }

    return _appCfg;
}

void
AbstractApplication::_V_configure_application( const Config * cfg ) {
    if( _immediateExit ) {
        return;
    }

    if( ROOT_features() ) {
        mixins::RootApplication
              ::append_ROOT_config_options( _configuration, ROOT_features() );
    }

    const Config & conf = *cfg;
    // Here, we have pre-initialization stage done: all modules loaded and
    // we're ready for assembling up common configuration dictionary:
    _V_append_common_config( _configuration );

    // Load configuration files
    _parse_configs( conf["config"].as<std::string>() );

    // Override configuration with command-line ones specified with
    // -O|--override-opt
    const auto & overridenOpts = conf["override-opt"].as_list_of<std::string>();
    for( const auto & overridenOpt : overridenOpts ) {
        std::size_t eqPos = overridenOpt.find('=');
        if( std::string::npos == eqPos ) {
            emraise( badParameter, "Unable to interpret expression "
                "\"%s\" (no equal sign in token). Please, use the notation "
                "\"-O<opt-name>=<opt-val>\" to override option from common "
                "config.", overridenOpt.c_str() );
        }
        std::string path = overridenOpt.substr(0, eqPos),
                    strVal = overridenOpt.substr(eqPos)
                    ;
        _set_common_option( path, strVal );
    }

    _process_options( cfg );
}

std::ostream *
AbstractApplication::_V_acquire_stream() {
    if( _immediateExit ) {
        return nullptr;
    }

    if( _appCfg->parameter("stdout").as<std::string>() != "-" ) {
        _lBuffer.add_target(
                new std::ofstream(_appCfg->parameter("logfile").as<std::string>()),
                false );
    }

    // TODO: Goo API must provide err stream acquizition method.
    _eStr = _V_acquire_errstream();

    return new std::ostream( &_lBuffer );
}

std::ostream *
AbstractApplication::_V_acquire_errstream() {
    if( _appCfg->parameter("stderr").as<std::string>() != "-" ) {
        _eBuffer.add_target(
                new std::ofstream(_appCfg->parameter("errfile").as<std::string>()),
                false );
    }

    return new std::ostream( &_eBuffer );
}

std::vector<goo::dict::Dictionary>
AbstractApplication::_V_get_options() const {
    return std::vector<goo::dict::Dictionary>();
}

void
AbstractApplication::_process_options( const Config * ) {

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

    if( _appCfg->parameter("ascii-display").as<bool>() ) {
        aux::ASCII_Display::enable_ASCII_display();
    }
}

void
AbstractApplication::_parse_configs( const std::string & path ) {
    struct stat pStat;
    if( 0 == ::stat( path.c_str(), &pStat ) ) {
        if( pStat.st_mode & S_IFDIR ) {
            // it's a directory
            DIR * dp;
            struct dirent * dirPtr;
            if( (dp  = opendir(path.c_str())) == NULL ) {
                emraise( ioError, "Unable to open dir \"%s\": %s.",
                    path.c_str(), strerror(errno) );
            }
            std::set<std::string> filesToParse;
            while( (dirPtr = readdir(dp)) != NULL ) {
                // Check if file name matches the pattern:
                std::string fName( dirPtr->d_name );
                std::size_t dotP = fName.find('.');
                if( dotP != std::string::npos ) {
                    std::string tail = fName.substr( dotP );
                    if( ".yml" == tail || ".yaml" == tail || ".conf" == tail ) {
                        filesToParse.insert( fName );
                    }
                }
            }
            if( filesToParse.empty() ) {
                emraise( badParameter, "Unable to find config files at \"%s\"."
                    "Expected at least one file with .yml/.yaml/.conf name postfix "
                    "(extension).", path.c_str() );
            }
            for( const auto & p : filesToParse ) {
                _parse_config_file( p );
            }
        } else if( pStat.st_mode & S_IFREG ) {
            // it's a file
            _parse_config_file( path );
        } else {
            emraise( fileNotReachable, "Unable to interpret stat() for "
                "path \"%s\". It is not a file, nor a directory.", path.c_str() );
        }
    } else {
        emraise( ioError, "Unable to retrieve stat() for path \"%s\": %s.",
            path.c_str(), strerror(errno) );
    }
}

void
AbstractApplication::_parse_config_file( const std::string & path ) {
    aux::read_yaml_config_file_to_goo_dict( _configuration, path );
}

void
AbstractApplication::_set_common_option( const std::string & path,
                                         const std::string & strVal ) {
    _configuration
        .goo::dict::Dictionary::parameter( path.c_str() )
        .parse_argument( strVal.c_str() )
        ;
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

