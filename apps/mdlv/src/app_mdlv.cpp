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

# include <goo_exception.hpp>

# include <G4RunManager.hh>

# include "app_mdlv.hpp"

# include "dummy_PG.hh"
# include "dummy_PhL.hh"

# ifndef G4_MDL_GUI
# error "Need G4_MDL_GUI to be enabled as mdlv is pure GUI-oriented util."
# endif  // G4_MDL_GUI

# ifndef G4_MDL_VIS
# error "Need G4_MDL_VIS to be enabled as mdlv is pure GUI-oriented util."
# endif  // G4_MDL_VIS

# include <G4UImanager.hh>
# include <G4VisExecutive.hh>
# include <G4UIExecutive.hh>

# include <G4LogicalVolumeStore.hh>
# include <G4TransportationManager.hh>
# include <G4GDMLParser.hh>
//# include <G4Material.hh>
# include <G4NistManager.hh>

namespace mdlv {

Application::Application( Config * cfg ) :
        sV::AbstractApplication(cfg), Parent( cfg ) {}

void
Application::_V_configure_concrete_app() {
    _treat_geant4_options( /*TODO: get Geant4 opts*/ );
    if( do_immediate_exit() ) return;
    _treat_geant4_gdml_options( /*TODO: get Geant4.GDML opts*/ );
}

void
Application::_initialize_physics() {
    G4RunManager::GetRunManager()->SetUserInitialization(new DummyPhL);
}

int
Application::_V_run() {
    if( do_immediate_exit() ) { return EXIT_FAILURE; }
    return _run_session( false, cfg_option<std::string>("g4.visMacroFile") );
}

} // namespace ecal


