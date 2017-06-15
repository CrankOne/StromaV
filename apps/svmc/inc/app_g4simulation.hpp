/*
 * Copyright (c) 2016 Renat R. Dusaev <crank@qcrypt.org>
 * Author: Renat R. Dusaev <crank@qcrypt.org>
 * Author: Bogdan Vasilishin <togetherwithra@gmail.com>
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

# ifndef H_APP_G4SIMULATION_H
# define H_APP_G4SIMULATION_H

# include "sV_config.h"

# include "app/mixins/geant4.hpp"
# include "app/mixins/root.hpp"

# include <fstream>
# ifdef RPC_PROTOCOLS
# include "app/mixins/protobuf.hpp"
# endif  // RPC_PROTOCOLS

namespace svmc {

/**@class Application
 * @brief Generic Geant4 application utilizing StromaV framework.
 *
 * Inherits StromaV Geant4 application in conjunction with PBEventApp and
 * RootApplication mixins to provide a flexible solution for extensible MC
 * simulation procedure.
 * */
class Application : public sV::mixins::Geant4Application,
                    public sV::mixins::RootApplication
                    # ifdef RPC_PROTOCOLS
                    , public sV::mixins::PBEventApp
                    # endif
                    {
public:
    typedef sV::mixins::Geant4Application Parent;
    typedef Parent::Config Config;
    typedef sV::mixins::Geant4Application G4AppMixin;
protected:
    /// Appends common config with additional parameters.
    virtual void _V_concrete_app_append_common_cfg() override;
    /// Applies configuration parameters to application instance.
    virtual void _V_concrete_app_configure() override;
    /// Forwards execution to Geant4 event loop.
    virtual int _V_run() override;
    /// Appends parent's implementation with _initialize_tracking_action()
    /// and _initialize_event_action().
    virtual void _build_up_run() override;
    /// Adds support for virtually-constructed tracking action instances.
    virtual void _initialize_tracking_action();
    /// Adds support for virtually-constructed event action instances.
    virtual void _initialize_event_action();
    // XXX?
    //std::fstream _fileRef;
public:
    Application( Config * cfg );
    virtual ~Application();
};

}  // namespace svmc

# endif  // H_APP_G4SIMULATION_H

