%module extParameters

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

%import "std_string.i"
%include "_gooExceptionWrapper.i"

%import(module="StromaV.appUtils") "appUtils.i"
//%import(module="StromaV.gooVCopy") "gooVCopy.i"
%include "gooVCopy.i"

/* SWIG of versions at least >=2.0.9 doesn't like the C++11 override/final
 * keywords, so we get rid of them using these macro defs: */
# ifdef SWIG
# define override
# define final
# endif  // SWIG

%import "sV_config.h"

%{
#include "sV_config.h"

#if !defined( RPC_PROTOCOLS )
#error "RPC_PROTOCOLS is not " \
"defined. Unable to build pipeline py-wrapper module."
#endif

#include "analysis/pipeline.hpp"
#if !defined( RPC_PROTOCOLS ) || !defined( ANALYSIS_ROUTINES )
#error "Either RPC_PROTOCOLS or ANALYSIS_ROUTINES not defined. Unable to " \
"build analysis module."
#endif

#include "goo_dict/parameters/path_parameter.hpp"
#include "goo_dict/parameter.tcc"

#include "goo_path.hpp"
#include "app/cvalidators.hpp"
# ifdef GEANT4_MC_MODEL
#   include <G4ThreeVector.hh>
# endif  // GEANT4_MC_MODEL

%}

%nodefaultctor goo::mixins::iDuplicable<
            goo::dict::iAbstractParameter,
            goo::dict::Parameter< goo::filesystem::Path >,
            goo::dict::iParameter< goo::filesystem::Path > >;
%nodefaultctor goo::mixins::iDuplicable<
            goo::dict::iAbstractParameter,
            goo::dict::iAbstractParameter,
            goo::dict::iAbstractParameter>;

//%feature("flatnested") Path::PathInterpolator;?

//
// This macro designed to automate wrapping of the custom parameters wrapping.
%define sV_M_wrap_parameter_type(
            typeName, nsTypeName,
            typeHeader, parameterHeader )
%extend nsTypeName {
    void _assign_to_parameter(goo::dict::iSingularParameter * ispPtr) const {
        auto pPtr = dynamic_cast<goo::dict::Parameter< nsTypeName >*>(ispPtr);
        if(!pPtr) {
            emraise( badCast, "Type mismatch. Unable to assign parameter value." );
        }
        pPtr->set_value( *$self );
    }
    typeName ( goo::dict::iSingularParameter * ispPtr ) {
        auto pPtr = dynamic_cast<goo::dict::Parameter< nsTypeName >*>(ispPtr);
        if( !pPtr ) {
            emraise( badCast, "Type mismatch. Unable to extract parameter value." );
        }
        return new nsTypeName( pPtr->as<nsTypeName>() );
    }
}
%include typeHeader
%include "goo_dict/parameter.tcc"
%template(_PType_ ## typeName ## _IFace) goo::dict::iParameter< nsTypeName >;
gooVCopy_shim_consumer( PType_ ## typeName
        , goo::dict::iAbstractParameter
        , goo::dict::Parameter< nsTypeName >
        , goo::dict::iParameter< nsTypeName > )
%include parameterHeader
%template(PType_ ## typeName) goo::dict::Parameter< nsTypeName>;
%enddef

//
// How this macro can be applied for C++-defined types:
sV_M_wrap_parameter_type(
        Path, goo::filesystem::Path,
        "goo_path.hpp", "goo_dict/parameters/path_parameter.hpp" )
sV_M_wrap_parameter_type(
        HistogramParameters1D, sV::aux::HistogramParameters1D,
        "app/cvalidators.hpp", "app/cvalidators.hpp")
sV_M_wrap_parameter_type(
        HistogramParameters2D, sV::aux::HistogramParameters2D,
        "app/cvalidators.hpp", "app/cvalidators.hpp")

# ifdef GEANT4_MC_MODEL
// Welp, some types will still require customized wrapper description. Note the
// %rename() instruction followed by the original/alias headers here.
%extend CLHEP::Hep3Vector {
    void _assign_to_parameter(goo::dict::iSingularParameter * ispPtr) const {
        auto pPtr = dynamic_cast<goo::dict::Parameter< CLHEP::Hep3Vector >*>(ispPtr);
        if(!pPtr) {
            emraise( badCast, "Type mismatch. Unable to assign parameter value." );
        }
        pPtr->set_value( *$self );
    }
    Hep3Vector ( goo::dict::iSingularParameter * ispPtr ) {
        auto pPtr = dynamic_cast<goo::dict::Parameter< CLHEP::Hep3Vector >*>(ispPtr);
        if( !pPtr ) {
            emraise( badCast, "Type mismatch. Unable to extract parameter value." );
        }
        return new G4ThreeVector( pPtr->as<G4ThreeVector>() );
    }
}
%rename(G4ThreeVector) CLHEP::Hep3Vector;   // < Here goes
%include "CLHEP/Vector/ThreeVector.h"       // < the custom
%include "G4ThreeVector.hh"                 // < block.
%template(_PType_G4ThreeVector_IFace) goo::dict::iParameter< G4ThreeVector >;
gooVCopy_shim_consumer( PType_G4ThreeVector
        , goo::dict::iAbstractParameter
        , goo::dict::Parameter< G4ThreeVector >
        , goo::dict::iParameter< G4ThreeVector > )
%template(PType_G4ThreeVector) goo::dict::Parameter<G4ThreeVector>;
# else
#   warning "The G4ThreeVector wrapper will not be generated."
# endif   //GEANT4_MC_MODEL

// vim: ft=swig

