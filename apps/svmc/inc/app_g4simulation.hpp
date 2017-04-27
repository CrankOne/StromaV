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

# include "afNA64_config.h"

# include "app/mixins/geant4.hpp"
# include "app/mixins/root.hpp"

# include <fstream>
# ifdef StromaV_RPC_PROTOCOLS
# include "app/mixins/protobuf.hpp"
# endif  // StromaV_RPC_PROTOCOL

namespace svmc {

class Application : public sV::mixins::Geant4Application,
                    public sV::mixins::RootApplication,
                    public sV::mixins::PBEventApp {
public:
    typedef sV::mixins::Geant4Application Parent;
    typedef Parent::Config Config;
    typedef sV::mixins::Geant4Application G4AppMixin;
protected:
    virtual Config * _V_construct_config_object( int argc, char * const argv[] ) const override;
    virtual std::vector<sV::po::options_description> _V_get_options() const override;
    virtual void _V_configure_concrete_app() override;
    virtual int _V_run() override;
    virtual void _build_up_run() override;

    virtual void _initialize_tracking_action();
    virtual void _initialize_event_action();
    std::fstream _fileRef;
public:
    Application( Config * cfg );
    virtual ~Application();
};

}  // namespace svmc

# endif  // H_APP_G4SIMULATION_H

