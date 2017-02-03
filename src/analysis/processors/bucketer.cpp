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

# include "analysis/processors/bucketer.hpp"

# if defined(RPC_PROTOCOLS) && defined(ANALYSIS_ROUTINES)

namespace sV {
namespace dprocessors {

Bucketer::Bucketer( const std::string &,
                    sV::iBucketDispatcher * bucketDispatcher ) {
    _bucketDispatcher = bucketDispatcher;
}

bool
Bucketer::_V_process_event( Event * uEvent ){
    _TODO_  // TODO
    if( is_bucket_full() ) {
        drop_bucket();
    }
    return false;
    _bucketDispatcher->push_event( uEvent );

}

StromaV_DEFINE_CONFIG_ARGUMENTS {
    po::options_description bucketerP = sV::iBucketDispatcher::_dispatcher_options();
    return bucketerP;
}
StromaV_DEFINE_DATA_PROCESSOR( TestingProcessor ) {
    _TODO_  // TODO
    # if 0
    return new Bucketer(
            "bucketer",
            goo::app<sV::AbstractApplication>().cfg_option<size_t>("multicast.storage-capacity"),
        );
    # endif
} StromaV_REGISTER_DATA_PROCESSOR( TestingProcessor,
    "bucketer",
    "Processor performing accumulation of events into buckets. TODO: more doc" )

}  // namespace dprocessors
}  // namespace sV

# endif  // defined(RPC_PROTOCOLS) && defined(ANALYSIS_ROUTINES)


