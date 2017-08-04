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

# include "analysis/processors/evStreamDispatch.hpp"

# if defined(RPC_PROTOCOLS) && defined(ANALYSIS_ROUTINES)

# include "ctrs_dict.hpp"

# include <goo_dict/parameters/path_parameter.hpp>
# include <goo_dict/parameter.tcc>

namespace sV {
namespace dprocessors {

EventsDispatcher::EventsDispatcher( 
                            const std::string & pn,
                            std::ostream * streamPtr ) :
                    AnalysisPipeline::iEventProcessor( pn ),
                    ASCII_Entry( goo::aux::iApp::exists() ?
                                &goo::app<AbstractApplication>() : nullptr, 2 ),
                    buckets::CompressedDispatcher( nullptr, *streamPtr ),
                _stream( streamPtr ),
                _genericCollectorPtr( nullptr ),
                _nBucketsDropped(0) {
    bzero(_prevHash, SHA256_DIGEST_LENGTH);
}

EventsDispatcher::EventsDispatcher(
            const std::string & pn,
            const goo::dict::Dictionary & dct,
            std::ostream * streamPtr ) : 
    EventsDispatcher( pn, streamPtr ) {

    const std::string compressionMethodName =
                dct["compression"].as<std::string>();

    compressor( sV::generic_new<iCompressor>( compressionMethodName ) );

    buckets::GenericCollector * generic = nullptr;
    for( const auto & nm : dct["suppInfo"].as_list_of<std::string>() ) {
        buckets::iAbstractInfoCollector * collectorPtr =
                            generic_new<buckets::iAbstractInfoCollector>( nm );
        add_collector( nm, *collectorPtr );
        if( "Generic" == nm ) {
            generic = dynamic_cast<buckets::GenericCollector *>( collectorPtr );
            if( !generic ) {
                emraise( badCast, "Wrong collector instance referred as "
                    "\"Generic\". RTTI type is \"%s\".", typeid(collectorPtr).name() );
            }
            _set_generic_collector( generic );
        }
        sV_log3( "Collector \"%s\", %p associated with eventsStore processor %p.\n",
            nm.c_str(), collectorPtr, this );
    }
    if( generic ) {
        _generic_collector().maximum_events( dct["maxBucketSize_events"].as<size_t>() );
        _generic_collector().maximum_raw_data_size( dct["maxBucketSize_kB"].as<size_t>()*1024 );
        sV_log2( "\"Generic\" collector %p is set for eventsStore processor %p.\n",
            generic, this);
    } else {
        // May be or may be not causing errors. For standard usage, the Generic
        // collector must be present. However for possible future inheritance
        // scenarios printing a warning here may not be desirable.
        sV_logw( "Configuration dict of events store %p does not include "
            "\"Generic\" collector.\n" );
    }
    bzero(_prevHash, SHA256_DIGEST_LENGTH);
}

EventsDispatcher::~EventsDispatcher() {}

void
EventsDispatcher::_set_generic_collector( buckets::GenericCollector * gcPtr ) {
    if( _genericCollectorPtr ) {
        emraise( badState, "EventsDispatcher instance %p can not manage more "
            "than one generic bucket info collector instance (inserting %p, "
            "having %p).",
            this, _genericCollectorPtr, gcPtr );
    }
    _genericCollectorPtr = gcPtr;
}

buckets::GenericCollector &
EventsDispatcher::_generic_collector() {
    if( !_genericCollectorPtr ) {
        emraise( badState, "Generic bucket info collector is not set for "
            "EventsDispatcher instance %p.", this );
    }
    return *_genericCollectorPtr;
}

aux::iEventProcessor::ProcRes
EventsDispatcher::_V_process_event( Event & uEvent ){
    push_event( uEvent );
    _update_stat();
    return RC_ACCOUNTED;
}

void
EventsDispatcher::_V_finalize() const {
    if( ! is_empty() ) {
        // todo: why does _V_finalize() has a const qualifier?
        const_cast<EventsDispatcher *>(this)->drop_bucket();
    }
}

void
EventsDispatcher::_update_stat() {
    if( !can_acquire_display_buffer() ) return;
    char ** lines = my_ascii_display_buffer();
    assert( lines[0] && lines[1] && !lines[2] );

    size_t rawL = latest_dropped_raw_len(),
           cmrsdL = latest_dropped_compressed_len();
    char compressionStatStr[32] = "--";
    if( rawL ) {
        snprintf( compressionStatStr, sizeof(compressionStatStr),
            "%d%%", 100 - int(100*(double(cmrsdL)/rawL)) );
    }

    if( _genericCollectorPtr ) {
        size_t kbFilled = _generic_collector().raw_data_size()/1024,
               nMaxKb = _generic_collector().maximum_raw_data_size()/1024,
               nEvents = _generic_collector().number_of_events(),
               nMaxEvents = _generic_collector().maximum_events()
               ;
        const uint8_t * latestHash = _generic_collector().hash();
        if( memcmp( _prevHash, latestHash, SHA256_DIGEST_LENGTH ) ) {
            // Hash have changed --- bucket dropped.
            memcpy( _prevHash, latestHash, SHA256_DIGEST_LENGTH );
            ++_nBucketsDropped;
        }
        snprintf( lines[0], ::sV::aux::ASCII_Display::LineLength,
                "Bucket: %.2f KB/event; %6zu/%zu events; %6zu/%zu Kbytes. Compress.ratio: %s",
                ( nEvents ? double(kbFilled)/nEvents : 0 ),
                nEvents, nMaxEvents,
                kbFilled, nMaxKb,
                compressionStatStr );
        snprintf( lines[1], ::sV::aux::ASCII_Display::LineLength,
                "  dropped:%4u, latest SHA256 hash: %02x%02x%02x%02x...%02x%02x%02x%02x",
                    _nBucketsDropped,
                    latestHash[0], latestHash[1], latestHash[2], latestHash[3],
                    latestHash[SHA256_DIGEST_LENGTH-4],
                    latestHash[SHA256_DIGEST_LENGTH-3],
                    latestHash[SHA256_DIGEST_LENGTH-2],
                    latestHash[SHA256_DIGEST_LENGTH-1] );
    } else {
        snprintf( lines[0], ::sV::aux::ASCII_Display::LineLength,
                "<no generic collector info available for %p>", this );
    }
}

}  // namespace ::sV::dprocessors
}  // namespace sV

# endif  // defined(RPC_PROTOCOLS) && defined(ANALYSIS_ROUTINES)


