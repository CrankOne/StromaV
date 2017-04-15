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

# include "sV_config.h"

# ifdef GEANT4_MC_MODEL

# include "app/app.h"

# include "g4extras/PGA.hpp"

# include <goo_exception.hpp>

# include <map>

namespace sV {

static std::map<std::string, aux::PGAConstructor> * _static_PGARegistryPtr = nullptr;

std::list<std::string>
user_primary_generator_actions_list() {
    std::list<std::string> res;
    for(auto it : *_static_PGARegistryPtr) {
        res.push_back(it.first);
    }
    return res;
}

G4VUserPrimaryGeneratorAction *
user_primary_generator_action( const std::string & name, G4UImessenger *& messengerPtr ) {
    if( ! _static_PGARegistryPtr ) {
        emraise( unsupported, "Have no any dynamic PGA constructor registered." );
    }
    auto it = _static_PGARegistryPtr->find( name );
    if( _static_PGARegistryPtr->end() == it ) {
        emraise( noSuchKey, "Have no PGA constructor named \"%s\".", name.c_str() );
    }
    G4VUserPrimaryGeneratorAction * pgaInstancePtr = it->second( messengerPtr );
    sV_log2( "PGA \"" ESC_CLRGREEN "%s" ESC_CLRCLEAR "\" created at %p.\n", name.c_str(), pgaInstancePtr );
    return pgaInstancePtr;
}

namespace aux {
void
register_PGA_ctr( const std::string & name, aux::PGAConstructor ctr ) {
    if( !_static_PGARegistryPtr ) {
        _static_PGARegistryPtr = new std::map<std::string, aux::PGAConstructor>();
    }
    (*_static_PGARegistryPtr)[name] = ctr;
}
}  // namespace aux

}  // namespace sV

# endif  // GEANT4_MC_MODEL

