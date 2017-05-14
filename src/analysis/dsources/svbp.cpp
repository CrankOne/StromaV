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

# include "analysis/dsources/svbs.hpp"

namespace sV {

BucketReader::BucketReader( events::Bucket * reentrantBucketPtr ) :
            iEventSequence( 0x0 ),
            _cBucket( reentrantBucketPtr ),
            _currentEvent(0) {
    assert(_cBucket);
}

events::Bucket &
BucketReader::bucket() {
    assert( _cBucket );
    return *_cBucket;
}

const events::Bucket &
BucketReader::bucket() const {
    assert( _cBucket );
    return *_cBucket;
}

bool
BucketReader::_V_is_good() {
    return (int) _currentEvent < bucket().events_size();
}

void
BucketReader::_V_next_event( Event *& ePtr ) {
    ePtr = &(::sV::mixins::PBEventApp::c_event());
    ePtr->Clear();
    ePtr->CopyFrom( bucket().events(_currentEvent) );
    ++_currentEvent;
}

BucketReader::Event *
BucketReader::_V_initialize_reading() {
    Event * eventPtr;
    _V_next_event( eventPtr );
    return eventPtr;
}

void
BucketReader::_V_finalize_reading() {
    _cBucket->Clear();
}

void
BucketReader::read_nth_event( Event *& evPtr, size_t evNo ) const {
    if( evNo >= bucket().events_size() ) {
        emraise( overflow, "Attempt of reading %zu-th event from bucket of %d "
            "events.", evNo, bucket().events_size() );
    }
    if( !ePtr ) {
        ePtr = &(::sV::mixins::PBEventApp::c_event());
    }
    ePtr->Clear();
    ePtr->CopyFrom( bucket().events(_currentEvent) );
}

size_t
BucketReader::n_events() const {
    return bucket().events_size();
}

# if 0
//void
//BucketReader::_V_print_brief_summary( std::ostream & ) const {}

BucketsFileReader::BucketsFileReader(
            const std::list<goo::filesystem::Path> & filenames,
            size_t maxEvents,
            bool enableProgressbar ) :
                        Parent( 0x0 ),  // TODO: md support
                        sV::AbstractApplication::ASCII_Entry( goo::aux::iApp::exists() ?
                                &goo::app<AbstractApplication>() : nullptr, 1 ),
                        _filenames( filenames.begin(), filenames.end() ),
                        _nEventsMax(0),
                        _pbParameters(nullptr) {
    if( enableProgressbar && maxEvents ) {
        _pbParameters = new PBarParameters;
        bzero( _pbParameters, sizeof(PBarParameters) );
        _pbParameters->mtrestrict = 250;
        _pbParameters->length = 80;
        _pbParameters->full = maxEvents;
    }
}
# endif

}  // namespace sV

