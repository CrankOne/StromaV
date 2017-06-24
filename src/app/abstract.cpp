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
# include "app/mixins/alignment.hpp"
# include "app/mixins/geant4.hpp"
# include "app/mixins/protobuf.hpp"
# include "app/mixins/root.hpp"
# include "yaml-adapter.hpp"
# include "detector_ids.h"
# include "utils.hpp"
# include "ctrs_dict.hpp"

# include <goo_versioning.h>
# include <goo_dict/parameters/path_parameter.hpp>

# include <fstream>

# include <dlfcn.h>
# include <dirent.h>

# include <yaml.h>

/**@defgroup app Appplications
 * @brief General OO infrastructure for applications.
 *
 * The abstract application descendants provide a facility to store information
 * about supported algorithms, detector systems, logging, configurations and
 * so on.
 *
 * In sV library the most basic class is the sV::AbstractApplication.
 * It defines the major configuration facility.
 *
 * The library provides a few of subclasses implementing additional
 * functionality for integration with ROOT or (and) Geant4 frameworks as well
 * as extra definitions for functional needs like alignment or pipeline-like
 * event analysis of experimental/modelled data.
 *
 * Examples:
 *  - @ref utilizing/application/my_application.cpp demonstrates basic usage
 *      of class inherited from AbstractApplication.
 * */

namespace sV {

//
// Abstract application
//////////////////////

AbstractApplication::ConstructableConfMapping *
AbstractApplication::ConstructableConfMapping::_self = nullptr;

AbstractApplication::ConstructableConfMapping &
AbstractApplication::ConstructableConfMapping::self() {
    if( !_self ) {
        _self = new AbstractApplication::ConstructableConfMapping();
    }
    return *_self;
}

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
            logging::Logger( "application",
                # if defined(TEMPLATED_LOGGING) && TEMPLATED_LOGGING
                "app-{{PID}}/{{this}}"
                # else
                "app"
                # endif
            ),
            _eStr(nullptr),
            _appCfg( cfg ),
            _configuration( "sV-config", "StromaV app-managed config" ),
            _immediateExit(false),
            _pathInterpolator( _configuration ) {
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
        .end_sect( "sV-paths" )
        .bgn_sect( "logging", "sV's logging facility configuration parameters." )
            .p<YAML::Node>( "families", "List of logging families. Each C++ "
                "class that is supposed to produce logging messages affiliated "
                "with particular instance must inherit the sV::logging::Logger "
                "class that, further, has to be included in a certain thematic "
                "group. Such groups are called \"families\" and may be "
                "parameterised using this config section hash. The keys become "
                "a name of particular group which must include at least the "
                "\"class\" entry referring to VCtr of \"iLoggingFamily\" "
                "descendants.")
        .end_sect( "logging" )
        ;
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

    int vrbst = conf["verbosity"].as<int>();
    if( vrbst > 3 ) {
        sV_loge("Invalid verbosity level specified: %d.\n", vrbst);
        vrbst = 3;
        sV_logw("Verbosity level %d was set.\n", vrbst);
    }
    const_cast<AbstractApplication*>(this)->verbosity(
                        (logging::LogLevel) vrbst );

    // TODO: acquire IO streams here!

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
AbstractApplication::_append_common_config_w_vctr( Config & ) {
    // Upon module loding is performed, one may fill common config with
    // supplementary options subsections. Each constructible has its own
    // configuration options definition stored in \ref IndexOfConstructables .
    // While the \ref ConstructableConfMapping defines options correspondance
    // with common config options. This information is used to build interim
    // configuration dictionaries within common config instance.
    ConstructableConfMapping & ccm = ConstructableConfMapping::self();
    for( auto injsPair : ccm.sections() ) {
        // Group of mappings related to one particular common base:
        const ConstructableConfMapping::Mappings & mappings = injsPair.second;
        const auto * constructables = sV::sys::IndexOfConstructables::self()
                                .constructors_for( injsPair.first )
                                ;
        if( !constructables ) {
            // It's ok --- they may be supplied within loadable modules that
            // for some reason wasn't loaded.
            sV_log3( "No virtual constructors available for section "
                    "\"%s\" (hash %zu).\n",
                    injsPair.second.baseName.c_str(), injsPair.first );
            continue;
        } else {
            sV_log3( "Adding parameters for section \"%s\" (with %zu entries) "
                "mapped to common config.\n", injsPair.second.baseName.c_str(),
                constructables->size() );
        }
        for( auto ctrs : *constructables ) {
            // Name of particular constructable entry:
            const std::string & entryName = ctrs.first;
            // Own config of particular constructable entry:
            const goo::dict::Dictionary & concreteCfg = ctrs.second->arguments;
            // Injection (mapping) object (common -> own):
            auto injIt = mappings.find(entryName);
            if( mappings.end() == injIt ) {
                sV_log3( "Arguments mapping for virtually-constructable object "
                    "\"%s\" (with base \"%s\" known to conf-mapping) is not "
                    "defined so its parameters won't be added to common "
                    "config.\n",
                    entryName.c_str(), mappings.baseName.c_str() );
                continue;
            }
            const ConstructableConfMapping::Injection inj = injIt->second;
            for( auto injectionPair : inj.injections() ) {
                const std::string & commonPath = injectionPair.first;
                goo::dict::DictionaryInjectionMap::MappedEntry & entry =
                                                        injectionPair.second;
                auto commonParameterPtr = _configuration.probe_parameter( commonPath );
                if( !commonParameterPtr ) {
                    goo::dict::Dictionary * targetSectionPtr = &_configuration;
                    // Create parameter by path "commonPath" of type
                    // concreteCfg[ownPath] with modifyed description:
                    std::size_t lDelim = commonPath.rfind( '.' );
                    if( std::string::npos != lDelim ) {
                        std::string sectPath = commonPath.substr(0, lDelim);
                        auto tSectPtr = _configuration.probe_subsection( sectPath );
                        if( tSectPtr ) {
                            targetSectionPtr = tSectPtr;
                        } else {
                            _configuration.insertion_proxy()
                                .bgn_sect( sectPath.c_str(), "" ).end_sect();
                            targetSectionPtr = &(_configuration.subsection( sectPath ));
                        }
                    } else {
                        lDelim = 0;
                    }
                    if( !entry.transformation ) {
                        // If transformation is not defined, guess identity
                        // mapping and preserve parameter type.
                        targetSectionPtr->insertion_proxy()
                                .insert_copy_of(
                                        concreteCfg.parameter(entry.path),
                                        commonPath.substr( lDelim+1 ).c_str()
                                    );
                    } else {
                        // The transformation is defined, but none common config
                        // entry was defined. It
                        // probably means that user code didn't yet
                        // appended the common conf dictionary with
                        // appropriate type. It is crucial for common entry to
                        // exist since type information has to be claimed.
                        emraise( badState, "For configuration mapping: "
                        "%s (common config) -> %s (local virtual ctr config) "
                        "user code defined paths and the transformation via "
                        "injection, but common config entry is not defined "
                        "upon mapping merge.",
                        commonPath.c_str(), entry.path.c_str() );
                    }
                } else {
                    // compare types for existing parameter
                    if( commonParameterPtr->target_type_info()
                        != concreteCfg.parameter( entry.path ).target_type_info() ) {
                        emraise( inconsistentConfig, "Type mismatch for VCtr "
                        "configuration dictionaries. Common config already has "
                        "parameter entry \"%s\" of type %s while the new mapping "
                        "coming from \"%s\":%s attempted to push parameter with "
                        "similar common path of type \"%s\" mapped to "
                        "local configuration path \"%s\".",
                            commonPath.c_str(), commonParameterPtr->target_type_info().name(),
                            entryName.c_str(), mappings.baseName.c_str(),
                            concreteCfg.parameter( entry.path ).target_type_info().name(),
                            entry.path.c_str() );
                    }
                    sV_log3( "Type for existing common config entry \"%s\" "
                        "matches to new one mapped withing vctr dict of "
                        "\"%s\"[%s] <- `%s`. Good.\n",
                        commonPath.c_str(), entryName.c_str(),
                        entry.path.c_str(), mappings.baseName.c_str() );
                }
            }
            # if 0
            // Construct temporary reverted map object.
            std::map<std::string, std::string> invInj;
            std::transform(
                    inj.injections().begin(), inj.injections().end(),
                    std::inserter( invInj, invInj.begin()),
                    // Lambda function flipping pair:
                    [](const std::pair<std::string, std::string> op){
                            return std::make_pair( op.second, op.first );
                        }
                );
            # endif
        }
    }

    # if 0
    //const AnalysisDictionary * ad = dynamic_cast<const AnalysisDictionary *>(this);
    if( /*ad &&*/ IndexOfConstructables::empty() ) {
        uint32_t n = 0;
        for( auto append : *AnalysisDictionary::supp_options() ) {
            append( _configuration );
            ++n;
        }
        sV_log2( "%u analysis subsections appended to common "
                "configuration.\n", n );
    }
    # endif
    // ...
}

/**
 * The application instances willing to inject additional parameters and
 * sections the common config may override this method. The common config
 * parameters has to be injected in common config before calling to parent
 * (this, AbstractApplication's) _V_configure_application():
 *
 * @snippet examples/utilizing/application/my_application.cpp Appending common configuration
 *
 * The `const * appCfgPtr` ptr refers to application config and may be used for
 * conditional composition of common config.
 *
 * @sa _V_concrete_app_configure()
 */
void
AbstractApplication::_V_concrete_app_append_common_cfg() {
    // Default implementation is just an empty stub.
}


/**
 * User application should be configured here.
 *
 * @snippet examples/utilizing/application/my_application.cpp Setting up common configuration 
 *
 * @sa _V_concrete_app_append_common_cfg()
 */
void
AbstractApplication::_V_concrete_app_configure() {
    // Default implementation is just an empty stub.
}

/**
 * This implementation of application configuration method is qualified as
 * "final" to prevent one from changing its (very basic) logic. User code has
 * to consider use of _V_concrete_app_append_common_cfg() and
 * _V_concrete_app_configure() methods to inject specific common-config
 * options.
 *
 * Method performes the following activity, in order:
 *
 * At the pre-configuration stage:
 *  1. injection of additional parameters to common config provided by mixins
 *    and user app (with _V_concrete_app_append_common_cfg()).
 *  2. injection of additional parameters brought by virtual constructor entries
 *    by subsequent invokation of _append_common_config_w_vctr() method on
 *    common config.
 *  3. injection of user-defined (descendant class) parameters within probably
 *    overriden method _V_append_concrete_app_config_parameters().
 *     
 * At the configuration stage:
 *  1. Load and parse configuration files by method _parse_configs() into the
 *    common config dictionary.
 *  2. Supersede common config parameters provided by command line
 *    `--override-opt` option argument.
 *  3. If `--inspect-config` option was provided, the dumps the common config
 *    into standard output (std::cout) and sets the `_immediateExit` flag.
 *
 * Finally, the applying configuration stage (what is called actual
 * "configuration" in other parts of program):
 *  1. Re-initialization of sV's logging infrastructure with invokation of
 *    logging::iLoggingFamily::initialize_families().
 *  2. Invokation of various config-applying methods in mixins included in
 *    inheritance chain of the final descendant class.
 *  3. invokation of _V_concrete_app_configure() to apply user-defined
 *    configuration parameters on application.
 *  4. Initialization of ASCII display, if it was demanded from app-config.
 */
void
AbstractApplication::_V_configure_application( const Config * appCfgPtr ) {
    const Config & appConf = *appCfgPtr;
    if( _immediateExit ) {
        return;
    }

    //
    // 1. PRE-CONFIGURATION: Supplementary configuration injections:
    // 1.1. Append mixins configuration:
    mixins::RootApplication * myRootMixin =
            dynamic_cast<mixins::RootApplication *>(this);
    mixins::Geant4Application * myGeant4Mixin =
            dynamic_cast<mixins::Geant4Application *>(this);
    if( myRootMixin ) {
        myRootMixin->_append_ROOT_config_options( _configuration );
    }
    if( myGeant4Mixin ) {
        myGeant4Mixin->_append_Geant4_config_options( _configuration );
    }
    // ... more mixins here.
    // 1.2. Injections from VCtr entries
    _append_common_config_w_vctr( _configuration );
    // 1.3. Finally, inject the conrete app parameters:
    _V_concrete_app_append_common_cfg();

    sV_log2( "Configuration 1/3: dictionary has been built.\n" );

    //
    // 2. CONFIGURATION:
    // 2.1. Load configuration files
    _parse_configs( appConf["config"].as<goo::filesystem::Path>() );
    // 2.2. Override configuration with command-line ones specified with
    // -O|--override-opt
    const auto & overridenOpts = appConf["override-opt"].as_list_of<std::string>();
    for( const auto & overridenOpt : overridenOpts ) {
        std::size_t eqPos = overridenOpt.find('=');
        if( std::string::npos == eqPos ) {
            emraise( badParameter, "Unable to interpret expression "
                "\"%s\" (no equal sign found). Please, use the notation "
                "\"-O<opt-name>=<opt-val>\" to override option from common "
                "config.", overridenOpt.c_str() );
        }
        std::string path = overridenOpt.substr(0, eqPos),
                    strVal = overridenOpt.substr(eqPos+1)
                    ;
        _set_common_option( path, strVal );
    }
    sV_log2( "Configuration 2/3: parameters dictionary ready.\n" );
    // 2.3 Dump config, if need:
    if( appConf["inspect-config"].as<bool>() ) {
        // TODO: support various help renderers from Goo.
        _configuration.print_ASCII_tree( std::cout );
        _immediateExit = true;
    }

    {   // Apply logging families configuration at this point.
        size_t nFamsInitd = logging::iLoggingFamily::initialize_families();
        sV_log1( "Existing %zu logging families have been re-initialized.\n",
                 nFamsInitd );
    }
    // Configure mixins:
    if( myRootMixin && !do_immediate_exit() ) {
        myRootMixin->_initialize_ROOT_system( _configuration );
    }
    if( myGeant4Mixin && !do_immediate_exit() ) {
        myGeant4Mixin->_initialize_Geant4_system( _configuration );
    }
    // ... more mixins here.
    // Configure particular application:
    _V_concrete_app_configure();
    // Configure ASCII display prior to run:
    if( _appCfg->parameter("ascii-display").as<bool>() ) {
        aux::ASCII_Display::enable_ASCII_display();
    }
    sV_log2( "Configuration 3/3: application configured.\n" );
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

    std::ostream * lStrPtr = new std::ostream( &_lBuffer );
    // extGDML::Journal::self().warn_stream( _eStr );
    // extGDML::Journal::self().info_stream( *lStrPtr );
    return lStrPtr;
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
    sV_log3( "Option \"%s\" overriden from command line with \"%s\".\n",
        path.c_str(), _configuration
            .goo::dict::Dictionary::parameter( path.c_str() )
            .to_string().c_str() );
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

