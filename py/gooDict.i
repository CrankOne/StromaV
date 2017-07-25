%module gooDict

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

%include "std_string.i"
%include "_gooExceptionWrapper.i"

%nodefaultctor goo::dict::InsertionProxy;

%ignore PACKAGE_VERSION;
%ignore GIT_STRING;

/* SWIG of versions at least >=2.0.9 doesn't like the C++11 override/final
 * keywords, so we get rid of them using these macro defs: */
# ifdef SWIG
# define override
# define final
# endif  // SWIG

%include "goo_config.h"

%include "goo_dict/parameter.tcc"
%include "goo_dict/insertion_proxy.tcc"
%include "goo_dict/dict.hpp"

//%pythonprepend goo::dict::InsertionProxy::p( PyObject * args, PyObject * kwargs ) {@sVPy_argspreproc}

%feature("shadow") goo::dict::InsertionProxy::p( PyObject *, PyObject * ) %{
    def p(self, *args, **kwargs):
        return _gooDict.InsertionProxy_p(self, args, kwargs)
%}

%extend goo::dict::InsertionProxy {
    goo::dict::InsertionProxy p( PyObject * args, PyObject * kwargs ) {
        // Actual signature of the function:
        //      ( type, name=None, description=None, shortcut=None )
        // Within the C API the args is an ordinary Python tuple, and kwargs is
        // the usual dictionary, so:

        // - check args:
        if( !PyTuple_Check(args) || 1 != PyTuple_Size(args) ) {
            emraise( badParameter, "Tuple of size carrying type expected as "
                "the first argument of p().");
        }
        PyObject * pyType_ = PyTuple_GET_ITEM(args, 0);
        if( ! ( PyType_Check(pyType_)
                || PyString_Check(pyType_)
                || PyTuple_Check(pyType_) ) ) {
            emraise( badParameter, "First argument of p() is being not "
                "a Python type or tuple, nor the string object." );
        }
        // - check kwargs:
        if( !PyDict_Check(kwargs) ) {
            emraise( badParameter, "Keywoard arguments expected: "
                "shortcut, description, name, default." );
        }

        # define _M_get_kwarg( nm, strNm )                          \
            PyObject * nm = PyDict_GetItemString( kwargs, strNm );  \
            if(nm) { PyDict_DelItemString( kwargs, strNm ); }       \
            if( Py_None == nm ) { nm = NULL; }
        _M_get_kwarg( pyName,     "name" );
        _M_get_kwarg( pyShortcut, "shortcut" );
        _M_get_kwarg( pyDescr,    "description" );
        _M_get_kwarg( pyDefault_, "default" );
        # undef _M_get_kwarg

        if( PyDict_Size( kwargs ) ) {
            PyObject * dictRepr = PyObject_Repr( kwargs );
            emraise( badParameter, "Unused keyword(s) for p(): %s. "
                "Expected: shortcut, description, name, default.",
                            PyString_AsString( dictRepr ) );
        }

        static char _empty_description[] = "<no description given>";
        char shortcut = 0x00;
        const char * description = _empty_description;
        const char * name = NULL;

        if( pyName ) {
            if( !PyString_Check(pyName) ) {
                emraise(badParameter, "Parameter name has to be a string.");
            }
            name = PyString_AS_STRING(pyName);
        }
        if( pyDescr ) {
            if( !PyString_Check(pyDescr) ) {
                emraise(badParameter, "Description has to be a string.");
            }
            description = PyString_AS_STRING(pyDescr);
        }
        if( pyShortcut ) {
            if( !PyString_Check(pyShortcut) || 1 != PyString_Size(pyShortcut) ) {
                emraise(badParameter, "Shortcut has to be a string of size 1.");
            }
            shortcut = PyString_AS_STRING(pyShortcut)[0];
        }

        # define _M_insert_parameter( parType, parseDefault )               \
            if( name && shortcut && pyDefault_ ) {                          \
                $self->p<parType>( shortcut, name, description, parseDefault(pyDefault_) ); \
            } else if( name && shortcut && !pyDefault_ ) {                  \
                $self->p<parType>( shortcut, name, description );           \
            } else if( name && (!shortcut) && pyDefault_ ) {                \
                $self->p<parType>( name, description, parseDefault(pyDefault_) );  \
            } else if( name && (!shortcut) && !pyDefault_ ) {               \
                $self->p<parType>( name, description );                     \
            } else if( (!name) && shortcut && pyDefault_ ) {                \
                $self->p<parType>( shortcut, description, parseDefault(pyDefault_) );  \
            } else if( (!name) && shortcut && !pyDefault_ ) {               \
                $self->p<parType>( shortcut, description );                 \
            } else {                                                        \
                emraise( badParameter, "Unable to insert parameter: either the " \
                    "shortcut, the name, or both have to be provided." );   \
            }

        PyTypeObject * pyType = (PyTypeObject *) pyType_;
        if( PyTuple_Check( pyType_ ) ) {
            emraise( unimplemented, "Tuple type parameter insertion is not "
                "yet implemented.");  // TODO
        } else if( PyType_IsSubtype( pyType, &PyBool_Type ) ) {
            _M_insert_parameter( bool, PyInt_AsLong );
        } else if( PyType_IsSubtype( pyType, &PyString_Type ) ) {
            if( name && shortcut && pyDefault_ ) {
                $self->p<std::string>( shortcut, name, description, PyString_AS_STRING(pyDefault_) );
            } else if( name && shortcut && !pyDefault_ ) {
                $self->p<std::string>( shortcut, name, description );
            } else if( name && (!shortcut) && pyDefault_ ) {
                $self->p<std::string>( name, description, PyString_AS_STRING(pyDefault_) );
            } else if( name && (!shortcut) && !pyDefault_ ) {
                $self->p<std::string>( name, description );
            } else if( (!name) && shortcut && pyDefault_ ) {
                $self->p<std::string>( shortcut, nullptr, description, PyString_AS_STRING(pyDefault_) );
            } else if( (!name) && shortcut && !pyDefault_ ) {
                $self->p<std::string>( shortcut, nullptr, description );
            } else {
                emraise( badParameter, "Unable to insert parameter: either the "
                    "shortcut, the name, or both have to be provided." );
            }
        } else if( PyType_IsSubtype( pyType, &PyInt_Type ) ) {
            _M_insert_parameter( long, PyInt_AsLong );
        } else if( PyType_IsSubtype( pyType, &PyFloat_Type ) ) {
            _M_insert_parameter( double, PyFloat_AsDouble );
        } else if( PyString_Check(pyType_) ) {
            emraise( unimplemented, "Dynamic type resolution and parameter "
                    "insertion is not yet implemented." );  // TODO
        } else {
            PyObject * typeRepr = PyObject_Repr( pyType_ );
            emraise( badParameter, "Unknown type provided: %s.",
                            PyString_AsString( typeRepr ) );
        }
        // TODO: https://docs.python.org/2/c-api/exceptions.html#c.PyErr_Occurred

        return *$self;
    }
}

//%dictargs( p, goo::dict::InsertionProxy &, PyObject )

//%template(int_param) goo::dict::InsertionProxy::p<int>;

%{

#include "sV_config.h"

#if !defined( PYTHON_BINDINGS )
#error "PYTHON_BINDINGS is not defined. Unable to build app py-wrapper module."
#endif

%}

// vim: ft=swig

