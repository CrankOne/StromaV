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

# ifndef H_STROMA_V_CERN_GEANT4_FRAMEWORK_BASED_APPLICATION_MIXIN_H
# define H_STROMA_V_CERN_GEANT4_FRAMEWORK_BASED_APPLICATION_MIXIN_H

# include "sV_config.h"

# ifdef GEANT4_MC_MODEL

# include "app/abstract.hpp"

class G4NistManager;
class G4GDMLParser;
class G4VisManager;
class G4VUserPrimaryGeneratorAction;

namespace sV {
namespace mixins {

/**@class Geant4Application
 *
 * @brief Application mixin implementing interaction with
 * Geant4 MC engine.
 *
 * This class offers an application implementation that inherits all the
 * neccessary StromaV run-time parametrization, including:
 *
 *  - Acquizition and parsing of geometry defined as parameterized template
 *    GDML, including fetching from remote destination.
 *  - Support for physics and physics list defined using virtual constructor
 *    entries (based on StromaV IndexOfConstructables class) with all
 *    auxilliary tags and extensions supplied within ext.gdml package.
 *  - Support for sensitive detectors defined using virtual constructor
 *    entries (based on StromaV IndexOfConstructables class).
 *  - Support for primary generators action defined using virtual constructor
 *    entries (based on StromaV IndexOfConstructables class).
 *
 * User classes may override default implementation offered by this class that
 * is defined in the set of virtual methods starting with _run_session().
 *
 * The _run_session() invokes the _build_up_run() to set up mandatory Geant4
 * classes and forwards execution to _interactive_run() or _batch_run()
 * depending on whether the `--batch` flag was set. The _run_session() will
 * also invoke callbacks associated to auxilliary tags using the
 * extGDML::AuxInfoSet::apply() method.
 *
 * The _build_up_run() invokes, in order, the _initialize_geometry() to
 * perform geometry construction, the _initialize_physics() to initialize
 * dynamic physics, and the _initialize_primary_generator_action() to construct
 * the primary generator action instance.
 *
 * @ingroup app
 * @ingroup mc
 * @ingroup g4
 */
class Geant4Application : public virtual AbstractApplication {
protected:
    // Overrides Geant4 default logging behaviour.
    // TODO: supersede its functions with sV::logging::*
    class AppSession;
protected:
    /// A ptr to internal sV's implementation of Geant4 session.
    AppSession * _session;
    /// Pointer to NIST material manager (if enabled).
    G4NistManager * _NISTMatMan;
    /// A GDML parser instance pointer.
    G4GDMLParser  * _parser;  // todo: delete in dtr?
    /// G4 vis manager instance pointer (if enabled).
    G4VisManager * _visManagerPtr;
    /// The name of currently viewing setup.
    std::string _setupName;
protected:
    /// Forwards execution to Geant4 system. Designed to be invoked from
    /// AbstractApplication::_V_run() directly, without any additional
    /// preparations.
    virtual int _run_session();

    /// Performs acquizition of the setup name, initializes geometry, physics
    /// and PGA. Requires G4RunManager to be created upon invokation.
    /// Called by _run_session().
    virtual void _build_up_run();
    /// Called by _run_session().
    virtual int _interactive_run( const std::string & macroFilePath );  // can be empty
    /// Called by _run_session().
    virtual int _batch_run( const std::string & macroFilePath );  // can NOT be empty

    /// Called by _build_up_run().
    virtual void _initialize_geometry();
    /// Called by _build_up_run().
    virtual void _initialize_physics();
    /// Called by _build_up_run().
    virtual void _initialize_primary_generator_action();

    /// Called from AbstractApplication once this mixin was included in
    /// inheritance chain: appends common config with Geant4-specific options.
    void _append_Geant4_config_options( goo::dict::Dictionary & commonCfg );
    /// Called from AbstractApplication once this mixin was included in
    /// inheritance chain: configures Geant4 system routines. Allocates GDML
    /// parser instance (G4GDMLParser class).
    void _initialize_Geant4_system( goo::dict::Dictionary & commonCfg );
public:
    Geant4Application( AbstractApplication::Config * );
    virtual ~Geant4Application();
    G4GDMLParser * gdml_parser_ptr() { return _parser; }
    int g4_verbosity();
    static void g4_abort();

    friend class AbstractApplication;
};  // class Geant4Application
/**@}*/

}  // namespace mixins
}  // namespace sV



//
// Primary Generator Action registering macros

# define StromaV_DEFINE_GEANT4_PGA( name, cxxClassName )    \
StromaV_DEFINE_CONSTRUCTABLE(                               \
        ::G4VUserPrimaryGeneratorAction,                    \
        name, cxxClassName)

# define StromaV_DEFINE_GEANT4_PGA_MCONF( name, cxxClassName )  \
StromaV_DEFINE_STD_CONSTRUCTABLE_MCONF( cxxClassName, name, ::G4VUserPrimaryGeneratorAction )

//
//

# endif  // GEANT4_MC_MODEL
# endif  // H_STROMA_V_CERN_GEANT4_FRAMEWORK_BASED_APPLICATION_MIXIN_H

