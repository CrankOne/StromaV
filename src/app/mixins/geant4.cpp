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
    _session = new AppSession();
    G4UImanager::GetUIpointer()->SetCoutDestination(_session);

    c->insertion_proxy()
        .flag( "list-physics",
            "List physics lists which are available at current build." )
        .flag( "list-sensitive-detectors",
            "Prints list of registered sensitive detectors that may be "
            "associated within GDML detector description." )
        .flag( "list-aux-tags",
            "Prints list of registered auxilliary tags that may be "
            "put within GDML geometry description to provide some extra "
            "behaviour (e.g. sensitive detector association, appearance "
            "styling, etc)." )
        .flag( "list-PGAs",
            "Prints list of registered primary generator classes that produces "
            "initial particles during Geant4 MC event simulation." )
        .p<goo::filesystem::Path>( "geometry",
            "GDML source to be parsed. May refer to file or network "
            "address." ) //.required_argument() ?
        .p<goo::filesystem::Path>( "visMacroFile",
                "Geant4 .mac script that usually steers the actual "
                "simulation in non-interactive mode, or prior to it."
            ) //.required_argument() ?
        .p<bool>( "customExceptionHandler",
                "enable custom exception handler for G4 (behaves similar to "
                "ordinary one, but a bit fancier)",
            true )
    ;

    # if 0
    AbstractApplication::ConstructableConfMapping::self()
        .set_basetype_description<...>( "sensitive-detectors",
            "Sensitive detectors that may be associated with volumes to "
            "provide reactive behaviour." )
        ;
    AbstractApplication::ConstructableConfMapping::self()
        .set_basetype_description<...>( "aux-tags",
            "Auxilliary tags introducing additional information into GDML "
            "file." )
        ;
    # endif
}

Geant4Application::~Geant4Application() {
    if( _parser ) {
        delete _parser;
    }
    if( _session ) {
        delete _session;
    }
}

void
Geant4Application::_append_Geant4_config_options( goo::dict::Dictionary & commonCfg ) {
    commonCfg.insertion_proxy()
    .bgn_sect("Geant4", "Major Geant4 framework integration "
        "options. Includes number of various pre-initialization parameters "
        "including .vis macros path, verbosity level, etc.")
        .p<bool>( "useNIST",
                "Whether to use NIST materials database (have prefix G4_* in "
                "Geant4 geometry).",
            true)
        .p<std::string>( "verbosity",
                "Geant4 core system verbosity level to start with. Will be set "
                "before any .mac script will be processed --- they can further "
                "override this level. May be set to \"application\" to "
                "indicate that initial Geant4 verbosity level matches sV's "
                "common.",
            "application" )
        .p<unsigned int>( "randomSeed",
                "Random generator seed to be used with CLHEP::HepRandom. "
                "Note, that null seed means no manual setting.",
            0 )
        .flag( "batch",
                "Disables a GUI session." )
        .p<std::string>( "primaryGenerator",
                "Primary particles generator name (from VCtr index).",
            "SimpleGun" )
        .p<std::string>( "physicsList",
                "Set physics list defining the entire MC simulation "
                "(from VCtr index).",
            "FTFP_BERT" )
        .bgn_sect( "extraPhysics", "Modular physics configuration section." )
            .list<std::string>( "module",
                    "Physics module to be included in modular physics list." )
            .p<std::string>("verbosity",
                    "Verbosity for physics list. Can be set to 0..3 or to "
                    "\"application\" to correspond global application "
                    "verbosity.",
                "application" )
            .p<std::string>("productName",
                    "Name of G4PhysListFactory product. See Geant4 manual for "
                    "guide how this name can be composed.",
                "FTFP_BERT_EMV" )
            //.p<bool>( "physicsSR-considerMaterials",
            //        "Take into consideration material parameters while "
            //        "modelling synchrotron radiation physics.",
            //    false )
        .end_sect("extraPhysics")
        .bgn_sect("gdml", "A GDML-related set of parameters. The GDML acronym "
            "comes for \"Geometry Description Mark-up Language\" which is "
            "merely a well-defined XML supplied within modern Geant4 "
            "distributions.")
                .p<bool>( "overlapCheck",
                        "Whether to perform overlap checking during GDML "
                        "parsing stage.",
                    true )
                .p<std::string>( "setup-name",
                        "Setup name to be used from GDML given description.",
                    "Default" )
                .p<std::string>( "defaultStyle",
                        "Default appearance style for drawable geometry when "
                        "no style is specified",
                    "dft:#777777aa,!wireframe" )
                .p<bool>( "enableXMLSchemaValidation",
                        "Enables GDML's XML schema validation.",
                    true )
                .list<std::string>( "enable-tag",
                        "GDML auxilliary tags to be enabled for processing")
        .end_sect("gdml")
        //.flag("sensitiveDetectorsList",
        //    "List sensitive detectors which are available at current build.")
        //.bgn_sect("simpleGun", "Simple primary particle gun configuration.")
        //    .p<std::string>("particleType",
        //            "Default particle type (can be overriden in messenger).",
        //        "e-" )
        //    .p<std::string>("position-cm",
        //            "Gun position vector, cm (can be further overriden in "
        //            "messenger).",
        //        "{0.,0.,-3.}" )
        //    .p<std::string>("direction",
        //            "Gun orientation vector (can be overriden in messenger).",
        //        "{0.,0.,1.}" )
        //    .p<double>("energy-MeV",
        //        "Gun energy, MeV (can be further overriden in messenger).",
        //        2e+5 )
        //.end_sect("simpleGun")
    .end_sect( "Geant4" )
    ;
}

void
Geant4Application::_initialize_Geant4_system( goo::dict::Dictionary & commonCfg ) {
    if( app_option<bool>("customExceptionHandler")() ) {
        sV::aux::ExceptionHandler::enable();
    }

    goo::filesystem::Path geomPath = app_option("geometry").as<goo::filesystem::Path>();
    geomPath.interpolator( goo::app<AbstractApplication>().path_interpolator() );

    sV_log2("Reading a GDML geometry from \"%s\".\n",
            geomPath.interpolated().c_str());

    _parser->Read( geomPath.interpolated(),
                   commonCfg["Geant4.gdml.enableXMLSchemaValidation"].as<bool>() );

    if( commonCfg["Geant4.useNIST"].as<bool>() ) {
        sV_log2("Enabling NIST.\n");
        (_NISTMatMan = G4NistManager::Instance())->SetVerbose( g4_verbosity() );
    }
    {
        unsigned int seed = commonCfg["Geant4.randomSeed"].as<unsigned int>();
        if( seed ) {
            sV_log2( "Setting HEP random seed to %u.\n", seed );
            CLHEP::HepRandom::setTheSeed( seed );
        }
    }
    _parser = new G4GDMLParser();
    if( commonCfg["overlapCheck"].as<bool>() ) {
        _parser->SetOverlapCheck( true );
    } else {
        _parser->SetOverlapCheck( false );
    }
}

int
Geant4Application::g4_verbosity() {
    std::string vrbTok = cfg_option<std::string>("Geant4.verbosity");
    if( "application" == vrbTok ) {
        return verbosity();
    } else {
        return atoi( vrbTok.c_str() );
    }
}

void
Geant4Application::g4_abort() {
    if( G4UImanager::GetUIpointer() ) {
        G4UImanager::GetUIpointer()->ApplyCommand("/run/abort");
    }
}

//
//
//

/**Requires G4RunManager instance to be created.
 *
 * According to the Geant4 manual there are three mandatory user classes to be
 * set prior to the run:
 *
 *   1. G4VUserDetectorConstruction -> _initialize_geometry()
 *   2. G4VUserPhysicsList          -> _initialize_physics()
 *   3. G4VUserActionInitialization which should include at least
 *      G4VUserPrimaryGeneratorAction ( _initialize_primary_generator_action() ).
 *
 * Thus, this method does the following, in order:
 *   1. Acquires the GDML setup name from common config.
 *   2. Invokes the _initialize_geometry()
 *   3. Invokes the _initialize_physics()
 *   4. Invokes the _initialize_primary_generator_action()
 * */
void
Geant4Application::_build_up_run() {
    if( cfg_option<bool>("Geant4.customExceptionHandler") ) {
        sV::aux::ExceptionHandler::enable();  // as G4RunManagerKernel overrides our handler in ctr,
                                                // we must re-set it again here.
    }
    _setupName = cfg_option<std::string>("Geant4.gdml.setup");
    // Do the G4 initialization stuff.
    _initialize_geometry();
    // assign physlist:
    _initialize_physics();
    // PGA
    _initialize_primary_generator_action();
}

# ifdef G4_MDL_GUI
int
Geant4Application::_interactive_run( const std::string & macroFilePath ) {
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
        char bf[128];
        snprintf( bf, sizeof(bf),
                  "/vis/verbose %d", cfg_option<int>("Geant4.verbosity"));
        G4UImanager::GetUIpointer()->ApplyCommand( bf );
        snprintf( bf, sizeof(bf),
                  "/control/execute %s", macroFilePath.c_str() );
        sV_log2("Vis manager now executing \"%s\"...\n", macroFilePath.c_str() );
        G4UImanager::GetUIpointer()->ApplyCommand( bf );
        sV_log2("... end of \"%s\" execution.\n", macroFilePath.c_str() );
    }
    # ifdef G4_MDL_VIS
    uiExec->SessionStart();
    delete uiExec;
    # endif

    return rc;
}
# endif

void
Geant4Application::_initialize_geometry() {
    sV_log2("Setting up Geant4 run manager on default volume \"%s\".\n",
                _setupName.c_str());
    G4RunManager::GetRunManager()->SetUserInitialization(
                new extGDML::DetectorConstruction
                        (gdml_parser_ptr()->GetWorldVolume(_setupName)) );
}

void
Geant4Application::_initialize_physics() {
    _TODO_  // TODO: use IndexOfConstructables
    if( cfg_option<std::string>("Geant4.physicsList") != "none" ) {
        // Create a PhysicsList instance if it is not configured to `none':
        G4RunManager::GetRunManager()->SetUserInitialization(
                sV::obtain_physics_list_instance(
                            co()["Geant4.physicsList"].as<std::string>() ) );
    } else {
        sV_logw( "No physics list assigned to MC simulation as there is no "
                 "physicsList option provided.\n" );
    }
}

void
Geant4Application::_initialize_primary_generator_action() {
    _TODO_  // TODO: use IndexOfConstructables
    G4UImessenger * srcMessenger = nullptr;  // TODO: set srcMessenger
    if( "none" != cfg_option<std::string>("Geant4.primaryGenerator") ) {
        G4RunManager::GetRunManager()->SetUserAction( sV::user_primary_generator_action(
                    cfg_option<std::string>("Geant4.primaryGenerator"),
                    srcMessenger
                ) );
    }
}

//
//
//

int
Geant4Application::_batch_run( const std::string & macroFilePath ) {
    char bf[128];
    // TODO: support for "application"!
    snprintf( bf, sizeof(bf),
              "/vis/verbose %d", g4_verbosity() );
    G4UImanager::GetUIpointer()->ApplyCommand( bf );
    snprintf( bf, sizeof(bf),
              "/control/execute %s", macroFilePath.c_str() );
    sV_log2("Vis manager now executing \"%s\"...\n", macroFilePath.c_str() );
    G4UImanager::GetUIpointer()->ApplyCommand( bf );
    sV_log2("... end of \"%s\" execution.\n", macroFilePath.c_str() );
    return EXIT_SUCCESS;
}

//
//
//

/**
 * This method provides a somewhat default initialization logic routing between
 * major configurable VCtr utilities in StromaV.
 *
 * Calls:
 *  - _build_up_run()
 *  - _batch_run() / _interactive_run()
 *
 * This method allocates the default G4RunManager() instance. If you desire to
 * use your own G4RunManager descendant, consider to avoid calling this method.
 * */
int
Geant4Application::_run_session(
        bool isBatch,
        const std::string & macroFilePath ) {
    int rc = EXIT_FAILURE;

    if( !! G4RunManager::GetRunManager() ) {
        emraise( badArchitect, "The G4RunManager instance was created while "
            "execution had been forwarded to Geant4Application::_run_session(). "
            "If you use your own G4RunManager, the _run_session() is not "
            "supposed to be invoked." );
    }

    // Allocate Geant4 run manager.
    G4RunManager * runManager = new G4RunManager();
    // Initializes run manager, geometry, sens. dets, etc.
    _build_up_run();
    // Close geometry and get ready for event loops.
    runManager->Initialize();

    sV_log2("Processing aux info.\n");
    const auto & tagNamesLst = co()["Geant4.gdml.enable-tag"]
                                                    .as_list_of<std::string>();
    extGDML::AuxInfoSet * auxInfoSet =
        new extGDML::AuxInfoSet(
            std::vector<std::string>(tagNamesLst.begin(), tagNamesLst.end()) );
    auxInfoSet->apply( *gdml_parser_ptr() );
    extGDML::extras::apply_styles_selector( _setupName );

    // Forward execution to Geant4 routines.
    if( isBatch ) {
        rc = _batch_run( macroFilePath );
    } else {
        # if G4_MDL_GUI
        rc = _interactive_run( macroFilePath );
        # else
        sV_loge( "Only batch mode available in this build.\n" );
        rc = EXIT_FAILURE;
        # endif
    }

    // Free run manager.
    delete runManager;

    // Free UI session if need.
    # ifdef G4_MDL_VIS
    if( _visManagerPtr ) {
        delete _visManagerPtr;
        _visManagerPtr = nullptr;
    }
    # endif
    return rc;
}

}  // namespace ::sV::mixins
}  // namespace sV

# endif  // GEANT4_MC_MODEL

