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

# include "ctrs_dict.hpp"
# include "buckets/compressed_bdsp.hpp"

# include <goo_dict/parameters/path_parameter.hpp>

namespace sV {
namespace dprocessors {

EventsStore::EventsStore(   const std::string & pn,
                            const std::string & filename,
                            sV::iBucketDispatcher * bucketDispatcher) :
                AnalysisPipeline::iEventProcessor( pn ),
                ASCII_Entry( goo::aux::iApp::exists() ?
                            &goo::app<AbstractApplication>() : nullptr, 1 ) {
    _file.open( filename, std::ios::out | std::ios::binary | std::ios::app );
    _bucketDispatcher = bucketDispatcher;
}

EventsStore::EventsStore( const goo::dict::Dictionary & dct ) :
                    iEventProcessor( "eventsStore" ),
                    ASCII_Entry( goo::aux::iApp::exists() ?
                            &goo::app<AbstractApplication>() : nullptr, 1 ),
                    _file(  dct["outFile"].as<goo::filesystem::Path>(),
                            std::ios::out | std::ios::binary | std::ios::app ) {
    const std::string compressionMethodName =
                dct["compression"].as<std::string>();
    iCompressor * cmprPtr = sV::generic_new<iCompressor>( compressionMethodName );
    _bucketDispatcher = new CompressedBucketDispatcher( cmprPtr, _file,
                dct["maxBucketSize_kB"].as<uint32_t>(),
                dct["maxBucketSize_events"].as<uint32_t>() );
}

EventsStore::~EventsStore() {}

bool
EventsStore::_V_process_event( Event * uEvent ){
    _bucketDispatcher->push_event( *(uEvent) );
    _update_stat();
    return true;
}

void
EventsStore::_update_stat() {
    if( !can_acquire_display_buffer() ) return;
    char ** lines = my_ascii_display_buffer();
    assert( lines[0] && !]ines[1] );

    size_t kbFilled = _bucketDispatcher->n_bytes()/1024;
    snprintf( lines[0], ::sV::aux::ASCII_Display::LineLength,
            "Bucket: %.2f KB/event %zu / %zu events, %zu / %zu Kbytes",
            ( _bucketDispatcher->n_events() ? double(kbFilled)/_bucketDispatcher->n_events() : 0 ),
            _bucketDispatcher->n_events(), _bucketDispatcher->n_max_events(),
            kbFilled, _bucketDispatcher->n_max_KB() );
    // ...
}

StromaV_ANALYSIS_PROCESSOR_DEFINE_MCONF( EventsStore, "eventsStore" ) {
    goo::dict::Dictionary evStorePDict( "eventsStore",
        "StromaV offers a kind of facility to store and retreive serialized "
        "data (usually physical events) based on Google Protocol Buffers. The "
        "so-called \"bucket dispatcher\" performs accumulation and in-RAM "
        "temporary bufferization of events. Such a write-back caching "
        "mechanism (pretty common) allows to increase performance." );
    evStorePDict.insertion_proxy()
        .p<uint32_t>("maxBucketSize_kB",
                        "Maximum bucket capacity (in kilobytes). 0 disables"
                        "criterion.", 500)
        .p<uint32_t>("maxBucketSize_events",
                        "Maximum bucket capacity (number of enents). 0 disables "
                        "criterion.", 0)
        .p<std::string>("compression",
                        "Algorithm name for compressing buckets.",
                        "bz2")
        .p<goo::filesystem::Path>("outFile",
                        "Output file for serialized data.",
                        "/tmp/sV_latest.svbs" )
        ;
    goo::dict::DictionaryInjectionMap injM;
        injM( "maxBucketSize_kB",       "analysis.processors.store.maxBucketSize_kB" )
            ( "maxBucketSize_events",   "analysis.processors.store.maxBucketSize_events" )
            ( "compression",            "analysis.processors.store.compression" )
            ( "outFile" ,               "analysis.processors.store.outFile" )
            ;
    return std::make_pair( evStorePDict, injM );
}

# if 0
StromaV_DEFINE_CONFIG_ARGUMENTS( commonConfig ) {
    commonConfig.insertion_proxy().bgn_sect( "bucketDispatching",
        "StromaV offers a kind of facility to store and retreive serialized "
        "data (usually physical events) based on Google Protocol Buffers. The "
        "so-called \"bucket dispatcher\" performs accumulation and in-RAM "
        "temporary bufferization of events. Such a write-back caching "
        "mechanism (pretty common) allows to increase performance.")
        .p<uint32_t>("maxBucketSize_kB",
                        "Maximum bucket capacity (in kilobytes). 0 --- "
                        "criterion not used.", 500)
        .p<uint32_t>("maxBucketSize_events",
                        "Maximum bucket capacity (number of enents). 0 --- "
                        "criterion not used.", 0)
        .p<std::string>("compression",
                        "Algorithm name for compressing buckets.",
                        "bz2")
        .p<std::string>("outFile",
                        "Default output file for serialized buckets.",
                        "/tmp/sV_latest.svbs" )
    .end_sect("bucketDispatching")
    ;
}
StromaV_DEFINE_DATA_PROCESSOR( BucketerProcessor ) {
    std::fstream * fileRef = new std::fstream();
    fileRef->open(goo::app<sV::AbstractApplication>().cfg_option<std::string>
        ("bucketDispatching.outFile"), std::ios::out | std::ios::binary |
                                  std::ios::app );
    sV::DummyCompressor * compressor = new sV::DummyCompressor;
    sV::ComprBucketDispatcher * dispatcher = new sV::ComprBucketDispatcher(
                compressor,
                *(fileRef),
                (size_t) goo::app<sV::AbstractApplication>().cfg_option<uint32_t>
                ("bucketDispatching.maxBucketSize_kB"),
                (size_t) goo::app<sV::AbstractApplication>().cfg_option<uint32_t>
                ("bucketDispatching.maxBucketSize_events"),
                (size_t) goo::app<sV::AbstractApplication>().cfg_option<uint32_t>
                ("bucketDispatching.bufferSize_kB")
            );
    return new Bucketer("bucketer", dispatcher, fileRef);
} StromaV_REGISTER_DATA_PROCESSOR( BucketerProcessor,
    "save",
    "This processor writes events received from analysis pipeline to file. "
    "File is referenced by \"bucketDispatching.outFile\" argument. Write-back "
    "caching may be configured using other options from \"bucketDispatching\" "
    "subsection.")
# endif

}  // namespace dprocessors
}  // namespace sV

# endif  // defined(RPC_PROTOCOLS) && defined(ANALYSIS_ROUTINES)


