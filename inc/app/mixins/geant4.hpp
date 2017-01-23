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

# include "../../config.h"

# ifdef GEANT4_MC_MODEL

# include "app/abstract.hpp"

class G4NistManager;
class G4GDMLParser;
class G4VisManager;

namespace sV {

namespace mixins {

/**@class Geant4Application
 *
 * @brief Application mixin implementing interaction with
 * Geant4 MC engine.
 *
 * @ingroup app
 * @ingroup mc
 * @ingroup g4
 */
class Geant4Application : public virtual AbstractApplication {
protected:
    // Overrides Geant4 default logging behaviour. TODO: further
    // development of this stream-redirection class to not only
    // store the messages in stringstreams.
    class AppSession;
private:
    static Geant4Application * _self_Geant4ApplicationPtr;
protected:
    AppSession * _session;
    G4NistManager * _NISTMatMan;
    G4GDMLParser  * _parser;
    G4VisManager * _visManagerPtr;

    int _argc;
    char * const * _argv;

    std::string _setupName;

    /// Used in overriden _V_construct_config_object() in order to
    /// remember args for G4 executive.
    void _set_cmd_args( int argc, char * const argv[] ) const;
public:
    Geant4Application( AbstractApplication::Config * );
    virtual ~Geant4Application();
    G4GDMLParser * gdml_parser_ptr() { return _parser; }
    static void g4_abort();
protected:
    virtual po::options_description _geant4_options() const;
    virtual po::options_description _geant4_gdml_options() const;
    virtual void _treat_geant4_options(         const po::variables_map & );
    virtual void _treat_geant4_gdml_options(    const po::variables_map & );
    void _clear_geant4_options(         const po::variables_map & );
    void _clear_geant4_gdml_options(    const po::variables_map & );

    virtual void _initialize_geometry();
    virtual void _initialize_physics();
    // Doubtfull. Follow the Geant4 manual there are three manatory user
    // classes:
    // 1) G4VUserDetectorConstruction -> _initialize_geometry
    // 2) G4VUserPhysicsList          -> _initialize_physics
    // 3) G4VUserActionInitialization which should include atleast
    // G4VUserPrimaryGeneratorAction
    virtual void _initialize_primary_generator_action();

    /// Note: do not be misleaded by name --- may run terminal-interactive mode
    /// also.
    virtual int _gui_run( const std::string & macroFilePath );  // can be empty
    virtual void _build_up_run();
    virtual int _batch_run( const std::string & macroFilePath );  // can not be empty
    virtual int _run_session( bool isBatch, const std::string & macroFilePath );
};  // class Geant4Application
/**@}*/

}  // namespace mixins

}  // namespace sV

# endif  // GEANT4_MC_MODEL
# endif  // H_STROMA_V_CERN_GEANT4_FRAMEWORK_BASED_APPLICATION_MIXIN_H

