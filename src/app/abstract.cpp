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
# include "analysis/dictionary.hpp"
# include "yaml-adapter.hpp"
# include "detector_ids.h"
# include "utils.hpp"

# include <goo_versioning.h>
# include <goo_dict/parameters/path_parameter.hpp>

# include <fstream>

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
            _verbosity(3),
            _pathInterpolator( _configuration ),
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

    // Assembles base application configuration dictionary. Descendants has to
    // further append the config object if they wish to introduce some specific
    // options like buildconf, list entries, load .so-libraries, etc.
    _appCfg->insertion_proxy()
        .flag( 'h', "help",
                "Prints help message and exits." )
        .flag( "build-info",
                "Prints build configuration info and exits." )
        .p<goo::filesystem::Path>( 'c', "config",
                "Path to .yml-configuration.", StromaV_DEFAULT_CONFIG_PATH )
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
        .list<goo::filesystem::Path>( 'l', "load-module",
                "Dynamically loads a third-party library. An interfacing "
                "option to set-up sV plugins." )
        .flag( 'A', "ascii-display",
                "When enabled, can print to terminal some info updated in "
                "real-time. Not useful with loquacious verbosity. Not "
                "supported by all the sV applications.")
        .flag( "inspect-config",
                "Will dump config state upon all modules (.so referenced with "
                "-l|--load-module) will be loaded, config file(s) parsed and "
                "(possibly) overriden with command line (with "
                "-O|--override-opt option). The config dump will be "
                "accompanied with descriptive reference information for each "
                "parameter. Application will be terminated after dump.")
        ;

    // This paths are related to StromaV and may be used for in-config
    // substitution.
    _configuration.insertion_proxy()
        .bgn_sect( "sV-paths", "StromaV system paths." )
            .p<goo::filesystem::Path>( "plugins",
                            "Path to StromaV plugins directory",
                            StromaV_MODULES_INSTALL_PATH )
            .p<goo::filesystem::Path>( "assets",
                            "Path to StromaV assests directory" )
            .p<goo::filesystem::Path>( "root-plugins",
                            "Path to sV's ROOT plugins dir." )
        .end_sect( "sV-paths" )
        ;
}

void
AbstractApplication::_enable_ROOT_feature( uint8_t ftCode ) {
    _ROOTAppFeatures |= ftCode;
}

AbstractApplication::Config *
AbstractApplication::_V_construct_config_object( int argc, char * const argv [] ) const {
    Config & conf = *_appCfg;
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
        sV_loge("Invalid verbosity level specified: %d.\n", (int) _verbosity);
        _verbosity = 3;
        sV_logw("Verbosity level %d was set.\n", (int) _verbosity);
    }

    auto & ll = conf["load-module"].as_list_of<goo::filesystem::Path>();
    for( const goo::filesystem::Path & dlPath : ll ) {
        dlPath.interpolator( path_interpolator() );
        const std::string intrpltdPath = dlPath.interpolated();
        if( intrpltdPath.empty() ) {
            sV_logw( "Empty load path ignored.\n" );
            continue;
        }
        // TODO: try too look-up provided paths relatively to sV-paths.plugins
        // location?
        void * handle = dlopen( intrpltdPath.c_str(), RTLD_NOW );
        if( !handle ) {
            sV_loge( "Failed to load module: %s.\n", dlerror() );
        } else {
            sV_log1( "Module \"%s\" loaded.\n", intrpltdPath.c_str() );
        }
    }
    return _appCfg;
}

void
AbstractApplication::_append_common_config() {
    // Upon module loding is performed, one may fill common config with
    // supplementary options subsections. Here we use dynamic_cast<>() to
    // determine whether this application (in its final descendant form)
    // common config dictionary needs corresponding supplementary options.
    // One may desire to append this list with various classes.
    # ifdef RPC_PROTOCOLS
    const AnalysisDictionary * ad = dynamic_cast<const AnalysisDictionary *>(this);
    if( ad && AnalysisDictionary::supp_options() ) {
        uint32_t n = 0;
        for( auto append : *AnalysisDictionary::supp_options() ) {
            append( _configuration );
            ++n;
        }
        sV_log2( "%u analysis subsections loaded.\n", n );
    }
    # endif
    // ...
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

    // Before config files will be parsed --- finalize the common config.
    _append_common_config();

    // Load configuration files
    _parse_configs( conf["config"].as<goo::filesystem::Path>() );

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
                    strVal = overridenOpt.substr(eqPos+1)
                    ;
        _set_common_option( path, strVal );
    }

    _process_options( cfg );

    if( conf["inspect-config"].as<bool>() ) {
        // TODO: support various help renderers from Goo.
        _configuration.print_ASCII_tree( std::cout );
        _immediateExit = true;
    }
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

void
AbstractApplication::_process_options( const Config * ) {
    if( _ROOTAppFeatures && !do_immediate_exit() ) {
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
AbstractApplication::_parse_configs( const goo::filesystem::Path & path ) {
    # if 1
    if( !path.exists() ) {
        emraise( ioError, "Path \"%s\" doesn't exist.", path.c_str() );
    }
    if( path.is_file() ) {
        _parse_config_file( path );
    } else if( path.is_dir() ) {
        DIR * dp;
        struct dirent * dirPtr;
        if( (dp  = opendir(path.c_str())) == NULL ) {
            emraise( ioError, "Unable to open dir \"%s\": %s.",
                path.c_str(), strerror(errno) );
        }
        std::set<std::string> filesToParse;
        while( (dirPtr = readdir(dp)) != NULL ) {
            // Check if file name matches the pattern:
            std::string fName( path.concat(dirPtr->d_name) );
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
    }
    # else
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
    # endif
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
       << "  default config file path ... : " << StromaV_DEFAULT_CONFIG_PATH << std::endl
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

std::string
AbstractApplication::ConfigPathInterpolator::interpolate( const std::string & p ) const {
    std::size_t bgnPos, endPos, bgnBrckt;
    std::string intrpltdCpy(p);
    while( std::string::npos != (bgnPos = intrpltdCpy.find("$(CFG:")) ||
           std::string::npos != (bgnBrckt = intrpltdCpy.find("$(")) ) {
        if( std::string::npos == 
                (endPos = intrpltdCpy.find( ')', bgnPos == std::string::npos
                                                 ? bgnBrckt : bgnPos )) ) {
            emraise( badParameter, "Unable to interpolate "
                "path expression \"%s\" (original: \"%s\"): "
                "unbalanced bracket at position %zu.",
                intrpltdCpy.c_str(), p.c_str(), bgnPos == std::string::npos
                                                ? bgnBrckt : bgnPos );
        }
        if( std::string::npos != bgnPos ) {
            // Substitute config parameter:
            std::string cfgPath = intrpltdCpy.substr(bgnPos + 6, endPos - bgnPos - 6),
                        value = _cfgRef.parameter(cfgPath.c_str()).to_string()
                        ;
            intrpltdCpy.replace( bgnPos, endPos - bgnPos + 1, value );
        } else {
            // Substitute environment variable:
            std::string envVarName = intrpltdCpy.substr(bgnBrckt + 2, endPos - bgnBrckt - 2);
            intrpltdCpy.replace( bgnBrckt, endPos - bgnBrckt + 1,
                                 goo::aux::iApp::envvar( envVarName ) );
        }
    }
    return intrpltdCpy;
}

}  // namespace sV

