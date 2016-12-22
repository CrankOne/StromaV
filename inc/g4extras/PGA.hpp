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

# ifndef H_STROMA_V_GEANT4_PRIMARY_GENERATOR_REGISTRY_H
# define H_STROMA_V_GEANT4_PRIMARY_GENERATOR_REGISTRY_H

# include "../config.h"

# ifdef GEANT4_MC_MODEL
# include <list>
# include <string>

class G4VUserPrimaryGeneratorAction;
class G4UImessenger;

namespace sV {

/// List available generators.
std::list<std::string> user_primary_generator_actions_list();

/// Creates and returns primary action instance according to selected type.
///
/// Note: messenger pointer may not be set, if PGA soen't imply it ti be.
G4VUserPrimaryGeneratorAction * user_primary_generator_action(
        const std::string &,
        G4UImessenger *& );

namespace aux {
typedef G4VUserPrimaryGeneratorAction * (*PGAConstructor)( G4UImessenger *& );

/// (internal) Registers new PGA constructor in system.
void register_PGA_ctr( const std::string & name, PGAConstructor );
}  // namespace aux

}  // namespace sV

# define StromaV_PGA_CONSTRUCTOR( pgaName, ctrName )     \
static void _static_ ## pgaName ## _ctr_enlst           \
    ( ) __attribute__(( constructor(156) ));            \
static void _static_ ## pgaName ## _ctr_enlst           \
    ( ) {                                               \
sV::aux::register_PGA_ctr( # pgaName, ctrName ); }



# endif  // GEANT4_MC_MODEL

# endif  // H_STROMA_V_GEANT4_PRIMARY_GENERATOR_REGISTRY_H

