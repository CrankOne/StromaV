%module(directors="1") sVEvents

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

%include "std_string.i"
%include "cpointer.i"  // xxx?
%include "_commonProtobuf.i"
%include "_gooExceptionWrapper.i"

/* SWIG concats enum names with their encompassing classes. The same does
 * protoc for some intriguing purpose as a dedicated consts. Yhus making SWIG
 * to process protoc output causes these identifiers to collide. Here we
 * resolve this conflicts by manually renaming the encompassing classes. */
%rename(SenderStatus)           sV::events::MulticastMessage_SenderStatusMessage;
%rename(CompressedDataMsg)      sV::events::CompressedData;

%ignore sV::events::protobuf_InitDefaults_event_2eproto_impl;
%ignore sV::events::protobuf_AddDesc_event_2eproto_impl;
%ignore sV::events::protobuf_AssignDesc_event_2eproto;
%ignore sV::events::protobuf_ShutdownFile_event_2eproto;

//%ignore "sV::events::*::operator=";  // doesn't work
%ignore sV::events::AssemblySummary::operator=;
%ignore sV::events::SimulatedEvent::operator=;
%ignore sV::events::ExperimentalEvent::operator=;
%ignore sV::events::DetectorSummary::operator=;
%ignore sV::events::Displayable::operator=;
%ignore sV::events::Event::operator=;
%ignore sV::events::MulticastMessage_SenderStatusMessage::operator=;
%ignore sV::events::MulticastMessage::operator=;
%ignore sV::events::BucketInfoEntry::operator=;
%ignore sV::events::BucketInfo::operator=;
%ignore sV::events::CommonBucketDescriptor::operator=;
%ignore sV::events::Bucket::operator=;
%ignore sV::events::ZLib_Parameters::operator=;
%ignore sV::events::BZip2_Parameters::operator=;
%ignore sV::events::LZMA_Parameters::operator=;
%ignore sV::events::CompressedData::operator=;
%ignore sV::events::DeflatedBucket::operator=;
%ignore sV::events::TestingMessage::operator=;

%include "event.pb.h"

%{
# include "sV_config.h"
# include "event.pb.h"

#if !defined( RPC_PROTOCOLS )
# include <google/protobuf/message.h>
# include "uevent.hpp"
#error "RPC_PROTOCOLS is not " \
"defined. Unable to build events py-wrapper module."
#endif

%}

// vim: ft=swig
