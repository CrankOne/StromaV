/*
 * Copyright (c) 2017 Renat R. Dusaev <crank@qcrypt.org>
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

# include "sV_config.h"

# ifdef RPC_PROTOCOLS

# include "analysis/dsources/svbs/buckets.hpp"
# include "app/analysis.hpp"

namespace sV {

Buckets::Buckets(
            events::DeflatedBucket * dfltdBcktPtr,
            events::Bucket * bucketPtr,
            events::BucketInfo * bInfo,
            events::CommonBucketDescriptor * particularInfo,
            const std::list<goo::filesystem::Path> & paths_,
            size_t nMaxEvents,
            int portNo,
            size_t recvBufferSize,
            bool enableProgressbar,
            uint8_t ASCII_lines ) :
                iEventSequence(0x0),
                Parent( dfltdBcktPtr, bucketPtr, bInfo, particularInfo,
                        &_decompressors, &_file ),
                ASCII_Entry( goo::aux::iApp::exists() ?
                        &goo::app<AbstractApplication>() : nullptr,
                        ASCII_lines ),
                _paths( paths_.begin(), paths_.end() ),
                _lastEvReadingWasGood(false),
                _pbParameters( nullptr ),
                _maxEventsNumber( nMaxEvents ),
                _eventsRead(0),
                _recvBufferSize(recvBufferSize),
                _recvStream(_recvBuffer),
                _reciever(portNo, this) {
    _recvBuffer.resize(_recvBufferSize);
    if( enableProgressbar && _maxEventsNumber ) {
        _pbParameters = new PBarParameters;
        bzero( _pbParameters, sizeof(PBarParameters) );
        _pbParameters->mtrestrict = 250;
        _pbParameters->length = 80;
        _pbParameters->full = _maxEventsNumber;
    }
    _sourcesIt = _paths.end();
    sV_mylog1( "Buckets %p: constructed.\n", this );
}

Buckets::Buckets( const goo::dict::Dictionary & dct ) : Buckets(
            sV_MSG_NEW( events::DeflatedBucket  ),
            sV_MSG_NEW( events::Bucket          ),
            sV_MSG_NEW( events::BucketInfo      ),
            sV_MSG_NEW( events::CommonBucketDescriptor ),
            std::list<goo::filesystem::Path>(
                goo::app<AbstractApplication>().app_options_list<goo::filesystem::Path>("input-file").begin(),
                goo::app<AbstractApplication>().app_options_list<goo::filesystem::Path>("input-file").end()
            ),
            goo::app<AbstractApplication>().app_option<size_t>("max-events-to-read"),
            dct["listeningPort"].as<int>(),
            dct["recvBufferSize_kB"].as<size_t>()*1024,
            dct["progressbar"].as<bool>(),
            1
        ) {}

Buckets::~Buckets() {
    for( auto p : _collectors ) {
        delete p.second;
    }
    for( auto p : _decompressors ) {
        delete p.second;
    }
}

std::type_index
Buckets::_V_get_collector_type_hash( const std::string & collectorName ) const {
    auto & sect = sV::sys::IndexOfConstructables::self()
                    .known_constructors<sV::buckets::iAbstractInfoCollector>();
    auto it = sect.find( collectorName );
    if( sect.end() == it ) {
        emraise( noSuchKey, "Unable to find RTTI registered for buckets supp "
            "info type \"%s\".", collectorName.c_str() );
    }
    return it->second->finalTypeIndex;
}

Buckets::MetaInfoCache
Buckets::_V_new_cache_entry( const std::string & collectorName ) const {
    MetaInfoCache ret;
    ret.name = collectorName;
    // Try to find existing collector:
    auto it = _collectors.find( collectorName );
    if( _collectors.end() == it ) {
        // If it is not found, construct one:
        it = _collectors.emplace(
                    collectorName,
                    sV::generic_new<sV::buckets::iAbstractInfoCollector>( collectorName )
                ).first;
    }
    ret.collectorPtr = it->second;
    ret.positionInMetaInfo = USHRT_MAX;
    return ret;
}

const iDecompressor *
Buckets::_decompressor( iDecompressor::CompressionAlgo algo ) const {
    auto it = _decompressors.find( algo );
    if( _decompressors.end() == it ) {
        std::string dcmprssrName = sV::sys::compression_algo_name( algo );
        it = _decompressors.emplace(
                    algo,
                    generic_new<iDecompressor>( dcmprssrName ) ).first;
    }
    return it->second;
}

Buckets::Event *
Buckets::_V_initialize_reading() {
    _open_source( *(_sourcesIt = _paths.begin())/*->interpolated()*/ );
    return Parent::_V_initialize_reading();
}

bool
Buckets::_V_is_good() {
    if( !_lastEvReadingWasGood ) {
        return false;
    }
    if( _maxEventsNumber && _eventsRead > _maxEventsNumber ) {
        return false;
    }
    return true;
}

bool
Buckets::_V_acquire_next_bucket( BucketReader::Event *& epr ) {
    if( _paths.end() == _sourcesIt ) {
        return _lastEvReadingWasGood = false;
    }
    if( Parent::_V_acquire_next_bucket( epr ) ) {
        sV_mylog2( "Next bucket in a queue acquired.\n" );
        return _lastEvReadingWasGood = true;
    }
    if( _paths.end() == ++_sourcesIt ) {
        sV_mylog2( "Buckets %p: has no more sources in queue of %zu entries. Done.\n",
            this, _paths.size() );
        return _lastEvReadingWasGood = false;
    }
    _file.close();
    _open_source( *_sourcesIt/*->interpolated()*/ );
    sV_mylog1( "Buckets %p: switched to next source in a queue: \"%s\".\n",
        this, _sourcesIt->/*interpolated().*/c_str() );
    return _lastEvReadingWasGood = Parent::_V_acquire_next_bucket( epr );
}

void
Buckets::_recieve_incoming_bucket(net::PeerConnection * clientConnectionPtr ) {
    buckets::RecievingServer::ClientConnection & cc =
            *static_cast<buckets::RecievingServer::ClientConnection *>(clientConnectionPtr);
    Buckets & self = cc.authority();
    // Recieve the incoming data into buffer:
    size_t nBytesRecieved = 0,
           nBytesOverall = 0;
    # define this (&self)
    sV_mylog3( "Recieving incoming data.\n" );
    do {
        nBytesRecieved = cc.recieve( const_cast<char*>(self._recvBuffer.data() + nBytesOverall),
                                     self._recvBufferSize );
        sV_mylog2( "Buckets %p: recv() fetched %zu bytes.\n",
                &self, nBytesRecieved);
        nBytesOverall += nBytesRecieved;
        if( nBytesRecieved ) {
            self._recvBuffer.resize( self._recvBuffer.size()
                                   + self._recvBufferSize );
        }
    } while( nBytesRecieved );
    self._recvBuffer.resize( nBytesOverall );
    sV_mylog1( "Buckets %p: recieved incoming "
        "data of size %zu.\n", &self, nBytesOverall );
    // Switch the stream:
    self.set_stream( self._recvStream );
    # undef this
}

void
Buckets::_open_source( const std::string & p ) {
    if( "@" != p ) {
        _file.open( p );
        set_stream( _file );
    } else {
        _reciever.bind_and_listen( 1 );
        _reciever.serve_connections( _recieve_incoming_bucket );
    }
}

StromaV_EVENTS_SEQUENCE_DEFINE_MCONF( Buckets, "svb" ) {
    goo::dict::Dictionary svbfRetrieve( "svb",
        "Generic reader for compressed buckets files (.svbp)." );
    svbfRetrieve.insertion_proxy()
        .flag("progressbar",
                "Displays simple ASCII progressbar on stdout.")
        .p<int>("listeningPort",
                "Network port for recieving buckets.",
            23011 )
        .p<size_t>("recvBufferSize_kB",
                "Size of recieving buffer in kilobytes. May have minor impact "
                "on performance while buckets (data) is supposed to be "
                "recieved via network.",
            256 )
        ;
    goo::dict::DictionaryInjectionMap injM;
        injM( "progressbar",        "analysis.data-sources.retreive.progressbar" )
            ( "listeningPort",      "analysis.data-sources.retreive.listeningPort" )
            ( "recvBufferSize_kB",  "analysis.data-sources.retreive.recvBufferSize_kB" )
            ;
    return std::make_pair( svbfRetrieve, injM );
}

}  // namespace sV

# endif  // RPC_PROTOCOLS


