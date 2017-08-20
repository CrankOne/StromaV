%module buckets

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

%import(module="StromaV.pipeline") "pipeline.i"
%import(module="StromaV.gooDict") "gooDict.i"

/* SWIG of versions at least >=2.0.9 doesn't like the C++11 override/final
 * keywords, so we get rid of them using these macro defs: */
# ifdef SWIG
# define override
# define final
# endif  // SWIG

%import "sV_config.h"

%{

# include "sV_config.h"

#if !defined( RPC_PROTOCOLS )
#error "RPC_PROTOCOLS is not " \
"defined. Unable to build pipeline py-wrapper module."
#endif

#include "analysis/pipeline.hpp"
#if !defined( RPC_PROTOCOLS ) || !defined( ANALYSIS_ROUTINES )
#error "Either RPC_PROTOCOLS or ANALYSIS_ROUTINES not defined. Unable to " \
"build analysis module."
#endif

# include "analysis/dsources/svbs/buckets.hpp"

%}

%include "analysis/dsources/svbs/bucketReader.hpp"
%include "analysis/dsources/svbs/suppInfoBucketReader.hpp"
%include "analysis/dsources/svbs/compressedBucketReader.hpp"
%include "analysis/dsources/svbs/aux.hpp"
%include "analysis/dsources/svbs/bucketStreamReader.tcc"
%template(BucketStreamReader_SHA256_Common) sV::buckets::BucketStreamReader<
                                sV::aux::SHA256BucketHash,
                                sV::events::CommonBucketDescriptor>;
%include "analysis/dsources/svbs/buckets.hpp"

// vim: ft=swig

