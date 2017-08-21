%module(directors="1") pipeline

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

%ignore PACKAGE_VERSION;

%import "std_string.i"
%import "stdint.i"
%import(module="StromaV.appBase") appBase.i
%include "_gooExceptionWrapper.i"
%include "cpointer.i"

//%import(module="StromaV.sVEvents") "sVEvents.i"

%nodefaultctor std::type_index;

%import "sV_config.h"

%inline %{
#include "uevent.hpp"

void
set_event_ptr(sV::events::Event ** dest, sV::events::Event * eventPtr) {
   *dest = eventPtr;
}

sV::events::Event *
dereference_event_ptr_ref(sV::events::Event ** v) {
   return *v;
}
%}

%feature("director") sV::aux::iEventProcessor;
%feature("director") sV::aux::iEventSequence;

%{

#include "analysis/pipeline.hpp"
#include "analysis/pipe_fj.hpp"
#include "ctrs_dict.hpp"
#include "app/py_session.hpp"

%}

%ignore sV::AnalysisPipeline::Handler; 

%include "app/mixins/protobuf.hpp"
%include "analysis/pipeline.hpp"
%include "analysis/pipe_fj.hpp"

// vim: ft=swig
