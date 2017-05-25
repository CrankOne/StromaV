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

/*
 * A helper SWIG module introducing support for auto-generated SWIG wrapping
 * code around auto-generated protobuf output.
 *
 * The purpose of these wrapping code is to provide 'beanlike' (i.e. with basic
 * setters and getters) access to protobuf message types within Python
 * environment. We did it this way by two reasons:
 *  1. The protobuf native Python implementation is known to be inefficient
 * since highly relies on run-time introspection. New features of protobuf
 * forwarding Python calls to C++ are still under development and can not be
 * supported out-of-the box.
 *  2. We have to deal with protobuf messages from within SWIG's directors and
 * need to "understand" C++ pointer and reference message types.
 */

/* SWIG doesn't include "google/protobuf/stubs/common.h" header where this
 * versioning macro is defined, but including it directly makes SWIG crazy.
 * So here we have to manually defining these macro to pass by. */
#define LIBPROTOBUF_EXPORT

%include "google/protobuf/stubs/common.h"
%include "google/protobuf/message_lite.h"
%include "google/protobuf/message.h"

%{
# include "google/protobuf/message_lite.h"

using google::protobuf::MessageLite;

# include <google/protobuf/message.h>

//
// This using and typedef declarations have to be appended manually to make
// SWIG compile the sources
using google::protobuf::Descriptor;
using google::protobuf::MessageLite;
using google::protobuf::FieldDescriptor;
using google::protobuf::OneofDescriptor;
using google::protobuf::EnumValueDescriptor;

typedef int32_t int32;
typedef uint32_t uint32;
typedef uint8_t uint8;
typedef int64_t int64;
typedef uint64_t uint64;

%}

// vim: ft=swig