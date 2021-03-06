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

# include "analysis/processors/bucketer.hpp"

# if defined(RPC_PROTOCOLS) && defined(ANALYSIS_ROUTINES)

# include "buckets/ComprBucketDispatcher.hpp"
# include "compr/DummyCompressor.hpp"

namespace sV {
namespace dprocessors {

Bucketer::Bucketer( const std::string & pn,
                    sV::iBucketDispatcher * bucketDispatcher,
                    std::fstream * fileRef ) :
                        AnalysisPipeline::iEventProcessor( pn ) {
    _bucketDispatcher = bucketDispatcher;
    _fileRef = fileRef;
}

Bucketer::~Bucketer() {
    _fileRef->close();
}

bool
Bucketer::_V_process_event( Event * uEvent ){
    _bucketDispatcher->push_event( *(uEvent) );
    return true;
}

StromaV_DEFINE_CONFIG_ARGUMENTS {
    po::options_description bucketerP = sV::iBucketDispatcher::_dispatcher_options();
    return bucketerP;
}
StromaV_DEFINE_DATA_PROCESSOR( BucketerProcessor ) {
    std::fstream * fileRef = new std::fstream();
    fileRef->open(goo::app<sV::AbstractApplication>().cfg_option<std::string>
        ("b-dispatcher.outFile"), std::ios::out | std::ios::binary |
                                  std::ios::app );
    sV::DummyCompressor * compressor = new sV::DummyCompressor;
    sV::ComprBucketDispatcher * dispatcher = new sV::ComprBucketDispatcher(
                compressor,
                *(fileRef),
                (size_t)goo::app<sV::AbstractApplication>().cfg_option<int>
                ("b-dispatcher.maxBucketSize.KB"),
                (size_t)goo::app<sV::AbstractApplication>().cfg_option<int>
                ("b-dispatcher.maxBucketSize.events"),
                (size_t)goo::app<sV::AbstractApplication>().cfg_option<int>
                ("b-dispatcher.BufSize.KB")
            );
    return new Bucketer("bucketer", dispatcher, fileRef);
} StromaV_REGISTER_DATA_PROCESSOR( BucketerProcessor,
    "bucketer",
    "Processor performing accumulation of events into buckets. TODO: more doc" )

}  // namespace dprocessors
}  // namespace sV

# endif  // defined(RPC_PROTOCOLS) && defined(ANALYSIS_ROUTINES)


