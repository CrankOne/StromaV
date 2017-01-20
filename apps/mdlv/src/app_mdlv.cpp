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

// # pragma GCC diagnostic push
// # pragma GCC diagnostic ignored "-Wdeprecated-register"  // "register" is depr-d
# include <G4RunManager.hh>
// # pragma GCC diagnostic pop

# include "app_mdlv.hpp"

//# include "g4_DC.hh"
# include "dummy_PG.hh"
# include "dummy_PhL.hh"
# include "ext.gdml/auxInfoProcessor.hpp"

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

std::vector<sV::po::options_description>
Application::_V_get_options() const {
    auto res = Parent::_V_get_options();
    res.push_back( _geant4_options() );
    res.push_back( _geant4_gdml_options() );
    return res;
}

void
Application::_V_configure_concrete_app() {
    _treat_geant4_options(      goo::app<Application>().co() );
    if( do_immediate_exit() ) return;
    _treat_geant4_gdml_options( goo::app<Application>().co() );
}

Application::Config *
Application::_V_construct_config_object( int argc, char * const argv[] ) const {
    Geant4Application::_set_cmd_args( argc, argv );
    return Parent::_V_construct_config_object(argc, argv);
}

sV::po::options_description
Application::_geant4_options() const  {
    sV::po::options_description g4cfg("Geant4 model options");
    g4cfg.add_options()
        ("g4.verbosity",
            sV::po::value<int>()->default_value(2 /* warnings */),
            "Geant4 core system verbosity (set before any .mac processing starts "
            "and further can be overriden by them).")
        ("g4.visMacroFile",
            sV::po::value<std::string>()->default_value("vis.mac"),
            "'vis' run-time script")
        ("g4.customExceptionHandler",
            sV::po::value<bool>()->default_value(true),
            "enable custom exception handler for G4 (behves just like ordinary one, but fancier)"
            )
        ("g4.useNIST",
            sV::po::value<bool>()->default_value(true),
            "use NIST materials (have prefix G4_ in GDML files).")
        ;
    return g4cfg;
}

//
// Viewer entry point.
//

void
Application::_initialize_physics() {
    G4RunManager::GetRunManager()->SetUserInitialization(new DummyPhL);
}

int
Application::_V_run() {
    if( do_immediate_exit() ) { return EXIT_FAILURE; }
    return _run_session( false, cfg_option<std::string>("g4.visMacroFile") );
}

void
Application::dump_build_info( std::ostream & os ) const {
    // P348 Library
    Parent::dump_build_info( os );
    // TODO: application-specific info...
}

} // namespace ecal


