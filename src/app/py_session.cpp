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

# include "app/py_session.hpp"

# ifdef PYTHON_BINDINGS

namespace sV {

# if 0
static const char _static_dftPySessionArgs[] = "sVPy";

static goo::dict::Configuration * _static_dftPySessionConf = nullptr;

goo::dict::Configuration *
default_pythob_session_app_conf() {
    if( _static_dftPySessionConf ) {
        return _static_dftPySessionConf;
    }

    char ** argv;
    int argc = goo::dict::Configuration::tokenize_string( _static_dftPySessionArgs, argv );
    _static_dftPySessionConf = new goo::dict::Configuration( "sVPy",
        "StromaV python session configuration." );
    _static_dftPySessionConf->extract( argc, argv, true );

    // goo::dict::Configuration::free_tokens( argc, argv );

    return _static_dftPySessionConf;
}
# endif

char ** PythonSession::_locArgv = nullptr;
int PythonSession::_locArgc = 0;
goo::dict::Configuration * PythonSession::_locConf = nullptr;
PyObject * PythonSession::_gooExceptionTypePtr = NULL;

PythonSession::PythonSession( sV::AbstractApplication::Config * cfgPtr ) :
                   AbstractApplication( cfgPtr ),
                   sV::AnalysisApplication( cfgPtr ) {
}

void
PythonSession::init_from_string(
            const char * appName,
            const char * appDescription,
            const char * strtoks_argv ) {
    _locConf = new goo::dict::Configuration( appName, appDescription );
    _locArgc = goo::dict::Configuration::tokenize_string( strtoks_argv, _locArgv );
    _locConf->extract( _locArgc, _locArgv, true );
    PythonSession * app = new PythonSession( _locConf );
    sV::AbstractApplication::Parent::init( _locArgc, _locArgv, app );
}

PythonSession::~PythonSession() {
    goo::dict::Configuration::free_tokens( _locArgc, _locArgv );
    delete _locConf;
}

void
PythonSession::initialize_exception_type() {
    auto exceptionModule = PyImport_ImportModule("StromaV");
    if( exceptionModule ) {
        PyObject * moduleDict = PyModule_GetDict( exceptionModule );
        _gooExceptionTypePtr = PyDict_GetItemString( moduleDict, "GooException" );
        if( !_gooExceptionTypePtr ) {
            PyErr_SetString( PyExc_RuntimeError,
                "Failed to locate native \"GooException\" python class in "
                ".gooException module." );
        } else {
            Py_INCREF( _gooExceptionTypePtr );
        }
    } else {
        PyErr_SetString( PyExc_RuntimeError,
                "Failed to locate \"StromaV\" module." );
    }
}

PyObject *
PythonSession::exception_type() {
    if( !_gooExceptionTypePtr ) {
        initialize_exception_type();
    }
    return _gooExceptionTypePtr;
}

/* A helper function extracting stacktrace information from Goo exception into
 * python dictionary for further use within native python exception. */
PyObject *
PythonSession::goo_exception2dict( const goo::Exception & e ) {
    PyObject * ret = PyDict_New();
    assert(PyDict_Check(ret));

    // Basic fields
    PyDict_SetItemString( ret, "code", Py_BuildValue("i", e.code()) );
    PyDict_SetItemString( ret, "what", Py_BuildValue("s", e.what()) );

    // C++ stacktrace:
    # ifndef EM_STACK_UNWINDING
    PyDict_SetItemString( ret, "cppStackDump", Py_BuildValue("s",
        "C++ stacktrace is unavailable in current build configuration.") );
    # else  // EM_STACK_UNWINDING
    std::stringstream ss;
    e.dump(ss);
    PyDict_SetItemString( ret, "cppStackDump", Py_BuildValue("s",
        ss.str().c_str()) );
    # endif  // EM_STACK_UNWINDING
    return ret;
}

void
PythonSession::test_exception_throw() {
    emraise( common, "This is a test exception instance thrown to check the "
        "wrapper code." );
}

}  // namespace sV

# endif

