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

%runtime %{
#include "goo_dict/dict.hpp"

static void _insert_parameter(
        goo::dict::InsertionProxy & self,
        PyObject * pyType_,
        char shortcut,
        const char * name,
        const char * description,
        PyObject * pyDefault_
    );

//
// Generic template value acquizition helpers

template<typename T> T
get_C_value(PyObject *);  // None default implementation

template<> bool
get_C_value<bool>(PyObject * o) {
    // Note: implemented as subclass of integers
    return PyObject_IsTrue( o );
}

template<> long
get_C_value<long>(PyObject * o) {
    return PyInt_AsLong( o );
}

template<> double
get_C_value<double>(PyObject * o) {
    return PyFloat_AsDouble( o );
}

template<> std::string
get_C_value<std::string>(PyObject * o) {
    return PyString_AsString( o );
}

// ...

template<typename T> std::list<T>
get_C_list( PyObject * pyTuple ) {
    std::list<T> lst;
    for( ssize_t i = 0; i < PyTuple_Size(pyTuple); ++i ) {
        PyObject * o = PyTuple_GET_ITEM( pyTuple, i );
        lst.push_back( get_C_value<T>( o ) );
    }
    return lst;
}

%}

//%pythonprepend goo::dict::InsertionProxy::p( PyObject * args, PyObject * kwargs ) {@sVPy_argspreproc}

%feature("shadow") goo::dict::InsertionProxy::p( PyObject *, PyObject * ) %{
    def p(self, *args, **kwargs):
        return _gooDict.InsertionProxy_p(self, args, kwargs)
%}

%extend goo::dict::InsertionProxy {
    goo::dict::InsertionProxy p( PyObject * args, PyObject * kwargs ) {
        // Actual signature of the function:
        //      ( type, name=None, description=None, shortcut=None, required=False )
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
        _M_get_kwarg( pyName,       "name" );
        _M_get_kwarg( pyShortcut,   "shortcut" );
        _M_get_kwarg( pyDescr,      "description" );
        _M_get_kwarg( pyDefault_,   "default" );
        _M_get_kwarg( pyRequired_,  "required" );
        # undef _M_get_kwarg

        if( PyDict_Size( kwargs ) ) {
            PyObject * dictRepr = PyObject_Repr( kwargs );
            emraise( badParameter, "Unused keyword(s) for p(): %s. "
                "Expected: shortcut, description, name, default, required.",
                            PyString_AsString( dictRepr ) );
        }

        static char _empty_description[] = "<no description given>";
        char shortcut = 0x00;
        const char * description = _empty_description;
        const char * name = NULL;
        bool required = false;

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
        if( pyRequired_ ) {
            if( !PyBool_Check(pyRequired_) || 1 != PyString_Size(pyShortcut) ) {
                emraise(badParameter, "Shortcut has to be a string of size 1.");
            }
            required = PyObject_IsTrue(pyShortcut);
        }
        _insert_parameter( *$self, pyType_, shortcut, name, description, pyDefault_ );
        // TODO: https://docs.python.org/2/c-api/exceptions.html#c.PyErr_Occurred

        return *$self;
    }
}

//%dictargs( p, goo::dict::InsertionProxy &, PyObject )

//%template(int_param) goo::dict::InsertionProxy::p<int>;

%{

#include "sV_config.h"

#if !defined( PYTHON_BINDINGS )
#error "PYTHON_BINDINGS is not defined. Unable to build goo's dicts py-wrapper module."
#endif

# define _M_insert_parameter_generic( parType ) \
    if( name && shortcut && pyDefault_ ) {                          \
        self.p<parType>( shortcut, name, description, get_C_value<parType>(pyDefault_) ); \
    } else if( name && shortcut && !pyDefault_ ) {                  \
        self.p<parType>( shortcut, name, description );             \
    } else if( name && (!shortcut) && pyDefault_ ) {                \
        self.p<parType>( name, description, get_C_value<parType>(pyDefault_) );  \
    } else if( name && (!shortcut) && !pyDefault_ ) {               \
        self.p<parType>( name, description );                       \
    }

# define _M_insert_parameter( parType )                             \
    _M_insert_parameter_generic(parType)                            \
     else if( (!name) && shortcut && pyDefault_ ) {                 \
        self.p<parType>( shortcut, description, get_C_value<parType>(pyDefault_) );  \
    } else if( (!name) && shortcut && !pyDefault_ ) {               \
        self.p<parType>( shortcut, description );                   \
    } else {                                                        \
        emraise( badParameter, "Unable to insert parameter: either the " \
            "shortcut, the name, or both have to be provided." );   \
    }

# define _M_insert_parameter_string( parType )                      \
    _M_insert_parameter_generic(parType)                            \
    else if( (!name) && shortcut && pyDefault_ ) {                  \
        self.p<parType>( shortcut, nullptr, description, get_C_value<parType>(pyDefault_) );  \
    } else if( (!name) && shortcut && !pyDefault_ ) {               \
        self.p<parType>( shortcut, nullptr, description );          \
    } else {                                                        \
        emraise( badParameter, "Unable to insert parameter: either the "  \
            "shortcut, the name, or both have to be provided." );   \
    }

# define _M_insert_list_p( type, ... ) self.insert_copy_of( goo::dict::Parameter<std::list<type>>( __VA_ARGS__ ) )

# define _M_insert_parameter_list( T, M )                           \
    if( pyDefault_ ) {                                              \
        if( shortcut && name ) {                                    \
            M( T, get_C_list<T>(pyDefault_), shortcut, name, description );  \
        } else if( (!shortcut) && name ) {                          \
            M( T, get_C_list<T>(pyDefault_), name, description );   \
        } else if( shortcut && !name ) {                            \
            M( T, get_C_list<T>(pyDefault_), shortcut, description );  \
        }                                                           \
    } else {                                                        \
        if( shortcut && name ) {                                    \
            M( T, shortcut, name, description );                    \
        } else if( (!shortcut) && name ) {                          \
            M( T, name, description );                              \
        } else if( shortcut && !name ) {                            \
            M( T, shortcut, description );                          \
        }                                                           \
    }
    
# define _M_insert_parameter_list_strings( T, M )                   \
    if( pyDefault_ ) {                                              \
        if( shortcut && name ) {                                    \
            M( T, get_C_list<T>(pyDefault_), shortcut, name, description );  \
        } else if( (!shortcut) && name ) {                          \
            M( T, get_C_list<T>(pyDefault_), name, description );   \
        } else if( shortcut && !name ) {                            \
            M( T, get_C_list<T>(pyDefault_), shortcut, nullptr, description );  \
        }                                                           \
    } else {                                                        \
        if( shortcut && name ) {                                    \
            M( T, shortcut, name, description );                    \
        } else if( (!shortcut) && name ) {                          \
            M( T, name, description );                              \
        } else if( shortcut && !name ) {                            \
            M( T, shortcut, nullptr, description );                 \
        }                                                           \
    }

static
void _insert_parameter(
        goo::dict::InsertionProxy & self,
        PyObject * pyType_,
        char shortcut,
        const char * name,
        const char * description,
        PyObject * pyDefault_
    ) {
    PyTypeObject * pyType = (PyTypeObject *) pyType_;
    if( ! (name || shortcut) ) {
        emraise( badParameter, "At least one of the pair shortcut/name "
                "keyword argument has to be provided for list parameter." )
    }
    if( PyTuple_Check( pyType_ ) && 1 == PyTuple_Size(pyType_) ) {
        pyType_ = PyTuple_GET_ITEM(pyType_, 0);
        if( ! (pyType_ && PyType_Check(pyType_)) ) {
            emraise( badParameter, "First argument of p() is not "
                "a Python tuple containing the single Python type "
                "element." );
        }
        PyTypeObject * pyType = (PyTypeObject *) pyType_;
        // Tuple objects:
        if( PyType_IsSubtype( pyType, &PyBool_Type ) ) {
            _M_insert_parameter_list( bool, _M_insert_list_p )
        } else if( PyType_IsSubtype( pyType, &PyString_Type ) ) {
            _M_insert_parameter_list_strings( std::string, _M_insert_list_p )
        } else if( PyType_IsSubtype( pyType, &PyInt_Type ) ) {
            _M_insert_parameter_list( long, _M_insert_list_p )
        } else if( PyType_IsSubtype( pyType, &PyFloat_Type ) ) {
            _M_insert_parameter_list( double, _M_insert_list_p )
        } else if( PyString_Check(pyType_) ) {
            emraise( unimplemented, "Dynamic type resolution and parameter "
                    "insertion is not yet implemented for lists." );  // TODO
        } else {
            PyObject * typeRepr = PyObject_Repr( pyType_ );
            emraise( badParameter, "Unknown type provided: %s.",
                            PyString_AsString( typeRepr ) );
        }
    } else if( PyType_IsSubtype( pyType, &PyBool_Type ) ) {
        _M_insert_parameter( bool );
    } else if( PyType_IsSubtype( pyType, &PyString_Type ) ) {
        _M_insert_parameter_string( std::string )
    } else if( PyType_IsSubtype( pyType, &PyInt_Type ) ) {
        _M_insert_parameter( long );
    } else if( PyType_IsSubtype( pyType, &PyFloat_Type ) ) {
        _M_insert_parameter( double );
    } else if( PyString_Check(pyType_) ) {
        emraise( unimplemented, "Dynamic type resolution and parameter "
                "insertion is not yet implemented." );  // TODO
    } else {
        PyObject * typeRepr = PyObject_Repr( pyType_ );
        emraise( badParameter, "Unknown type provided: %s.",
                        PyString_AsString( typeRepr ) );
    }
    // ...
}
%}

// vim: ft=swig

