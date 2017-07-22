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

# ifndef H_STROMA_V_PYTHON_SESSION_APP_H
# define H_STROMA_V_PYTHON_SESSION_APP_H

# include "sV_config.h"

# ifdef PYTHON_BINDINGS

# include "app/analysis.hpp"

//# include <Python.h>

namespace sV {

// Returns an instance to default static python "app" conf. XXX
//goo::dict::Configuration * default_pythob_session_app_conf(); XXX

/**@class PythonSession
 * @brief A sV's wrapper around AbstractApplication for Python.
 *
 * This class implements some additional functions that may be required within
 * working Python session. Most of interfacing methods providing runtime logic
 * are merely a stubs here since actual application logic has to be provided
 * by Python scripts. It's _V_run() methad will never be invoked.
 *
 * It is implied that new instance of application is created once StromaV any
 * module is imported (via root __init__.py script). The instance will be
 * configured to fit, whenever it possible, the running python session by
 * redirecting standard streams, logging, intercepting exceptions, etc.
 * */
class PythonSession : public AnalysisApplication {
private:
    static char ** _locArgv;
    static int _locArgc;
    static goo::dict::Configuration * _locConf;
protected:
    /// Has to never be invoked.
    virtual int _V_run() override { _FORBIDDEN_CALL_; }
    PythonSession( sV::AbstractApplication::Config * cfgPtr );
public:
    ~PythonSession();

    /// A special static method designed for immediate initialization of python
    /// session with joined argv[]-string acquired with
    /// Python ' '.join(sys.argv) or similar.
    static void init_from_string(
            const char * appName,
            const char * appDescription,
            const char * strtoks_argv );
};  // class PythonSession

}  // namespace sV

# endif  // PYTHON_BINDINGS

# endif  // H_STROMA_V_PYTHON_SESSION_APP_H

