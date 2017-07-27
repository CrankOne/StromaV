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
%include "std_list.i"

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

%runtime %{
#include "goo_dict/dict.hpp"
#include "goo_types.h"

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

PyObject *
iSingularParameter2PyObject( goo::dict::iSingularParameter * isp );

%}

%pythoncode %{
import re  # need for Dictionary.__getattr__
%}

%feature("shadow") goo::dict::InsertionProxy::p( PyObject *, PyObject * ) %{
def p(self, *args, **kwargs):
    return _gooDict.InsertionProxy_p(self, args, kwargs)
%}

%feature("shadow") goo::dict::Dictionary::parameters() const %{
def parameters( self ):
    return self.__list_parameters()
%}

%feature("shadow") goo::dict::Dictionary::__getattr__( PyObject * pyStrKey ) %{
def __getattr__(self, pyStrKey):
    # Note: if no matches found, keep default behaviour to raise original error.
    if '_' in pyStrKey:
        rxsLookup = pyStrKey.replace( "_", "[-_]" )
        rxeLookup = re.compile( rxsLookup )
        candidates = list(filter( rxeLookup.match, self.parameters() ))
        if len(candidates) > 1:
            raise KeyError( "Disambiguation. The \"%s\" identifier may "
                "refer to: %s. Consider manual getattr() invokation."%(
                pyStrKey, str(candidates) ) )
        elif 1 == len(candidates):
            pyStrKey = candidates[0]
    return $action(self, pyStrKey)
%}

%include "goo_dict/parameter.tcc"
%include "goo_dict/insertion_proxy.tcc"
%include "goo_dict/dict.hpp"

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

%extend goo::dict::Dictionary {
    PyObject * __getattr__( PyObject * pyStrKey ) {
        if( !PyString_Check( pyStrKey ) ) {
            PyObject * typeRepr = PyObject_Repr( pyStrKey );
            emraise( badParameter, "Goo's dictionary __getattr__ called "
                "with not a string type argument: %s.", PyString_AsString( typeRepr ) );
        }
        const char * attrKey = PyString_AS_STRING(pyStrKey);
        goo::dict::iSingularParameter * parameter = $self->probe_parameter( attrKey );
        if( !parameter ) {
            char bf[128];
            snprintf( bf, sizeof(bf),
                "Entry \"%s\" not found in parameter dictionary %p.",
                attrKey, $self );
            PyErr_SetString( PyExc_KeyError, bf );
            return NULL;
        }
        return iSingularParameter2PyObject( parameter );
    }

    PyObject * __list_parameters() const {
        auto pLst = $self->parameters();
        PyObject * pyPLst = PyTuple_New( pLst.size() );
        uint32_t n = 0;
        for( auto it = pLst.begin(); pLst.end() != it; ++it, ++n ) {
            PyObject * pName;
            if( (*it)->name() ) {
                pName = PyString_FromString( (*it)->name() );
            } else {
                pName = PyString_FromFormat( "%c", (*it)->shortcut() );
            }
            PyTuple_SET_ITEM( pyPLst, n, pName );
        }
        return pyPLst;
    }
}

%{

#include "sV_config.h"

#if !defined( PYTHON_BINDINGS )
#error "PYTHON_BINDINGS is not defined. Unable to build goo's dicts py-wrapper module."
#endif


//
// Parameter insertion helper macros and routine
///////////////////////////////////////////////

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

PyObject *
iSingularParameter2PyObject( goo::dict::iSingularParameter * isp ) {
    // Parameter found. Now resolve its type and return as python object.
    const std::type_info & TI = isp->target_type_info();
    # ifndef GOO_TYPES_H
    # error Macros from goo_types.h header are required.
    # endif

    # define _M_return_pyInt( gooCode, cType, gooType, gooSType )       \
    if( typeid(cType) == TI ) {                                         \
        return PyInt_FromLong( (long) isp->as<cType>() );               \
    } else

    # define _M_return_pyFloat( gooCode, cType, gooType, goosType )     \
    if( typeid(cType) == TI ) {                                         \
        return PyFloat_FromDouble( (long double) isp->as<cType>() );    \
    } else

    for_all_integer_datatypes( _M_return_pyInt )
    for_all_floating_point_datatypes( _M_return_pyFloat )
    if( typeid(std::string) == TI ) {
        return PyString_FromString( isp->as<std::string>().c_str() );
    //} else if(  ) {
    } else {
        char bf[128];
        snprintf( bf, sizeof(bf),
            "Requested Goo dictionary entry has "
            "type \"%s\" which can not be converted into native Python type.",
            TI.name() );
        PyErr_SetString( PyExc_TypeError, bf );
        return NULL;
    }
}

%}

// vim: ft=swig

