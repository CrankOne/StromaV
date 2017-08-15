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

%ignore google::protobuf::Any::operator=;

%include "google/protobuf/stubs/common.h"

/* NOTE: not sure, whether the 3.1 is exact version number. At least for 3.1
 * the instructions below have to be disabled, while version 3.3 makes them
 * necessary.
 */
# if GOOGLE_PROTOBUF_VERSION > 3001000
/* Generating wrapper to this function yields a weird issue in recent versions
 * of protobufs. The mangled symbol is, like:
 * _ZN6google8protobuf8internal27MergePartialFromCodedStreamEPNS0_11MessageLiteERKNS1_10ParseTableEPNS0_2io16CodedInputStreamE
 * will appear in compiled binaries causing the symbol resolution failure since
 * it isn't defined at any of the system libraries. Since we're not planning to
 * actually use partial encoded stream we do omit this intenral function. */
%ignore google::protobuf::internal::MergePartialFromCodedStream;
%ignore google::protobuf::internal::TypeImplementsMergeBehaviorProbeForMergeFrom;
%include "google/protobuf/generated_message_util.h"
%ignore google::protobuf::internal::AuxillaryParseTableField::enum_aux;
%ignore google::protobuf::internal::AuxillaryParseTableField::message_aux;
%ignore google::protobuf::internal::AuxillaryParseTableField::string_aux;
%include "google/protobuf/generated_message_table_driven.h"
# endif  // GOOGLE_PROTOBUF_VERSION > 3001000

%include "google/protobuf/message_lite.h"
%include "google/protobuf/message.h"

//See issue #173
//# define GOOGLE_ATTRIBUTE_ALWAYS_INLINE
//# define GOOGLE_ATTRIBUTE_NOINLINE
//%include "google/protobuf/repeated_field.h"

// Many default instances generated by protobuf are exposed within `extern'
// declarations inside header files generated by protobuf starting (at lest)
// from 3.3. They are not ever supposed to be mutable, so this line is to
// prevent SWIG from generating setter around them (their classes are
// internal).
%immutable "google::protobuf::*_default_instance_";

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
# if GOOGLE_PROTOBUF_VERSION > 3001000
using google::protobuf::ProtobufOnceType;
# endif  // GOOGLE_PROTOBUF_VERSION > 3001000

typedef int32_t int32;
typedef uint32_t uint32;
typedef uint8_t uint8;
typedef int64_t int64;
typedef uint64_t uint64;

using std::string;

%}

%ignore google::protobuf::internal::AnyMetadata::AnyMetadata;
%nodefaultctor google::protobuf::internal::AnyMetadata;
%include "google/protobuf/any.h"

%ignore google::protobuf::protobuf_google_2fprotobuf_2fany_2eproto::TableStruct;
%ignore google::protobuf::protobuf_google_2fprotobuf_2fany_2eproto::AddDescriptors;
%ignore google::protobuf::protobuf_google_2fprotobuf_2fany_2eproto::InitDefaults;
%include "google/protobuf/any.pb.h"

// vim: ft=swig
