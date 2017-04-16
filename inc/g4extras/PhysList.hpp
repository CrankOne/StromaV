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

# ifndef H_STROMA_V_SIM_PHYSICS_LIST_H
# define H_STROMA_V_SIM_PHYSICS_LIST_H

# include "sV_config.h"

# ifdef GEANT4_MC_MODEL

# include "app/app.h"

# include <goo_ansi_escseq.h>

# include <string>
# include <unordered_map>

# include <G4VUserPhysicsList.hh>
# include <G4VModularPhysicsList.hh>

class G4VUserPhysicsList;
class PhysicsListMessenger;

namespace sV {

/** @brief Returns a physics list instance.
 *
 * Designed to always return a pointer to valid physic list instance.
 * See https://geant4.web.cern.ch/geant4/UserDocumentation/UsersGuides/ForApplicationDeveloper/html/ch06.html
 * for details about how those physics lists are constructed.
 *
 *
 */
G4VUserPhysicsList * obtain_physics_list_instance( const std::string & name );

/// Returns a std::vector<std::string> containing available PhysLists.
std::vector<std::string> available_physics_lists();

# ifdef GEANT4_DYNAMIC_PHYSICS

/**@class DynamicPhysicsList
 * @brief Run-time composed physics list class.
 *
 * The approach of dynamically-composed phys. list.
 * This instance should be dynamically formed at run-time
 * according to user-provided list.
 *
 * TODO: This approach is currently UNIMPLEMENTED as it requires
 * huge work with taking into account all Geant4 particles
 * and connected physics.
 */
class DynamicPhysicsList : public G4VUserPhysicsList {
public:
    DynamicPhysicsList();
    ~DynamicPhysicsList();

    // Construct particles
    virtual void ConstructParticle();
    void ConstructBosons();
    void ConstructLeptons();

    virtual void SetCuts();
    //void SetAnalyticSR(G4bool val) {fSRType = val;};

    // Construct processes and register them
    virtual void ConstructProcess();
    void ConstructGeneral();
    void ConstructEM();

private:
    //G4bool                  fSRType;
    PhysicsListMessenger *  _messenger;
};

/**@class ModularPhysicsList
 * @brief Run-time composed physics list class based on native
 * Geant4 G4VModularPhysicsList.
 *
 * Geant4 provides G4VModularPhysicsList which implements
 * dynamic physics modules composition.
 */
class ModularPhysicsList : public G4VModularPhysicsList {
public:
    typedef std::vector<std::string> NamesList;
    static int physicsVerbosity;
public:
    ModularPhysicsList( const NamesList & );
    virtual ~ModularPhysicsList();
    virtual void SetCuts() override;
    static void push_g4_physics_module( const std::string & name, G4VPhysicsConstructor * (*f)() );

    /// Returns a std::vector<std::string> containing available G4VPhysicsConstructor successors.
    static std::vector<std::string> available_physics_modules();

    /// Implemented as a template in order to provide the way of fine customizing via explicit
    /// template specialization.
    template<typename T> static T * construct_physics() {
        auto ptr = new T( physicsVerbosity );
        sV_log2( "Constructing standard physics module \"" ESC_CLRGREEN "%s" ESC_CLRCLEAR "\""
                     " of type %d.\n",
                     ptr->GetPhysicsName().c_str(),
                     ptr->GetPhysicsType() );
        return ptr;
    }

    static void register_modules_at( const NamesList &,
                                     G4VModularPhysicsList * );
private:
    static std::unordered_map<std::string, G4VPhysicsConstructor * (*)()> * _phListCtrsDict;

    friend void __static_fill_physics_modules_dictionary() __attribute__ ((constructor(156)));
};  // class ModularPhysicsList

# define REGISTER_PHYSICS_MODULE( className )                                   \
G4VPhysicsConstructor *                                                         \
_static_construct_ ## className ## _phys_module () {                            \
    return sV::ModularPhysicsList::construct_physics<className>(); }        \
void __static_register_custom_physics_module_ ## className () __attribute__ ((constructor(157))); \
void __static_register_custom_physics_module_ ## className () {                 \
    sV::ModularPhysicsList::push_g4_physics_module( # className ,           \
                   _static_construct_ ## className ##  _phys_module );          \
}

# endif  // GEANT4_DYNAMIC_PHYSICS

}  // namespace sV

# endif  // GEANT4_MC_MODEL

# endif  // H_STROMA_V_SIM_PHYSICS_LIST_H

