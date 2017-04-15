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

# ifndef H_STROMA_V_CUSTOM_EXCEPTION_HANDLER_H
# define H_STROMA_V_CUSTOM_EXCEPTION_HANDLER_H

# include "sV_config.h"

# ifdef GEANT4_MC_MODEL

# include <G4VExceptionHandler.hh>

namespace sV {
namespace aux {

/**@class ExceptionHandler
 * @brief Custom exception handler class overriding default G4 handler.
 *
 * Mainly, this class is reserved for future use. By now, I just need to get
 * a stacktrace on exception.
 * */
class ExceptionHandler : public G4VExceptionHandler {
private:
    static G4VExceptionHandler * _selfPtr,
                               * _oldHandlerPtr;
public:
    ExceptionHandler() { }
    ~ExceptionHandler() { }

    virtual G4bool Notify(const char * originOfException,
                          const char * exceptionCode,
                          G4ExceptionSeverity severity,
                          const char * description) override;

    ExceptionHandler & operator=(const ExceptionHandler &);
    G4int operator==( const ExceptionHandler & right ) const;
    G4int operator!=( const ExceptionHandler & right ) const;

    static void enable();
    static void disable();
};

}  // namespace aux
}  // namespace sV

# endif  // GEANT4_MC_MODEL

# endif  // H_STROMA_V_CUSTOM_EXCEPTION_HANDLER_H

