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

# include "app/mixins/geant4.hpp"

# ifdef GEANT4_MC_MODEL

# include "g4extras/eHandler.hpp"

// # pragma GCC diagnostic push
// # pragma GCC diagnostic ignored "-Wdeprecated-register"  // "register" is depr-d
# include <G4UImanager.hh>
# include <G4NistManager.hh>
# include <G4GDMLParser.hh>
# include <G4UIsession.hh>
# include <G4RunManager.hh>
# include <G4VisExecutive.hh>
# include <G4UIExecutive.hh>
// # pragma GCC diagnostic pop


# include "g4extras/PGA.hpp"
# include "g4extras/PhysList.hpp"

# include "g4extras/eHandler.hpp"

# include <ext.gdml/SensDetDict.hpp>
# include <ext.gdml/auxInfoSet.hpp>
# include <ext.gdml/DetectorConstruction.hpp>
# include <ext.gdml/extras.hpp>
# include <ext.gdml/gdml_aux_visStyles.hpp>

# include <TFile.h>

/**@defgroup mc MC Siulation
 * @brief Monte-Carlo simulation helpers.
 *
 * Group containing various Monte-Carlo (MC) procedures. Most of them
 * are integrated with Geant4 frameowork and are included also in
 * Geant4-integration group.
 * */

/**@defgroup g4 Geant4-integration
 * @brief Helper classes and routines adopted for Geant4 applications.
 *
 * Provides various stuff for interacting with Geant4 frameowork.
 * */

namespace sV {
namespace mixins {

Geant4Application * Geant4Application::_self_Geant4ApplicationPtr = nullptr;

class Geant4Application::AppSession : public G4UIsession {
protected:
    std::stringstream _g4ls, _g4es;
public:
    AppSession(){}
    //virtual G4UIsession * SessionStart() override;
    //virtual void PauseSessionStart(const G4String& Prompt) override;
    virtual G4int ReceiveG4cout(const G4String & msg) override {
            std::cout << msg;  // XXX
            _g4ls << msg;
            return 0; }
    virtual G4int ReceiveG4cerr(const G4String & msg) override {
            std::cerr << msg;  // XXX
            _g4es << msg;
            return 0; }
};

Geant4Application::Geant4Application( AbstractApplication::Config * c ) :
                      AbstractApplication(c),
                      _session(nullptr),
                      _NISTMatMan(nullptr),
                      _parser(nullptr),
                      _visManagerPtr(nullptr) {
    Geant4Application::_self_Geant4ApplicationPtr = this;
    _session = new AppSession();
    G4UImanager::GetUIpointer()->SetCoutDestination(_session);
}

Geant4Application::~Geant4Application() {
    delete _session;
}

# if 0
std::ostream &
Geant4Application::g4_log_stream() {
    return G4cout;
}

std::ostream &
Geant4Application::g4_err_stream() {
    return G4cerr;
}
# endif

po::options_description
Geant4Application::_geant4_options() const {
    po::options_description g4cfg("Geant4 model options");
    g4cfg.add_options()
        ("g4.useNIST",
            po::value<bool>()->default_value(true),
            "use NIST materials (have prefix G4_ in GDML files).")
        ("g4.verbosity",
            po::value<int>()->default_value(2 /* warnings */),
            "Geant4 core system verbosity (set before any .mac processing starts "
            "and further can be overriden by them).")
        ("g4.visMacroFile",
            po::value<std::string>()->default_value("vis.mac"),
            "'vis' run-time script")
        ("g4.customExceptionHandler",
            po::value<bool>()->default_value(true),
            "enable custom exception handler for G4 (behves just like ordinary one, but fancier)"
            )
        ("g4.batch",
            "If given, runs model without a GUI.")
        ("g4.primaryGenerator",
            po::value<std::string>()->default_value("SimpleGun"),
            "Primary particles generator type specification.")
        ("g4.physicsList",
            po::value<std::string>()->default_value("FTFP_BERT"),
            "Set physics list defining the entire MC simulation.")
        ("g4.list-physics",
            "List physics lists which are available at current build.")
        ("extraPhysics.physlist.module",
            po::value<std::vector<std::string> >(),
            "Physics module to be included in modular physics list.")
        ("extraPhysics.verbosity",
            po::value<std::string>()->default_value("application"),
            "Verbosity for physics list. Can be set to 0..3 or to \"application\""
            " to correspond global application verbosity.")
        ("extraPhysics.productName",
            po::value<std::string>()->default_value("FTFP_BERT_EMV"),
            "Name of G4PhysListFactory product. See Geant4 manual for guide how "
            "this name can be composed.")
        ("extraPhysics.physicsSR.considerMaterials",
            po::value<bool>()->default_value(false),
            "Take into consideration material parameters while modelling synchrotron radiation physics.")
        ("g4.sensitiveDetectorsList",
            "List sensitive detectors which are available at current build.")
        ("g4-simpleGun.particleType",
            po::value<std::string>()->default_value("e-"),
            "Default particle type (can be overriden in messenger).")
        ("g4-simpleGun.position-cm",
            po::value<std::string>()->default_value("{0.,0.,-3.}"),
            "Gun position vector, cm (can be further overriden in messenger).")
        ("g4-simpleGun.direction",
            po::value<std::string>()->default_value("{0.,0.,1.}"),
            "Gun orientation vector (can be overriden in messenger).")
        ("g4-simpleGun.energy-MeV",
            po::value<double>()->default_value(2e+5),
            "Gun energy, MeV (can be further overriden in messenger).")
        ;
    return g4cfg;
}

po::options_description
Geant4Application::_geant4_gdml_options() const {
    po::options_description gdmlCfg("GEANT4/GDML-relevant options");
    gdmlCfg.add_options()
        ("geometry",
            po::value<std::string>(),
            "GDML file to treat." )
        ("gdml.overlapCheck",
            po::value<bool>()->default_value(true),
            "Do or not overlap checking at parsing stage.")
        ("gdml.setup",
            po::value<std::string>()->default_value("Default"),
            "Default setup to be used from GDML description.")
        ("gdml.defaultStyle",
            po::value<std::string>()->default_value("dft:#777777aa,!wireframe"),
            "Default style for drawable items.")
        ("gdml.enableXMLSchemaValidation",
            po::value<bool>()->default_value(true),
            "Enable GDML's XML schema validation (useful for initial speed-up and offline work).")
        ("gdml.aux.tag",
            po::value<std::vector< std::string> >(),
            "GDML aux tags to be enabled for processing")

        ;
    return gdmlCfg;
}

void
Geant4Application::_treat_geant4_options( const po::variables_map & vm ) {
    // Set up a NIST material manager.
    // Note: materials (not elements!) of NIST library can be referenced from GDML
    // by G4_ prefix.
    if( cfg_option<bool>("g4.useNIST") ) {
        //sV_log3("Enabling NIST.\n");
        (_NISTMatMan = G4NistManager::Instance())->SetVerbose(1);
    }

    if( cfg_option<bool>("g4.customExceptionHandler") ) {
        sV::aux::ExceptionHandler::enable();
    }

    if( !vm.count("geometry") ) {
        emraise( malformedArguments, "Geometry file must be specified." );
    }
}

void
Geant4Application::_treat_geant4_gdml_options( const po::variables_map & vm ) {
    _parser = new G4GDMLParser();
    if( vm["gdml.overlapCheck"].as<bool>() ) {
        _parser->SetOverlapCheck(true);
    }
    //sV_log2("Parsing a GDML geometry from \"%s\".\n", vm["geometry"].as<std::string>().c_str());
    _parser->Read(vm["geometry"].as<std::string>(),
        vm["gdml.enableXMLSchemaValidation"].as<bool>() );
    //sV_log2("Parsing GDML geometry succeed.\n");
}

void
Geant4Application::_clear_geant4_options( const po::variables_map & vm ) {
    if( vm["g4.customExceptionHandler"].as<bool>() ) {
        sV::aux::ExceptionHandler::disable();
    }
}

void
Geant4Application::_clear_geant4_gdml_options( const po::variables_map & ) {
    if( _parser ) {
        delete _parser;
    }
}

void
Geant4Application::_set_cmd_args( int argc, char * const argv[] ) const {
    const_cast<Geant4Application*>(this)->_argc = argc;
    const_cast<Geant4Application*>(this)->_argv = argv;
}

void
Geant4Application::g4_abort() {
    # ifdef GEANT4_MC_MODEL
    if( G4UImanager::GetUIpointer() ) {
        G4UImanager::GetUIpointer()->ApplyCommand("/run/abort");
    }
    # endif
}

void
Geant4Application::_initialize_geometry() {
    sV_log2("Setting up GEANT4 run manager on default volume \"%s\".\n",
                _setupName.c_str());
    G4RunManager::GetRunManager()->SetUserInitialization(
                new extGDML::DetectorConstruction
                        (gdml_parser_ptr()->GetWorldVolume(_setupName)) );
}

void
Geant4Application::_initialize_physics() {
    if( co().count("g4.physicsList") &&
            cfg_option<std::string>("g4.physicsList") != "none" ) {
        // Create a PhysicsList instance if it is not configured to `none':
        G4RunManager::GetRunManager()->SetUserInitialization(
                sV::obtain_physics_list_instance( co()["g4.physicsList"].as<std::string>() )
            );
    } else {
        sV_logw( "No physics list assigned to MC simulation as there is no physicsList option provided.\n" );
    }
}

void
Geant4Application::_initialize_primary_generator_action() {
    G4UImessenger * srcMessenger = nullptr;  // TODO: set srcMessenger
    if( "none" != cfg_option<std::string>("g4.primaryGenerator") ) {
        G4RunManager::GetRunManager()->SetUserAction( sV::user_primary_generator_action(
                    cfg_option<std::string>("g4.primaryGenerator"),
                    srcMessenger
                ) );
    }
}

void
Geant4Application::_build_up_run() {
    if( cfg_option<bool>("g4.customExceptionHandler") ) {
        sV::aux::ExceptionHandler::enable();  // as G4RunManagerKernel overrides our handler in ctr,
                                                // we must re-set it again here.
    }
    _setupName = cfg_option<std::string>("gdml.setup");
    // Do the G4 initialization stuff.
    _initialize_geometry();
    // assign physlist:
    _initialize_physics();
    // PGA
    _initialize_primary_generator_action();
    G4RunManager::GetRunManager()->Initialize();
}

# ifdef G4_MDL_GUI
int
Geant4Application::_gui_run( const std::string & macroFilePath ) {
    int rc = EXIT_SUCCESS;
    # ifdef G4_MDL_VIS
    _visManagerPtr = new G4VisExecutive();
    _visManagerPtr->Initialize( /* former syntax required argc, argv */ );
    # endif

    // Define (G)UI
    # ifdef G4_MDL_VIS
    G4UIExecutive * uiExec = new G4UIExecutive(_argc, const_cast<char **>(_argv));
    # endif
    if( !macroFilePath.empty() ) {
        rc = _batch_run( macroFilePath );
    }
    # ifdef G4_MDL_VIS
    uiExec->SessionStart();
    delete uiExec;
    # endif

    return rc;
}
# endif

int
Geant4Application::_batch_run( const std::string & macroFilePath ) {
    char bf[128];
    snprintf( bf, sizeof(bf),
              "/vis/verbose %d", cfg_option<int>("g4.verbosity"));
    G4UImanager::GetUIpointer()->ApplyCommand( bf );
    snprintf( bf, sizeof(bf),
              "/control/execute %s", macroFilePath.c_str() );
    sV_log2("Vis manager now executing \"%s\"...\n", macroFilePath.c_str() );
    G4UImanager::GetUIpointer()->ApplyCommand( bf );
    sV_log2("... end of \"%s\" execution.\n", macroFilePath.c_str() );
    return EXIT_SUCCESS;
}

int
Geant4Application::_run_session( bool isBatch, const std::string & macroFilePath ) {
    int rc = EXIT_FAILURE;

    // Allocate Geant4 run manager.
    G4RunManager * runManager = new G4RunManager();
    // Initializes run manager, geometry, sens. dets, etc.
    _build_up_run();
    sV_log2("Processing aux info.\n");
    extGDML::AuxInfoSet * auxInfoSet =
        new extGDML::AuxInfoSet(cfg_option<std::vector<std::string> >
                                    ("gdml.aux.tag"));
    auxInfoSet->apply( *gdml_parser_ptr() );
    extGDML::extras::apply_styles_selector( _setupName );

    if( isBatch ) {
        rc = _batch_run( macroFilePath );
    } else {
        # if G4_MDL_GUI
        rc = _gui_run( macroFilePath );
        # else
        sV_loge( "Only batch mode available in this build.\n" );
        rc = EXIT_FAILURE;
        # endif
    }

    assert( G4RunManager::GetRunManager() == runManager );  // XXX?

    // Free run manager.
    delete runManager;

    # ifdef G4_MDL_VIS
    if( _visManagerPtr ) {
        delete _visManagerPtr;
        _visManagerPtr = nullptr;
    }
    # endif

    /// Close root file, if needed.
    if( gFile ) {
        gFile->Write();
        gFile->Close();
    }
    return rc;
}

}  // mixins
}  // namespace sV

# endif  // GEANT4_MC_MODEL

