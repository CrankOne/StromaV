%module(directors="1") appUtils

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

%import pipeline.i

%include "std_string.i"
%include "std_list.i"
#define PACKAGE
%ignore PACKAGE_VERSION;
%ignore GIT_STRING;
%rename(instance) goo::aux::iApp::self();

%ignore goo::aux::iApp::HandlerEntry;  // nested class
%ignore sV::AbstractApplication::ConfigPathInterpolator;  // nested class
%ignore sV::AbstractApplication::ConstructableConfMapping;  // nested class
%rename(instance) sV::AbstractApplication::ConstructableConfMapping::self;  // 

%include "_gooExceptionWrapper.i"

%nodefaultctor sV::AbstractApplication;
%nodefaultctor sV::mixins::PBEventApp;

/* SWIG of versions at least >=2.0.9 doesn't like the C++11 override/final
 * keywords, so we get rid of them using these macro defs: */
# ifdef SWIG
# define override
# define final
# endif  // SWIG

%include "goo_config.h"
%include "sV_config.h"

%include "goo_app.hpp"
%template(BaseApp) goo::App<goo::dict::Configuration, std::ostream>;

%include "goo_vcopy.tcc"

%include "app/abstract.hpp"
%include "app/mixins/protobuf.hpp"
%include "app/analysis.hpp"
%include "app/py_session.hpp"

%init %{
sV::PythonSession::initialize_exception_type();
%}

%{

#include "sV_config.h"

#if !defined( PYTHON_BINDINGS )
#error "PYTHON_BINDINGS is not defined. Unable to build app py-wrapper module."
#endif

#define SWIG_FILE_WITH_INIT

#include "app/py_session.hpp"
#include "analysis/pipe_fj.hpp"

%}

// vim: ft=swig
