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
        std::cerr << "Keeping ROOT signal handlers in place." << std::endl;
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
                        AbstractApplication::Config * c,
                        const std::string & appClassName ) :
            AbstractApplication(c),
            _tApp(nullptr),
            _appClassName(appClassName),
            _t_argc(0),
            _t_argv(nullptr) {
    AbstractApplication::_enable_ROOT_feature( enableTApplication );
}

RootApplication::~RootApplication() {
    if( _t_argc ) {
        // TODO: to be replaced by goo's ::goo::dict::Configuration::tokenize_string()
        // or ::goo::dict::Configuration::free_tokens()
        // when goo/appParameters branch will be merged to goo/master:
        ::sV::aux::goo_XXX_free_tokens( _t_argc, _t_argv );
    }
}

void
RootApplication::_create_TApplication( const std::string & rootAppArguments ) {
    if( rootAppArguments.empty() ) {
        _tApp = new TApplication( _appClassName.c_str(), nullptr, nullptr, nullptr, 0 );
    } else {
        _t_argc = ::sV::aux::goo_XXX_tokenize_argstring(
                    rootAppArguments,
                    _t_argv );
        _tApp = new TApplication( _appClassName.c_str(), &_t_argc, _t_argv, nullptr, 0 );
    }
    _tApp->SetReturnFromRun(kTRUE);
    _tApp->Init();
}

void
RootApplication::append_ROOT_config_options(
                goo::dict::Dictionary & rootAppCfg,
                uint8_t featuresEnabled ) {
    auto ip = rootAppCfg.insertion_proxy().bgn_sect(
            "ROOT",
            "CERN ROOT analysis framework integration runtime options." );
    if( enableCommonFile && featuresEnabled ) {
        ip.p<goo::filesystem::Path>("output-file",
                "output ROOT-file path for current session. Set to \"none\" "
                "to omit.",
            "none" );
    }
    if( enablePlugins && featuresEnabled ) {
        ip.p<goo::filesystem::Path>("plugin-handlers-file",
                "Path to file containing ROOT plugin handlers (see reference "
                "to TPluginManager for syntax).",
            "sV-plugins.rootrc" );
    }
    if( enableDynamicPath && featuresEnabled ) {
        // TODO: Make it list!
        ip.list<goo::filesystem::Path>("dynamic-path",
                "Additional dynamic path to extend ROOT TSystem "
                "(search path for shared libraries).");
    }
    if( enableTApplication && featuresEnabled ) {
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
RootApplication::initialize_ROOT_system( uint8_t featuresEnabled ) {
    using goo::filesystem::Path;
    AbstractApplication & thisApp = goo::app<AbstractApplication>();
    ConfigPathInterpolator * pathIpPtr = thisApp.path_interpolator();
    if( featuresEnabled & enableDynamicPath ) {
        if( !thisApp.cfg_option<Path>("ROOT.dynamic-path").empty()
         && !thisApp.do_immediate_exit() ) {
            for( auto additionalPath : thisApp.cfg_options_list<Path>("ROOT.dynamic-path") ) {
                additionalPath.interpolator( pathIpPtr );
                if( !additionalPath.empty() ) {
                    gSystem->AddDynamicPath( additionalPath.interpolated().c_str() );
                    sV_log2( "Added ROOT's dynamic path: %s.\n", 
                            additionalPath.interpolated().c_str() );
                }
            }
        }
    }
    if( featuresEnabled & enablePlugins ) {
        Path path = thisApp.cfg_option<Path>("ROOT.plugin-handlers-file");
        path.interpolator( pathIpPtr );
        if( !path.interpolated().empty() ) {
            sV_log2( "Adding ROOT plugins handlers file \"%s\".\n",
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
    if( featuresEnabled & enableTApplication ) {
        goo::app<RootApplication>()._create_TApplication(
            thisApp.cfg_option<std::string>("ROOT.TApplication-args") );
    }
    if( featuresEnabled & enableCommonFile ) {
        Path path = thisApp.cfg_option<Path>("ROOT.output-file");
        path.interpolator( pathIpPtr );
        // Try to open ROOT file, if provided:
        if( !path.interpolated().empty()
         && "none" != path.interpolated()
         && !thisApp.do_immediate_exit() ) {
            // will be closed as gFile
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
