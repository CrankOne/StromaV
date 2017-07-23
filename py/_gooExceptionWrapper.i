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
    // For other type of exceptions one may add:
    // catch( na64ee::Exception & e ) {
    //     PyErr_SetString( PyExc_RuntimeError, e.what() );
    //     return NULL;
    // }
}

// vim: ft=swig
