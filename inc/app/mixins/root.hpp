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

# ifndef H_STROMA_V_CERN_ROOT_FRAMEWORK_BASED_APPLICATION_MIXIN_H
# define H_STROMA_V_CERN_ROOT_FRAMEWORK_BASED_APPLICATION_MIXIN_H

# include "app/abstract.hpp"

/// Name of environment variable (logic value) name controlling whether to
/// keep default ROOT signal handlers.
# define PRESERVE_ROOT_SIGNAL_HANDLERS_ENVVAR "PRESERVE_ROOT_SIGNAL_HANDLERS"

class TApplication;

namespace sV {

namespace mixins {

/**@class RootApplication
 * @brief A wrapper for CERN's ROOT TApplication.
 *
 * The purpose of static method handling various ROOT features based on
 * features flag is to provide some convinient routines to user code that does
 * not desire to rely on StromaV application classes.
 *
 * @ingroup app
 */
class RootApplication : public virtual AbstractApplication {
public:
    static const uint8_t enableCommonFile,
                         enablePlugins,
                         enableDynamicPath,
                         enableTApplication;
private:
    /// This should only be set by descendant classes, from ctrs.
    uint8_t _ROOTAppFeatures;
    TApplication * _tApp;
    std::string _appClassName;
    /// argc for TApplication (set by AbstractApplication).
    int _t_argc;
    /// argv[] for TApplication (set by AbstractApplication).
    char ** _t_argv;
protected:
    /// May be invoked from ctrs of descendant classes.
    void _enable_ROOT_feature( uint8_t ftCode );
    /// Invoked by abstract application. Appends config object with
    /// ROOT-specific options.
    void _append_ROOT_config_options( goo::dict::Dictionary & commonCfg );
    /// Invoked by abstract application to init configured features.
    void _initialize_ROOT_system( goo::dict::Dictionary & commonCfg );
public:
    RootApplication( AbstractApplication::Config * c,
                     const std::string & appClassName );
    virtual ~RootApplication();

    const TApplication & get_TApplication() const { assert(_tApp); return *_tApp; }
    TApplication & get_TApplication() { assert(_tApp); return *_tApp; }

    uint8_t ROOT_features() const { return _ROOTAppFeatures; }


    //
    // Miscellaneous ROOT-related routines which does not require ROOT mixin be
    // inherited by user's App, but claiming some common things about ROOT.
    /// Turns off ROOT signal handlers that is set by default when project
    /// linked against ROOT libraries.
    static void reset_ROOT_signal_handlers();
    /// Appends config object with ROOT-specific options according to given
    /// features list.
    static void append_ROOT_config_options(
            uint8_t flags,
            goo::dict::Dictionary & commonCfg );
    /// Initializes ROOT according to given features flags and dictionary
    /// config.
    static void initialize_ROOT_system(
            uint8_t flags,
            goo::dict::Dictionary & commonCfg,
            int argc &, const char **& argv );
    /// Will create new TApplication instance with arguments extracted from
    /// given string expression (as if it was processed to argc/argv by shell).
    /// The argc/argv arguments gas to be freed further with
    /// ::goo::dict::Configuration::free_tokens() procedure. Will be invoked
    /// by initialize_ROOT_system() if corresponding feature flag provided.
    static TApplication * new_native_ROOT_application_instance(
            const std::string &,
            int rootArgc &,
            const char **& rootArgv );
    friend class AbstractApplication;
};

}  // namespace mixins

}  // namespace sV

# endif  // H_STROMA_V_CERN_ROOT_FRAMEWORK_BASED_APPLICATION_MIXIN_H


