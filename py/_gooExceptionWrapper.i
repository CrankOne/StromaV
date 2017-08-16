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

/* This SWIG include-only module introduces wrapping exception to handle the
 * Goo error as native python exception to establish seamless error-reporting 
 * mechanism between wrapped C++ code and python environment.
 */

%{
# include "app/py_session.hpp"
%}

#ifdef SWIGPYTHON

// NOTE: the Swig::DirectorException block is usually unused, since we
// supersede default director exception with goo::Exception in director:except.
// It is possible, however, that some external code may still throw this C++
// exception. In this case, the Swig::DirectorException block ma help to
// identify some additional details in future. Even now it is much better to
// get the Python exception instead of unhandled C++ exception.
%exception {
    try {
        $action
    } catch( goo::Exception & e ) {
        if( sV::PythonSession::exception_type() ) {
            PyErr_SetObject( sV::PythonSession::exception_type(),
                             sV::PythonSession::goo_exception2dict(e) );
        } else {
            PyErr_SetString( PyExc_RuntimeError, e.what() );
            e.dump( std::cerr );
        }
        return NULL;
    }
    # ifdef SWIG_PYTHON_DIRECTOR_VTABLE
    catch( Swig::DirectorException & e ) {
        PyErr_SetString( PyExc_RuntimeError, e.what() );
        return NULL;
    }
    # endif
    // For other type of exceptions one may add:
    // catch( na64ee::Exception & e ) {
    //     PyErr_SetString( PyExc_RuntimeError, e.what() );
    //     return NULL;
    // }
}

%feature("director:except") {
    if( $error != NULL ) {
        PyObject * pyErrPtr = PyErr_Occurred();
        if( !pyErrPtr ) {
            emraise( thirdParty, "SWIG director common error. "
                " Target method: $symname()." );
        }
        // TODO: think about the way to attach the Python exception to
        // goo::Exception. For now, we just dump it with PyErr_PrintEx():
        PyErr_PrintEx(1);
        emraise( thirdParty, "SWIG director detects error in python code."
                " Target method: \"$symname\"." );
    }
}

#endif  //ifdef SWIGPYTHON

// vim: ft=swig
