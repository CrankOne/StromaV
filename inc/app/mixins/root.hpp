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
 * @ingroup app
 */
class RootApplication : public virtual AbstractApplication {
public:
    static const uint8_t enableCommonFile,
                         enablePlugins,
                         enableDynamicPath,
                         enableTApplication;
private:
    TApplication * _tApp;
    std::string _appClassName;
    /// argc for TApplication (set by AbstractApplication).
    int _t_argc;
    /// argv[] for TApplication (set by AbstractApplication).
    char ** _t_argv;

    /// Accepts argv as a plain text string variable.
    void _create_TApplication( const std::string & rootAppArguments );
public:
    RootApplication( AbstractApplication::Config * c,
                     const std::string & appClassName );
    virtual ~RootApplication();

    const TApplication & get_TApplication() const { assert(_tApp); return *_tApp; }
    TApplication & get_TApplication() { assert(_tApp); return *_tApp; }

    //
    // Miscellaneous ROOT-related routines which does not require ROOT mixin be
    // inherited by user's App, but claiming some common things about ROOT.

    /// Forbits ROOT signal handlers that is set by default when project linked against ROOT libraries.
    static void reset_ROOT_signal_handlers();

    /// Appends config object with ROOT-specific options.
    static void append_ROOT_config_options( goo::dict::Dictionary &,
                                            uint8_t featuresEnabled );

    /// Invoked by abstract application to init configured features.
    static void initialize_ROOT_system( uint8_t featuresEnabled );

    friend class AbstractApplication;
};

}  // namespace mixins

}  // namespace sV

# endif  // H_STROMA_V_CERN_ROOT_FRAMEWORK_BASED_APPLICATION_MIXIN_H


