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

//# pragma GCC diagnostic push
//# pragma GCC diagnostic ignored "-Wdeprecated-register"  // "register" is depr-d
# include <G4RunManager.hh>
# include <G4LogicalVolumeStore.hh>
# include <G4TransportationManager.hh>
# include <G4GDMLParser.hh>
//# pragma GCC diagnostic pop

# include <TFile.h>

# include <goo_exception.hpp>

# include "app_g4simulation.hpp"
# include "actions/TrackingAction.hpp"
# include "actions/EventAction.hpp"
# include "g4extras/PhysList.hpp"
# include "g4extras/PGA.hpp"
# include "ext.gdml/SensDetDict.hpp"

# ifdef G4_MDL_VIS
# include <G4VisManager.hh>
#   include <G4VisExecutive.hh>
# endif

# ifdef G4_MDL_GUI
#   include <G4UImanager.hh>
#   include <G4UIExecutive.hh>
# endif

namespace svmc {

Application::Application( Config * cfg) :
                                AbstractApplication(cfg),
                                Geant4Application(cfg),
                                RootApplication(cfg, "Application", true, true)
                                # ifdef RPC_PROTOCOLS
                                , PBEventApp(cfg)
                                # endif  // RPC_PROTOCOL
                                {
}

Application::~Application( ) {
}

Application::Config *
Application::_V_construct_config_object( int argc, char * const argv[] ) const {
    Geant4Application::_set_cmd_args( argc, argv );
    return Parent::_V_construct_config_object(argc, argv);
}

std::vector<sV::po::options_description>
Application::_V_get_options() const /*{{{*/ {
    auto res = Parent::_V_get_options();
    res.push_back( _geant4_options() );
    res.push_back( _geant4_gdml_options() );
    res.push_back( sV::iBucketDispatcher::_dispatcher_options());  // BucketDispatcher options
    {
        sV::po::options_description g4SDCfg;
        g4SDCfg.add_options()
            ("g4-SD-ECAL_cell.timeVSedepHisto",
                sV::po::value<bool>()->default_value(true),
                "XXX")
            ("g4-SD-ECAL_cell.timeVSedepMaxTime-ns",
                sV::po::value<double>()->default_value(5),
                "XXX")
            ("g4-SD-ECAL_cell.timeVSedep-timeNBins",
                sV::po::value<int>()->default_value(200),
                "XXX")
            ("g4-SD-ECAL_cell.timeVSedepMaxEDep-MeV",
                sV::po::value<double>()->default_value(2),
                "XXX")
            ("g4-SD-ECAL_cell.timeVSedep-edepNBins",
                sV::po::value<int>()->default_value(200),
                "XXX")
            ("g4-SD-ECAL_cell.scorerPool-NCells",
                sV::po::value<int32_t>()->default_value(10000),
                "Scorer pool size (number of values stored per event) for \"ECAL_cell\" detector.")
            ;
        res.push_back( g4SDCfg );
    }
    return res;
} /* }}} */

void
Application::_initialize_tracking_action() {
    G4RunManager::GetRunManager()->SetUserAction(new TrackingAction());
    sV_log2("User TrackingAction has been initialized\n");
}

void
Application::_initialize_event_action() {
    _fileRef.open(goo::app<sV::AbstractApplication>().cfg_option<std::string>
        ("b-dispatcher.outFile"), std::ios::out | std::ios::binary |
                                  std::ios::app );
    G4RunManager::GetRunManager()->SetUserAction(new EventAction(_fileRef));
    sV_log2("User EventAction has been initialized\n");
}
void
Application::_V_configure_concrete_app() /*{{{*/ {
    const sV::po::variables_map & vm = goo::app<sV::AbstractApplication>().co();
    if( do_immediate_exit() ) return;
    _treat_geant4_options(      goo::app<Application>().co() );
    if( do_immediate_exit() ) return;
    _treat_geant4_gdml_options( goo::app<Application>().co() );
    if( do_immediate_exit() ) return;

    if( vm.count("g4.list-physics") ) {
        // Physics list:
        auto phll = sV::available_physics_lists()
                # ifdef GEANT4_DYNAMIC_PHYSICS
                , mdls = sV::ModularPhysicsList::available_physics_modules()
                # endif  // GEANT4_DYNAMIC_PHYSICS
             ;
        if( phll.empty() ) {
            sV_loge( "No physics list available at current build!\n" );
        }
        int i = 1;
        sV_log1( ESC_CLRBOLD "Pre-formed physics list:" ESC_CLRCLEAR "\n" );
        for( auto it = phll.cbegin(); phll.cend() != it; ++it, ++i ) {
            sV_log1( "%30s%c", it->c_str(), ( i%3 ? '\t' : '\n') );
        }
        i = 1;
        sV_log1( "\n" ESC_CLRBOLD "Physics modules:" ESC_CLRCLEAR "\n" );
        # ifdef GEANT4_DYNAMIC_PHYSICS
        for( auto it = mdls.cbegin(); mdls.cend() != it; ++it, ++i ) {
            sV_log1( "%30s%c", it->c_str(), ( i%3 ? '\t' : '\n') );
        }
        # else  // GEANT4_DYNAMIC_PHYSICS
        sV_loge( "No physics modules are available at current build (since -DGEANT4_DYNAMIC_PHYSICS=OFF)!\n" );
        # endif // GEANT4_DYNAMIC_PHYSICS
        // PGAs
        auto pgal = sV::user_primary_generator_actions_list();
        if( pgal.empty() ) {
            sV_loge( "No primary generators available at current build!\n" );
        }
        i = 1;
        sV_log1( "\n" ESC_CLRBOLD "Primary generators:" ESC_CLRCLEAR "\n" );
        for( auto it = pgal.cbegin(); pgal.cend() != it; ++it, ++i ) {
            sV_log1( "%30s%c", it->c_str(), ( i%3 ? '\t' : '\n') );
        }
        sV_log1( "\n" );
        _immediateExit = true;
        return;
    }

    if ( vm.count("g4.sensitiveDetectorsList") ) {
        std::cout << "List of available sensitive detectors:" << std::endl;
        sV::SDDictionary::self().print_SD_List();
        std::cout << "* Basically, value of sensDet should consist of two parts separated with column ':' sign."
        << std::endl << "* E.g.:" << std::endl
        << "* ECAL_cell:/sVdet/ecal" << std::endl
        << "* Will refer to SensitiveDetector subclass named 'ECAL_cell' and create an instance"
        << std::endl << "* named '/sVdet/ecal'." << std::endl;                    // TODO
        _immediateExit = true;
        return;
    }
    initialize_ROOT_system( RootApplication::enableCommonFile );
} /*}}}*/

int
Application::_V_run() /*{{{*/ {
    if( do_immediate_exit() ) return EXIT_FAILURE;
    int rc = _run_session( co().count("g4.batch"), cfg_option<std::string>("g4.visMacroFile") );
    _clear_geant4_options( co() );
    _clear_geant4_gdml_options( co() );
    return rc;
}  /*}}}*/

} // namespace svmc

