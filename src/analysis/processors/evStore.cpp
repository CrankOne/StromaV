/*
 * Copyright (c) 2016 Renat R. Dusaev <crank@qcrypt.org>
 * Author: Renat R. Dusaev <crank@qcrypt.org>
 * Author: Bogdan Vasilishin <togetherwithra@gmail.com>
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

# include "analysis/processors/evStore.hpp"

# if defined(RPC_PROTOCOLS) && defined(ANALYSIS_ROUTINES)

# if 0

# include "ctrs_dict.hpp"
# include "buckets/compressedDispatcher.hpp"

# include <goo_dict/parameters/path_parameter.hpp>
# include <goo_dict/parameter.tcc>

namespace sV {
namespace dprocessors {

EventsStore::EventsStore( const goo::dict::Dictionary & dct ) :
                    EventsDispatcher( "eventsStore", dct, &_file ) {
    _file.open( dct["outFile"].as<goo::filesystem::Path>()/*.interpolated()*/,
                std::ios::out | std::ios::binary );
    //set_out_stream( _file );
}

EventsStore::~EventsStore() {
    _file.close();
}

StromaV_ANALYSIS_PROCESSOR_DEFINE_MCONF( EventsStore, "eventsStore" ) {
    goo::dict::Dictionary evStorePDict( "eventsStore",
        "StromaV offers a kind of facility to store and retreive serialized "
        "data (usually physical events) based on Google Protocol Buffers. The "
        "so-called \"bucket dispatcher\" performs accumulation and in-RAM "
        "temporary bufferization of events. Such a write-back caching "
        "mechanism (pretty common) allows to increase performance. Note that "
        "despite of using standard compressed bucket dispatcher collector, the "
        "drop-bucket criteria (max events and max bucket size) will be "
        "parameterised in own section instead of common \"buckets\" section "
        "in common config.");
    evStorePDict.insertion_proxy()
        .p<size_t>("maxBucketSize_kB",
                        "Maximum bucket capacity (in kilobytes). 0 disables"
                        "criterion.",
                    500)
        .p<size_t>("maxBucketSize_events",
                        "Maximum bucket capacity (number of enents). 0 disables "
                        "criterion.",
                    0)
        .p<std::string>("compression",
                        "Algorithm name for compressing buckets.",
                    "bz2")
        .list<std::string>( "suppInfo",
                        "Supplementary info collectors.",
                        { "Generic" } )
        .p<goo::filesystem::Path>("outFile",
                        "Output file for serialized data.",
                    "/tmp/sV_latest.svbs" )
        ;
    goo::dict::DictionaryInjectionMap injM;
        injM( "maxBucketSize_kB",       "analysis.processors.store.maxBucketSize_kB" )
            ( "maxBucketSize_events",   "analysis.processors.store.maxBucketSize_events" )
            ( "compression",            "analysis.processors.store.compression" )
            ( "suppInfo",               "analysis.processors.store.suppInfo" )
            ( "outFile" ,               "analysis.processors.store.outFile" )
            ;
    return std::make_pair( evStorePDict, injM );
}

}  // namespace ::sV::dprocessors
}  // namespace sV

# endif

# endif  // defined(RPC_PROTOCOLS) && defined(ANALYSIS_ROUTINES)


