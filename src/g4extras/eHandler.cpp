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

# include "sV_config.h"

# ifdef GEANT4_MC_MODEL

# include "g4extras/eHandler.hpp"
# include "app/abstract.hpp"
# include "app/mixins/geant4.hpp"

# include <goo_exception.hpp>

# include <G4VExceptionHandler.hh>
# include <G4StateManager.hh>

// # pragma GCC diagnostic push
// # pragma GCC diagnostic ignored "-Wdeprecated-register"  // "register" is depr-d
# include <G4RunManager.hh>
// # pragma GCC diagnostic pop

namespace sV {
namespace aux {

G4VExceptionHandler
    * ExceptionHandler::_selfPtr       = nullptr,
    * ExceptionHandler::_oldHandlerPtr = nullptr;

G4bool
ExceptionHandler::Notify(const char * originOfException,
              const char * exceptionCode,
              G4ExceptionSeverity severity,
              const char * description) {
    //std::cerr << "*** *** *** ***" << std::endl; //XXX
    //std::cerr.flush();  // XXX
    //G4cout.flush();
    bool abortionForCoreDump = false;
    sV_loge( "G4Exception : " ESC_BLDYELLOW "%s" ESC_CLRCLEAR "\n", exceptionCode );
    sV_loge( "  issued by : %s\n", originOfException );
    sV_loge( ESC_BLDYELLOW "%s" ESC_CLRCLEAR "\n", description );
    switch(severity) {
        case FatalException : {
            sV_loge( " (G4 fatal)\n");
            abortionForCoreDump = true;
        } break;
        case FatalErrorInArgument : {
            sV_loge( " (G4 fatal error in argument)\n");
            abortionForCoreDump = true;
        } break;
        case RunMustBeAborted : {
            sV_loge( " (G4 fatal in run; run must be aborted)\n");
            G4RunManager::GetRunManager()->AbortRun(false);
            abortionForCoreDump = false;
        } break;
        case EventMustBeAborted : {
            sV_loge( " (G4 runtime error; event must be aborted)\n");
            G4RunManager::GetRunManager()->AbortEvent();
            abortionForCoreDump = false;
        } break;
        default : {
            sV_loge( " (G4 no abort --- it's just a warning)\n");
            abortionForCoreDump = false;
        } break;
    }
    {  // dump stacktrace
        goo::Exception ge( goo::Exception::thirdParty, "Geant4 error." );
        ge.dump( goo::app<AbstractApplication>().es() );
    }
    {   // dump contents of G4's cout/cerr to terminal as it often contains
        // useful information (see, i.e. issue #7 on our redmine).
        //goo::app<mixins::Geant4Application>().stdout_stream()
    }
    return abortionForCoreDump;
}

void
ExceptionHandler::enable() {
    if( !_selfPtr ) {
        _oldHandlerPtr =
            G4StateManager::GetStateManager()->GetExceptionHandler();
        G4StateManager::GetStateManager()->SetExceptionHandler(
                _selfPtr = new sV::aux::ExceptionHandler() // todo: configure it here?
            );
        sV_log1( "Custom exception handler enabled for Geant4 (ctrd).\n" );
    } else {
        if( _selfPtr != G4StateManager::GetStateManager()->GetExceptionHandler() ) {
            sV_logw( "Custom exception handler was overriden some time before. Re-setting back.\n" );
            G4StateManager::GetStateManager()->SetExceptionHandler( _selfPtr );
        }
    }
}

void
ExceptionHandler::disable() {
    sV_log1( "Custom exception handler disabled for Geant4.\n" );
    G4StateManager::GetStateManager()->SetExceptionHandler( _oldHandlerPtr );
    delete sV::aux::ExceptionHandler::_selfPtr;
}

ExceptionHandler &
ExceptionHandler::operator=(const ExceptionHandler &) {
    return *this;
}

G4int
ExceptionHandler::operator==( const ExceptionHandler & right ) const {
    return (this == &right);
}

G4int
ExceptionHandler::operator!=( const ExceptionHandler & right ) const {
    return (this != &right);
}

}  // namespace aux
}  // namespace sV

# endif  // GEANT4_MC_MODEL

