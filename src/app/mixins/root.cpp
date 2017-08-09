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

# include "app/mixins/root.hpp"
# include "utils.hpp"

# include <TApplication.h>
# include <TSystem.h>
# include <TPluginManager.h>
# include <TEnv.h>
# include <TROOT.h>
# include <TFile.h>
# include <TSystem.h>

# include <goo_app.hpp>
# include <goo_dict/parameters/path_parameter.hpp>
# include <goo_dict/configuration.hpp>

static void __static_disable_damn_root_handlers() __attribute__ ((constructor(101)));
void __static_disable_damn_root_handlers() {
    ::goo::aux::iApp::add_environment_variable(
        PRESERVE_ROOT_SIGNAL_HANDLERS_ENVVAR,
        "(logic var) Controls whether to keep ROOT system signal handlers."
        "Note, that setting it to false does not prevent application from "
        "invokation of other handlers before the ROOT ones.");
    // Default resetting signal handlers behaviour can be overriden by
    // environment variable which is checked here.
    if( ::goo::aux::iApp::envvar_as_logical(PRESERVE_ROOT_SIGNAL_HANDLERS_ENVVAR) ) {
        std::cerr << "ROOT signal handlers preserved (kept in place)." << std::endl;
    } else {
        sV::mixins::RootApplication::reset_ROOT_signal_handlers();
    }
}

namespace sV {

namespace mixins {

const uint8_t RootApplication::enableCommonFile     = 0x1,
              RootApplication::enablePlugins        = 0x2,
              RootApplication::enableDynamicPath    = 0x4,
              RootApplication::enableTApplication   = 0x8;

RootApplication::RootApplication(
                        AbstractApplication::Config * c ) :
            AbstractApplication(c),
            _ROOTAppFeatures(0x0),
            _tApp(nullptr),
            _t_argc(0),
            _t_argv(nullptr) {
    _enable_ROOT_feature( enableTApplication );
}

RootApplication::~RootApplication() {
    if( _t_argc ) {
        ::goo::dict::Configuration::free_tokens( _t_argc, _t_argv );
    }
}

void
RootApplication::_enable_ROOT_feature( uint8_t ftCode ) {
    _ROOTAppFeatures |= ftCode;
}

void
RootApplication::append_ROOT_config_options(
                uint8_t rootFts,
                goo::dict::Dictionary & commonCfg ) {
    commonCfg.subsection("sV-paths").insertion_proxy()
        .p<goo::filesystem::Path>( "root-plugins",
            "Path to sV's ROOT plugins dir." )
        ;
    auto ip = commonCfg.insertion_proxy().bgn_sect(
            "ROOT",
            "CERN ROOT analysis framework integration runtime options." );
    if( enableCommonFile && rootFts ) {
        ip.p<goo::filesystem::Path>("output-file",
                "output ROOT-file path for current session. Set to \"none\" "
                "to omit.",
            "none" );
    }
    if( enablePlugins && rootFts ) {
        ip.p<goo::filesystem::Path>("plugin-handlers-file",
                "Path to file containing ROOT plugin handlers (see reference "
                "to TPluginManager for syntax).",
            "sV-plugins.rootrc" );
    }
    if( enableDynamicPath && rootFts ) {
        ip.list<goo::filesystem::Path>("dynamic-path",
                "Additional dynamic path to extend ROOT TSystem "
                "(search path for shared libraries).");
    }
    if( enableTApplication && rootFts ) {
        ip.p<std::string>("TApplication-args",
                "A string to be parsed as TApplication command line. "
                "See TApplication::GetOptions() reference for your "
                "ROOT installation for reference. Note, that any spaces "
                "or special characters need to be escaped.",
            "" );
    }
    ip.end_sect( "ROOT" );
}

void
RootApplication::reset_ROOT_signal_handlers() {
    // Disable those damn default ROOT's signal handler.
    for( int sig = 0; sig < kMAXSIGNALS; sig++ ) {
        gSystem->ResetSignal((ESignals)sig);
    }
}

void
RootApplication::initialize_ROOT_system(
                    uint8_t rootFts,
                    goo::dict::Dictionary & commonCfg,
                    int & argc, char **& argv ) {
    using goo::filesystem::Path;

    bool doImmediateExit = false;
    ConfigPathInterpolator * pathIpPtr = nullptr;
    if( goo::aux::iApp::exists() ) {
        pathIpPtr = goo::app<AbstractApplication>().path_interpolator();
        doImmediateExit = goo::app<AbstractApplication>().do_immediate_exit();
    }

    if( rootFts & enableDynamicPath ) {
        const goo::dict::iSingularParameter & dynPath = commonCfg["ROOT.dynamic-path"];
        if( !dynPath.as_list_of<Path>().empty()
         && !doImmediateExit ) {
            for( auto additionalPath : dynPath.as_list_of<Path>() ) {
                if( !! pathIpPtr ) {
                    additionalPath.interpolator( pathIpPtr );
                }
                if( !additionalPath.empty() ) {
                    sV_log2( "Adding ROOT's dynamic path: %s.\n", 
                            additionalPath.interpolated().c_str() );
                    gSystem->AddDynamicPath( additionalPath.interpolated().c_str() );
                }
            }
        }
    }

    if( rootFts & enablePlugins ) {
        Path path = commonCfg["ROOT.plugin-handlers-file"].as<Path>();
        if( !! pathIpPtr ) {
            path.interpolator( pathIpPtr );
        }
        if( !path.interpolated().empty() ) {
            sV_log2( "Adding ROOT's plugins handlers file \"%s\".\n",
                path.interpolated().c_str() );
            // Set plugin handlers:
            TEnv * env = new TEnv( path.interpolated().c_str() );
            env->SaveLevel(kEnvLocal);
            # if 0
            // Due to a known issue
            // https://sft.its.cern.ch/jira/si/jira.issueviews:issue-html/ROOT-8109/ROOT-8109.html
            // we're unable to use gPluginMgr variable here.
            gPluginMgr->LoadHandlersFromEnv( env );
            # else
            // Load from env:
            gROOT->GetPluginManager()->LoadHandlersFromEnv( env );
            # endif
        }
    }

    if( rootFts & enableTApplication ) {
        new_native_ROOT_application_instance(
                commonCfg["ROOT.TApplication-args"].as<std::string>(),
                argc, argv );
    }

    if( rootFts & enableCommonFile ) {
        Path path = commonCfg["ROOT.output-file"].as<Path>();
        if( pathIpPtr ) {
            path.interpolator( pathIpPtr );
        }
        // Try to open ROOT file, if provided:
        if( !path.interpolated().empty()
             && "none" != path.interpolated()
             && !doImmediateExit ) {
            sV_log2( "Opening ROOT output file \"%s\".\n",
                path.interpolated().c_str() );
            // Will be closed as gFile
            auto f = new TFile( path.interpolated().c_str(), "RECREATE" );
            if( f->IsZombie() ) {
                sV_logw( "Error opening file \"%s\".\n",
                             path.interpolated().c_str() );
            } else {
                sV_log1( "Output file opened: \"%s\".\n",
                             path.interpolated().c_str() );
            }
        }
    }
}

TApplication *
RootApplication::new_native_ROOT_application_instance(
            const std::string & rooAppArgsStr, int & argc, char **& argv ) {
    TApplication * ret = nullptr;
    if( rooAppArgsStr.empty() ) {
        ret = new TApplication( "sV-app", nullptr, nullptr, nullptr, 0 );
    } else {
        argc = ::goo::dict::Configuration::tokenize_string(
                    rooAppArgsStr, argv );
        ret = new TApplication( argv[0], &argc, argv, nullptr, 0 );
    }
    ret->SetReturnFromRun(kTRUE);
    ret->Init();
    return ret;
}


void
RootApplication::_append_ROOT_config_options( goo::dict::Dictionary & commonCfg ) {
    append_ROOT_config_options( ROOT_features(), commonCfg );
}

void
RootApplication::_initialize_ROOT_system( goo::dict::Dictionary & commonCfg ) {
    initialize_ROOT_system( ROOT_features(), commonCfg, _t_argc, _t_argv );
}

}  // namespace mixins

}  // namespace sV

# if 0
static void __static_add_handler_plugins() __attribute__ ((constructor(156)));
static void __static_add_handler_plugins() {
    // add handler crutch:
    gPluginMgr->AddHandler( "TGMainFrame", "*", "P348CommandWidgetPlugin",
         "P348CommandWidgetPlugin", "P348CommandWidgetPlugin(const TGWindow*,UInt_t,UInt_t)" );
}
# endif
