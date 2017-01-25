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
# include "buckets/iBucketDispatcher.hpp"

# ifdef RPC_PROTOCOLS

# include "event.pb.h"
# include <iostream>
// # include <fstream>

namespace sV {

iBucketDispatcher::iBucketDispatcher(
        size_t nMaxKB,
        size_t nMaxEvents ) :
    _nMaxKB(nMaxKB),
    _nMaxEvents(nMaxEvents) {
};

iBucketDispatcher::~iBucketDispatcher() {
}

size_t iBucketDispatcher::drop_bucket() {
    return _V_drop_bucket();
}

void iBucketDispatcher::clear_bucket() {
    _currentBucket.Clear();
}

bool iBucketDispatcher::is_bucket_full() {
    return ( (n_KB() >= _nMaxKB && _nMaxKB != 0) ||
         (n_Events() >= _nMaxEvents && _nMaxEvents != 0) );
}

void iBucketDispatcher::push_event(const events::Event & reentrantEvent) {
    events::Event* event = _currentBucket.add_events();
    event->CopyFrom(reentrantEvent);

    std::cout << std::dec << "BucketCached size: " << n_KB();
    std::cout << " | events size: " << _currentBucket.events_size() << std::endl;
    if ( is_bucket_full() ) {
        std::cout << "---Drop bucket---" << std::endl;
        std::cout << " BucketCached size: " << n_KB();
        std::cout << " max size KB: " << _nMaxKB << std::endl;
        std::cout << " events size: " << _currentBucket.events_size();
        std::cout << " max event size: "<< _nMaxEvents << std::endl;
        _V_drop_bucket();
        // _currentBucket.Clear();  // move this call to _V_drop_bucket() ?
    }
}

po::options_description iBucketDispatcher::_dispatcher_options() {
    po::options_description dispatcherCfg("BucketDispatcher options");
    dispatcherCfg.add_options()
        ("b-dispatcher.maxBucketSize.KB",
         po::value<int>()->default_value(500),
         "Maximum size (in kbytes) of the bucket storing without \
         serialization")
        ("b-dispatcher.maxBucketSize.events",
         po::value<int>()->default_value(0),
         "Maximum size (nof enents) of the bucket storing without \
         serialization")
        ("b-dispatcher.comressionAlgorithm",
         po::value<std::string>()->default_value("bz2"),
         "Compression algorithm for bucket compression")
        ("b-dispatcher.outFile",
         po::value<std::string>()->default_value("/tmp/testout.buckets"),
         "Output file for serialized data")
        ;
    return dispatcherCfg;
}

}  //  namespace sV

# endif  //  RPC_PROTOCOLS

