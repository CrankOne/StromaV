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

%include "_gooExceptionWrapper.i"
%include "_sV_utility.i"

%nodefaultctor goo::dict::InsertionProxy;

/* SWIG of versions at least >=2.0.9 doesn't like the C++11 override/final
 * keywords, so we get rid of them using these macro defs: */
# ifdef SWIG
# define override
# define final
# endif  // SWIG

%import "extParameters.i"

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
// Template value check helpers for simple type XXX?

template<typename PyT> bool
py_value_type_check( PyObject * );  // None default implementation

template<> bool
py_value_type_check<bool>( PyObject * o ) {
    return PyBool_Check(o);
}

template<> bool
py_value_type_check<long>( PyObject * o ) {
    return PyInt_Check(o);
}

template<> bool
py_value_type_check<double>( PyObject * o ) {
    return PyFloat_Check(o);
}

template<> bool
py_value_type_check<std::string>( PyObject * o ) {
    return PyString_Check(o);
}

# define for_each_py_conversion_type( m, ... )  \
    m( bool         , ## __VA_ARGS__ )          \
    m( long         , ## __VA_ARGS__ )          \
    m( double       , ## __VA_ARGS__ )          \
    m( std::string  , ## __VA_ARGS__ )

//
// Template value acquizition helpers for simple type

template<typename T> T
get_C_value(PyObject *);  // None default implementation

# if 1
// Seems that Python somehow performs implicit conversion of types, similar to
// defined in #else-block, so there is apparently no need in manual conversion.
# define _M_get_C_value_implem(T, fun)                           \
template<> T get_C_value<T>(PyObject * o) { return fun (o); }
_M_get_C_value_implem( bool, PyObject_IsTrue ) // Note: implemented as subclass of integers
_M_get_C_value_implem( long, PyInt_AsLong )
_M_get_C_value_implem( double, PyFloat_AsDouble )
_M_get_C_value_implem( std::string, PyString_AsString )
# undef _M_get_C_value_implem
# else
template<> bool
get_C_value<bool>(PyObject * o) {
    return PyObject_IsTrue( o );
}

template<> long
get_C_value<long>(PyObject * o) {
    if( PyInt_Check(o) ) {
        return PyInt_AsLong( o );
    }
    if( PyFloat_Check(o) ) {
        // implicit conversion from double to long
        return (long) PyFloat_AsDouble(o);
    }
    PyObject * repr = PyObject_Repr( o );
    emraise( badParameter, "Numeric (integer) value expected insted of \"%s\".",
            PyString_AsString( repr ) );
}

template<> double
get_C_value<double>(PyObject * o) {
    if( PyFloat_Check(o) ) {
        return PyFloat_AsDouble(o);
    }
    if( PyInt_Check(o) ) {
        // implicit conversion from long to double
        return (double) PyInt_AsLong( o );
    }
    PyObject * repr = PyObject_Repr( o );
    emraise( badParameter, "Numeric (floating point) value expected insted of \"%s\".",
            PyString_AsString( repr ) );
}

template<> std::string
get_C_value<std::string>(PyObject * o) {
    if( ! PyString_Check(o) ) {
        PyObject * repr = PyObject_Repr( o );
        emraise( badParameter, "String value expected instead of \"%s\".",
            PyString_AsString( repr ) );
    }
    return PyString_AsString( o );
}
# endif

//
// Generic template value creation helpers

template<typename T> PyObject *
new_Py_value( const T & );  // None default implementation

# define _M_new_Py_value_implem( code, T, gooType, gooSType, fun, pyCType ) \
template<> PyObject * new_Py_value<T>( const T & val ) { return fun( (pyCType) val ); }
for_all_floating_point_datatypes( _M_new_Py_value_implem, PyFloat_FromDouble, double )
for_all_integer_datatypes( _M_new_Py_value_implem, PyInt_FromLong, long )
# undef _M_new_Py_value_implem

template<> PyObject *
new_Py_value<std::string>( const std::string & val ) { 
    return PyString_FromString( val.c_str() );
}

template<> PyObject *
new_Py_value<bool>( const bool & val ) { 
    return PyBool_FromLong( val ? 1 : 0 );
}


// ...

// WARNING: no PyTuple_Check() inside!
template<typename T> std::list<T>
get_C_list( PyObject * pyTuple ) {
    std::list<T> lst;
    for( ssize_t i = 0; i < PyTuple_Size(pyTuple); ++i ) {
        PyObject * o = PyTuple_GET_ITEM( pyTuple, i );
        lst.push_back( get_C_value<T>( o ) );
    }
    return lst;
}

static PyObject *
iSingularParameter2PyObject( goo::dict::iSingularParameter * isp );

static int
iSingularParameterSetFromPyObject(
            goo::dict::iSingularParameter * isp,
            PyObject * pyValue );

// ...

template<typename EntryT, typename ValueT> void
set_parameter( goo::dict::iSingularParameter * isp, PyObject * pyVal ) {
    goo::dict::iParameter<EntryT> * pPtr
                        = dynamic_cast<goo::dict::iParameter<EntryT>*>(isp);
    if( !pPtr ) {
        // Should not happen if I there is no mistake with type check. But it
        // is still safer to check this.
        emraise(badCast, "Internal error: unable to cast parameter declared "
            "with target type \"%s\" to iParameter<> parameterised with "
            "type \"%s\".",
            isp->target_type_info().name(), typeid(EntryT).name() );
    }
    pPtr->set_value( (EntryT) get_C_value<ValueT>(pyVal) );
}

// ...

// WARNING: no PyTuple_Check() inside!
template<typename T, typename pyT> void
set_list_parameter( goo::dict::iSingularParameter * isp, PyObject * pyVal ) {
    std::list<pyT> lst_ = get_C_list<pyT>( pyVal );
    std::list<T> lst;
    for( auto v : lst_ ) {
        lst.push_back( (T) v);   // actual conversion here
    }
    goo::dict::Parameter<std::list<T>> * pPtr
                    = dynamic_cast<goo::dict::Parameter<std::list<T>> *>(isp);
    if( !pPtr ) {
        // Same note as in set_parameter()
        emraise(badCast, "Internal error: unable to cast parameter declared "
            "with target type \"%s\" to iParameter<list<\"%s\">>.",
            isp->target_type_info().name(), typeid(T).name() );
    }
    pPtr->assign( lst );
}

%}  // %runtime

%newobject goo::dict::Dictionary::insertion_proxy;
// We do not need this method in python since we won't work with parameter
// instances.
%ignore goo::dict::InsertionProxy::insert_copy_of;
%ignore goo::dict::InsertionProxy::flag;

self_returning_method( goo::dict::InsertionProxy & goo::dict::InsertionProxy::p)
self_returning_method( goo::dict::InsertionProxy & goo::dict::InsertionProxy::bgn_sect )
self_returning_method( goo::dict::InsertionProxy & goo::dict::InsertionProxy::end_sect )

%pythoncode %{
import re, inspect

def _nearest_name( name, namesList ):
    """
    This function searches the given string among given ones following the
    substitution rule: the underscore symbol ('_') may correspond to
    hyphen-minus symbol ('-', dash) as well. This was introduced to support
    dash characters in goo::dict names.
    If no matches found, the original name will be ketp to raise original
    error in user code.
    Returns found name (or original, if it was not found) and bool value
    indicating whether the name (original or found one) was found in list.
    """
    if '_' in name:
        rxsLookup = '^' + name.replace( "_", "[-_]" ) + '$'
        rxeLookup = re.compile( rxsLookup )
        candidates = list(filter( rxeLookup.match, namesList ))
        if len(candidates) > 1:
            raise KeyError( "Disambiguation. The \"%s\" identifier may "
                "refer to parameters named: %s. Consider manual getattr() "
                "invokation."%(
                name, str(candidates) ) )
        elif 1 == len(candidates):
            name = candidates[0]
    return name, name in namesList
%}

%feature("shadow") goo::dict::InsertionProxy::p( PyObject *, PyObject * ) %{
def p(self, *args, **kwargs):
    if 1 == len(args) and args[0] is bool:
        if 'default' not in kwargs.keys():
            kwargs = dict(kwargs)
            kwargs['default'] = False
    elif inspect.isclass(args[0]) and issubclass( args[0], extParameters.iSingularParameter ):
        parameterInstance = args[0]( *(args[:1]), **kwargs )
        return self
    return _gooDict.InsertionProxy_p(self, args, kwargs)
%}

// We do not need this method being native in python since we won't work with
// parameter instances. It just returns the tuple of names instead.
%feature("shadow") goo::dict::Dictionary::parameters() const %{
def parameters( self ):
    return self.__list_parameters()
%}

// The wrapping of this method natively has no sense considering significant
// cost of wrapping the std::map<> and copying sub-dictionaries. It just
// returns the tuple of names instead.
%feature("shadow") goo::dict::Dictionary::dictionaries() const %{
def dictionaries( self ):
    return self.__list_subsections()
%}

%feature("shadow") goo::dict::Dictionary::__getattr__( PyObject * pyStrKey ) %{
def __getattr__(self, pyStrKey):
    pyStrKey, found = _nearest_name(pyStrKey, self.parameters() + self.dictionaries())
    if pyStrKey in self.dictionaries():
        return self.subsection( pyStrKey )
    return $action(self, pyStrKey)
%}

%feature("shadow") goo::dict::Dictionary::__setattr__( PyObject * pyStrKey, PyObject * pyValue ) %{
def __setattr__(self, pyStrKey, pyVal):
    if 'this' == pyStrKey:
        _swig_setattr(self, self.__class__, pyStrKey, pyVal)
        return
    eName, isEntry = _nearest_name(pyStrKey, self.parameters() + self.dictionaries())
    if isEntry:
        $action(self, eName, pyVal)
    else:
        _swig_setattr(self, self.__class__, pyStrKey, pyVal)
%}

%include "goo_dict/insertion_proxy.tcc"
%include "goo_dict/dict.hpp"

%extend goo::dict::InsertionProxy {
    goo::dict::InsertionProxy & p( PyObject * args, PyObject * kwargs ) {
        // Actual signature of the function for insertion of parameter of simple
        // type:
        //      ( type, name=None, description=None, shortcut=None, required=False )
        // For complex types the expected form may vary, but has to consist
        // of wrapped type instance going first. The rest arguments will be
        // forwarded to its constructor:
        //      ( complexType, args ... )
        // Within the C API the args is an ordinary Python tuple, and kwargs is
        // the usual dictionary.

        // - check args:
        if( !PyTuple_Check(args) || 1 != PyTuple_Size(args) ) {
            emraise( badParameter, "Python native type (str, float, int or "
                "bool) or tuple of size 1 carrying type expected as "
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

        if( required ) {
            // Mark last inserted parameter as required.
            $self->required_argument();
        }

        return *$self;
    }
}

%extend goo::dict::Dictionary {
    PyObject * __getattr__( PyObject * pyStrKey ) {
        if( !PyString_Check( pyStrKey ) ) {
            PyObject * typeRepr = PyObject_Repr( pyStrKey );
            emraise( badParameter, "Goo's dictionary __getattr__ called "
                "with not a string type argument: %s.",
                PyString_AsString( typeRepr ) );
        }
        const char * attrKey = PyString_AS_STRING(pyStrKey);
        goo::dict::iSingularParameter * parameter = $self->probe_parameter( attrKey );
        goo::dict::Dictionary * sub = $self->probe_subsection( attrKey );

        if( !parameter && !sub ) {
            char bf[128];
            snprintf( bf, sizeof(bf),
                "Entry \"%s\" not found in parameters dictionary \"%s\" (%p).",
                attrKey, $self->name(), $self );
            PyErr_SetString( PyExc_KeyError, bf );
            return NULL;
        } else if ( parameter ) {
            return iSingularParameter2PyObject( parameter );
        } else {
            // This case is possible when user tries to trick our
            // overriden "__getattr__()" method and somehow got access to
            // native underlying SWIG lambda function or similar...
            emraise( badState, "The dictionary \"%s\" (%p) is declared as "
                "a subsection of dictionary \"%s\" (%p) while it was "
                "requested as a parameter. Consider use of subsection() "
                "method.", sub->name(), sub,
                $self->name(), $self );
        }
    }

    int __setattr__( PyObject * pyStrKey, PyObject * pyValue ) {
        /* the C setattr function should return an integer, 0 on success, -1
         * when an exception is raised.  When the given value is NULL, then
         * "delattr" was called instead of "setattr". */
        if( !PyString_Check( pyStrKey ) ) {
            PyObject * repr = PyObject_Repr( pyStrKey );
            emraise( badParameter, "Goo's dictionary __setattr__ called "
                "with not a string type argument: %s.",
                PyString_AsString( repr ) );
        }
        const char * attrKey = PyString_AS_STRING(pyStrKey);
        if( !pyValue ) {
            emraise( badArchitect, "Goo's dictionary doesn't support "
                "attribute deletion (deletion of attribute \"%s\" requested).",
                PyString_AsString( pyStrKey ) );
        }
        goo::dict::iSingularParameter * parameter = $self->probe_parameter( attrKey );
        goo::dict::Dictionary * sub = $self->probe_subsection( attrKey );

        if( !parameter && !sub ) {
            char bf[128];
            snprintf( bf, sizeof(bf),
                "Entry \"%s\" not found in parameters dictionary \"%s\" (%p).",
                attrKey, $self->name(), $self );
            PyErr_SetString( PyExc_KeyError, bf );
            return -1;
        } else if ( parameter ) {
            return iSingularParameterSetFromPyObject( parameter, pyValue );
        } else {
            // This case is possible when user tries to trick our
            // overriden "__setattr__()" method and somehow got access to
            // native underlying SWIG lambda function or similar...
            emraise( badState, "The dictionary \"%s\" (%p) is declared as "
                "a subsection of dictionary \"%s\" (%p) while it was "
                "requested as a (scalar or tuple) parameter to be setted.",
                sub->name(), sub, $self->name(), $self );
        }
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

    PyObject * __list_subsections() const {
        const auto & subsMap = $self->dictionaries();
        PyObject * pyPLst = PyTuple_New( subsMap.size() );
        uint32_t n = 0;
        for( auto it = subsMap.begin(); subsMap.end() != it; ++it, ++n ) {
            PyTuple_SET_ITEM(
                pyPLst, n, PyString_FromString( it->first.c_str() ) );
        }
        return pyPLst;
    }
}

%extend goo::dict::Dictionary {
    PyObject * set_from_str( PyObject * pyStrKey, PyObject * pyValue ) {
        if( !PyString_Check( pyStrKey ) ) {
            PyObject * repr = PyObject_Repr( pyStrKey );
            emraise( badParameter, "Goo's dictionary set_from_str() called "
                "with not a string type argument: %s.",
                PyString_AsString( repr ) );
        }
        const char * attrKey = PyString_AS_STRING(pyStrKey);
        goo::dict::iSingularParameter * parameter = $self->probe_parameter( attrKey );
        if( !parameter ) {
            char bf[128];
            snprintf( bf, sizeof(bf),
                "Entry \"%s\" not found in parameters dictionary \"%s\" (%p).",
                attrKey, $self->name(), $self );
            PyErr_SetString( PyExc_KeyError, bf );
            return NULL;
        }
        if( !PyString_Check( pyValue ) ) {
            PyObject * repr = PyObject_Repr( pyStrKey );
            emraise( badParameter, "String expected as a second argument. Got \"%s\".",
                    PyString_AsString( repr ) );
        }
        parameter->parse_argument( PyString_AS_STRING( pyValue ) );
        return iSingularParameter2PyObject( parameter );
    }
}

%extend goo::dict::Dictionary {
    PyObject * strval_of( PyObject * pyStrKey ) {
        if( !PyString_Check( pyStrKey ) ) {
            PyObject * repr = PyObject_Repr( pyStrKey );
            emraise( badParameter, "Goo's dictionary strval_of() called "
                "with not a string type argument: %s.",
                PyString_AsString( repr ) );
        }
        const char * attrKey = PyString_AS_STRING(pyStrKey);
        goo::dict::iSingularParameter * parameter = $self->probe_parameter( attrKey );
        if( !parameter ) {
            char bf[128];
            snprintf( bf, sizeof(bf),
                "Entry \"%s\" not found in parameters dictionary \"%s\" (%p).",
                attrKey, $self->name(), $self );
            PyErr_SetString( PyExc_KeyError, bf );
            return NULL;
        }
        return PyString_FromString(parameter->to_string().c_str());
    }
}

%{

#include "sV_config.h"

#if !defined( PYTHON_BINDINGS )
#error "PYTHON_BINDINGS is not defined. Unable to build goo's dicts py-wrapper module."
#endif


//
// Parameter insertion helper macros and routine {{{

# define _M_insert_parameter_generic( parType )                     \
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
    
# define _M_insert_parameter_list_strings( M )                      \
    if( pyDefault_ ) {                                              \
        if( shortcut && name ) {                                    \
            M( std::string, get_C_list<std::string>(pyDefault_), shortcut, name, description );  \
        } else if( (!shortcut) && name ) {                          \
            M( std::string, get_C_list<std::string>(pyDefault_), name, description );   \
        } else if( shortcut && !name ) {                            \
            M( std::string, get_C_list<std::string>(pyDefault_), shortcut, nullptr, description );  \
        }                                                           \
    } else {                                                        \
        if( shortcut && name ) {                                    \
            M( std::string, shortcut, name, description );          \
        } else if( (!shortcut) && name ) {                          \
            M( std::string, name, description );                    \
        } else if( shortcut && !name ) {                            \
            M( std::string, shortcut, nullptr, description );       \
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
        if( pyDefault_ ) {
            if( !PyTuple_Check(pyDefault_) ) {
                PyObject * typeRepr = PyObject_Repr( pyType_ );
                emraise( badParameter,
                    "Tuple expected as 'default' argument. Got %s instead.",
                    PyString_AsString( typeRepr ) );
            }
        }
        // Tuple objects:
        if( PyType_IsSubtype( pyType, &PyBool_Type ) ) {
            _M_insert_parameter_list( bool, _M_insert_list_p )
        } else if( PyType_IsSubtype( pyType, &PyString_Type ) ) {
            _M_insert_parameter_list_strings( _M_insert_list_p )
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
    // todo: shall we check PyErr_Occurred() here in case something went
    // wrong with subsequent python invokations?
    // See reference at:
    // https://docs.python.org/2/c-api/exceptions.html#c.PyErr_Occurred
}

# undef _M_insert_parameter_generic
# undef _M_insert_parameter
# undef _M_insert_parameter_string
# undef _M_insert_list_p
# undef _M_insert_parameter_list
# undef _M_insert_parameter_list_strings

// }}} Parameter insertion helper macros and routine
//


static PyObject *
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

    if( isp->has_multiple_values() ) {
        // Parameter is a tuple.
        # define _M_return_pyTuple( T )                                 \
        if( typeid(T) == TI ) {                                         \
            const std::list<T> & lst = isp->as_list_of<T>();            \
            PyObject * pyLst = PyTuple_New( lst.size() );               \
            uint32_t n = 0;                                             \
            for( auto & pRef : lst ) {                                  \
                PyTuple_SET_ITEM( pyLst, n++, new_Py_value<T>( pRef ) );  \
            }                                                           \
            return pyLst;                                               \
        } else
        # define _M_goo_return_tuple_types_wrap( gooCode, cType, gooType, goosType ) \
        _M_return_pyTuple( cType )
        for_all_integer_datatypes( _M_goo_return_tuple_types_wrap )
        for_all_floating_point_datatypes( _M_goo_return_tuple_types_wrap )
        _M_return_pyTuple(bool)
        _M_return_pyTuple(std::string)
        # undef _M_goo_return_tuple_types_wrap
        # undef _M_return_pyTuple
        {
            // Do nothing, forward execution to finalizing block that raises
            // an exception.
        }
    } else
    for_all_integer_datatypes( _M_return_pyInt )
    for_all_floating_point_datatypes( _M_return_pyFloat )
    if( typeid(std::string) == TI ) {
        return PyString_FromString( isp->as<std::string>().c_str() );
    } else if( typeid(bool) == TI ) {
        return PyBool_FromLong( isp->as<bool>() ? 1 : 0 );
    }
    // All the checks have failed.
    char bf[128];
    snprintf( bf, sizeof(bf),
        "Requested Goo dictionary entry has "
        "type \"%s\" which can not be converted into native Python type.",
        TI.name() );
    PyErr_SetString( PyExc_TypeError, bf );
    return NULL;
    # undef _M_return_pyInt
    # undef _M_return_pyFloat
}

static int
iSingularParameterSetFromPyObject(
            goo::dict::iSingularParameter * isp,
            PyObject * pyValue ) {
    char shrtctBf[2] = " ";
    const char * name = shrtctBf;
    if( isp->name() ) {
        name = isp->name();
    } else {
        shrtctBf[0] = isp->shortcut();
    }

    // NOTE: the float and int variables shall be implicitly convertible!
    const std::type_info & TI = isp->target_type_info();
    if( PyTuple_Check( pyValue ) && isp->has_multiple_values() ) {
        # define _M_set_list_parameter( gooCode, cType, t1, t2, pyCT )      \
        if( typeid(cType) == TI ) {                                         \
            set_list_parameter<cType, pyCT>(isp, pyValue); return 0;        \
        }
        for_all_integer_datatypes( _M_set_list_parameter, long )
        for_all_floating_point_datatypes( _M_set_list_parameter, double )
        _M_set_list_parameter( xxx, bool, xxx, xxx, bool )
        _M_set_list_parameter( xxx, std::string, xxx, xxx, std::string )
        # undef _M_set_list_parameter
        // ... custom types?
    } else if(PyBool_Check( pyValue )) {
        if( typeid(bool) == TI ) { set_parameter<bool, bool>( isp, pyValue ); return 0; }
    } else if( PyInt_Check( pyValue ) || PyFloat_Check( pyValue )) {
        # define _M_set_int_parameter( gooCode, cType, t1, t2 ) \
        if( typeid(cType) == TI ) { set_parameter<cType, long>(isp, pyValue); return 0; }
        for_all_integer_datatypes( _M_set_int_parameter )
        # undef _M_set_int_parameter
        # define _M_set_float_parameter( gooCode, cType, t1, t2 ) \
        if( typeid(cType) == TI ) { set_parameter<cType, double>(isp, pyValue); return 0; }
        for_all_floating_point_datatypes( _M_set_float_parameter )
        # undef _M_set_float_parameter
    } else if( PyString_Check( pyValue ) ) {
        // The string value may be implicitly parsed for non-string types.
        // Here, however we have to check the parameter type first, and if it
        // is a std::string parameter we will just set the string. Otherwise,
        // when it is not a plain string parameter, we have raise an exception
        // instead of implicit parsing. This was considered a better style
        // after discussion.
        if( typeid(std::string) == TI ) { 
            set_parameter<std::string, std::string>( isp, pyValue );
            return 0;
        } else {
            //isp->parse_argument( PyString_AsString(pyValue) );
            PyObject * valRepr = PyObject_Repr( pyValue );
            emraise( badState, "Given Python value \"%s\" can not be implicitly "
                    "set as a value of a parameter \"%s\" of type \"%s\". If "
                    "you intended parsing from string, consider usage of method "
                    "Dictionary.set_from_str instead of assignment operation.",
                    PyString_AsString( valRepr ),
                    name, TI.name() );
            return 0;
        }
    }
    // We have to also consider a special case when user sets the boolean
    // value from python object. It has to be generally legit.
    if( typeid(bool) == TI ) {
        auto & pRef = *static_cast<goo::dict::iParameter<bool>*>( isp );
        pRef.set_value( PyObject_IsTrue(pyValue) );
        return 0;
    }
    PyObject * valRepr = PyObject_Repr( pyValue );
    emraise( badParameter, "Given Python value \"%s\" can not be "
        "set as a value of a parameter \"%s\" of type \"%s\".",
        PyString_AsString( valRepr ),
        name, TI.name() );
    return -1;
}

%}

// vim: ft=swig

