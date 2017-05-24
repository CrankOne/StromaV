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
# include "buckets/compressedDispatcher.hpp"
//# include "buckets/iBundlingDispatcher.hpp"

# include <goo_dict/parameters/path_parameter.hpp>
//# include <goo_dict/parameters/pointer_parameter.tcc>
# include <goo_dict/parameter.tcc>

namespace sV {
namespace dprocessors {

EventsStore::EventsStore(   const std::string & pn,
                            const std::string & filename,
                            buckets::iBundlingDispatcher * bucketDispatcher) :
                AnalysisPipeline::iEventProcessor( pn ),
                ASCII_Entry( goo::aux::iApp::exists() ?
                            &goo::app<AbstractApplication>() : nullptr, 1 ),
                _genericCollectorPtr( nullptr ) {
    _file.open( filename, std::ios::out | std::ios::binary );
    _bucketDispatcher = bucketDispatcher;
}

EventsStore::EventsStore( const goo::dict::Dictionary & dct ) :
                    iEventProcessor( "eventsStore" ),
                    ASCII_Entry( goo::aux::iApp::exists() ?
                            &goo::app<AbstractApplication>() : nullptr, 2 ),
                    _file(  dct["outFile"].as<goo::filesystem::Path>(),
                            std::ios::out | std::ios::binary ),
                    _genericCollectorPtr(nullptr) {
    const std::string compressionMethodName =
                dct["compression"].as<std::string>();
    iCompressor * compressorPtr = sV::generic_new<iCompressor>( compressionMethodName );
    _bucketDispatcher = new buckets::CompressedDispatcher( compressorPtr, _file );
    buckets::iAbstractInfoCollector * generic = nullptr;
    for( const auto & nm : dct["suppInfo"].as_list_of<std::string>() ) {
        buckets::iAbstractInfoCollector * collectorPtr =
                            generic_new<buckets::iAbstractInfoCollector>( nm );
        _bucketDispatcher->add_collector( nm, *collectorPtr );
        if( "Generic" == nm ) {
            generic = collectorPtr;
        }
        sV_log3( "Collector \"%s\", %p associated with eventsStore processor %p.\n",
            nm.c_str(), collectorPtr, this );
    }
    if( generic ) {
        _genericCollectorPtr = dynamic_cast<buckets::GenericCollector *>( generic );
        _genericCollectorPtr->maximum_events( dct["maxBucketSize_events"].as<size_t>() );
        _genericCollectorPtr->maximum_raw_data_size( dct["maxBucketSize_kB"].as<size_t>()*1024 );
        sV_log2( "\"Generic\" collector %p is set for eventsStore processor %p.\n",
            _genericCollectorPtr, this);
    } else {
        // May be or may be not causing errors. For standard usage, the Generic
        // collector must be present. However for possible future inheritance
        // scenarios printing a warning here may not be desirable.
        sV_logw( "Configuration dict of events store %p does not include "
            "\"Generic\" collector.\n" );
    }
}

EventsStore::~EventsStore() {}

bool
EventsStore::_V_process_event( Event * uEvent ){
    _bucketDispatcher->push_event( *(uEvent) );
    _update_stat();
    return true;
}

void
EventsStore::_V_finalize() const {
    if( ! _bucketDispatcher->is_empty() ) {
        _bucketDispatcher->drop_bucket();
    }
}

void
EventsStore::_update_stat() {
    if( !can_acquire_display_buffer() ) return;
    char ** lines = my_ascii_display_buffer();
    assert( lines[0] && lines[1] && !lines[2] );

    size_t rawL = static_cast<buckets::CompressedDispatcher*>(_bucketDispatcher)
                                            ->latest_dropped_raw_len(),
           cmrsdL = static_cast<buckets::CompressedDispatcher*>(_bucketDispatcher)
                                            ->latest_dropped_compressed_len()
           ;
    char compressionStatStr[32] = "--";
    if( rawL ) {
        snprintf( compressionStatStr, sizeof(compressionStatStr),
            "%d%%", int(100*(double(cmrsdL)/rawL)) );
    }

    if( _genericCollectorPtr ) {
        size_t kbFilled = _genericCollectorPtr->raw_data_size()/1024,
               nMaxKb = _genericCollectorPtr->maximum_raw_data_size()/1024,
               nEvents = _genericCollectorPtr->number_of_events(),
               nMaxEvents = _genericCollectorPtr->maximum_events()
               ;
        snprintf( lines[0], ::sV::aux::ASCII_Display::LineLength,
                "Bucket: %.2f KB/event; %zu =< %zu events; %zu =< %zu Kbytes. Compress.ratio: %s",
                ( nEvents ? double(kbFilled)/nEvents : 0 ),
                nEvents, nMaxEvents,
                kbFilled, nMaxKb,
                compressionStatStr );
    } else {
        snprintf( lines[0], ::sV::aux::ASCII_Display::LineLength,
                "<no generic collector info available for %p>", this );
    }
    snprintf( lines[1], ::sV::aux::ASCII_Display::LineLength,
                "<here be dragons>" );  // TODO: latest bucket hash
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

}  // namespace dprocessors
}  // namespace sV

# endif  // defined(RPC_PROTOCOLS) && defined(ANALYSIS_ROUTINES)


