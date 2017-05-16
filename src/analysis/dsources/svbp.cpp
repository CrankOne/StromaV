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

namespace buckets {

BucketReader::BucketReader( events::Bucket * reentrantBucketPtr ) :
            iEventSequence( 0x0 ),
            _cBucket( reentrantBucketPtr ),
            _it( reentrantBucketPtr ) {
}

const events::Bucket &
BucketReader::_V_bucket() const {
    if( !_cBucket ) {
        emraise( badState, "Bucket pointer was not set for reading handle %p.",
            this );
    }
    return *_cBucket;
}

bool
BucketReader::_V_is_good() {
    return _it.sym().nEvent < (size_t) bucket().events_size();
}

void
BucketReader::_V_next_event( Event *& ePtr ) {
    ePtr = &(::sV::mixins::PBEventApp::c_event());
    ePtr->Clear();
    ePtr->CopyFrom( bucket().events( _it.sym().nEvent ) );
    ++_it;
}

BucketReader::Event *
BucketReader::_V_initialize_reading() {
    Event * eventPtr;
    _V_next_event( eventPtr );
    return eventPtr;
}

void
BucketReader::_V_finalize_reading() {
    _it.sym().nEvent = 0;
}

size_t
BucketReader::n_events() const {
    if( !is_bucket_set() ) {
        return 0;
    }
    return bucket().events_size();
}

//
//
//

MetaInfoBucketReader::MetaInfoBucketReader( events::Bucket * bucketPtr ) :
                                                iEventSequence( 0x0 ),
                                                BucketReader( bucketPtr ) {}

//
//
//

CompressedBucketReader::CompressedBucketReader(
                events::DeflatedBucket * dfltdBcktPtr,
                events::Bucket * bucketPtr,
                const Decompressors * decompressors ) :
                        iEventSequence( 0x0 ),
                        MetaInfoBucketReader( bucketPtr ),
                        _dfltdBucketPtr( dfltdBcktPtr ),
                        _decompressedBucketValid( false ),
                        _decompressors( decompressors )
{}

const events::DeflatedBucket &
CompressedBucketReader::deflated_bucket() const {
    if( !_dfltdBucketPtr ) {
        emraise( badState, "Deflated bucket pointer was not set for "
            "reading handle %p.", this );
    }
    return *_dfltdBucketPtr;
}

const events::BucketMetaInfo &
CompressedBucketReader::_V_metainfo() const {
    return deflated_bucket().metainfo();
}

const iDecompressor *
CompressedBucketReader::_decompressor( iDecompressor::CompressionAlgo algo ) const {
    auto it = _decompressors->find( algo );
    if( _decompressors->end() == it ) {
        emraise( notFound, "Has no decompressor referenced by code %d.",
                (int) algo );
    }
    return it->second;
}

void
CompressedBucketReader::_decompress_bucket() const {
    const iDecompressor * dcmprPtr = _decompressor(
                                    _dfltdBucketPtr->data().compressionalgo() );
    size_t decompressedLength;  // provisioned
    _dcmBuffer.reserve(
            decompressedLength = dcmprPtr->decompressed_dest_buffer_len(
                (const uint8_t *) _dfltdBucketPtr->data().compressedcontent().c_str(),
                _dfltdBucketPtr->data().compressedcontent().size()
            ) );
    _dcmBuffer.resize(
        dcmprPtr->decompress_series(
            (const uint8_t *) _dfltdBucketPtr->data().compressedcontent().c_str(),
            _dfltdBucketPtr->data().compressedcontent().size(),
            _dcmBuffer.data(),
            _dcmBuffer.capacity() )
        );
    if( decompressedLength < _dcmBuffer.size() ) {
        emraise( badState, "Decompressed length was erroneousle predicted: "
                "%zu (provisioned) < %zu (real).",
                decompressedLength, _dcmBuffer.size() );
    }
    if( ! const_cast<CompressedBucketReader *>(this)
                ->_mutable_bucket_ptr()
                ->ParseFromArray( _dcmBuffer.data(), _dcmBuffer.size() ) ) {
        emraise( thirdParty, "Protobuf failed to parse decompressed data of "
            "size %zu.", _dcmBuffer.size() );
    }
    _decompressedBucketValid = true;
}

const events::Bucket &
CompressedBucketReader::_V_bucket() const  {
    if( !is_decompressed_bucket_valid() ) {
        const_cast<CompressedBucketReader *>(this)->_decompress_bucket();
    }
    return BucketReader::_V_bucket();
}

void
CompressedBucketReader::set_bucket_ptr( events::Bucket * ptr ) {
    invalidate_decompressed_bucket_cache();
    MetaInfoBucketReader::set_bucket_ptr( ptr );
}

void
CompressedBucketReader::set_deflated_bucket_ptr( events::DeflatedBucket * dfltBctPtr ) {
    invalidate_decompressed_bucket_cache();
    _dfltdBucketPtr = dfltBctPtr;
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

}  // namespace ::sV::buckets

}  // namespace sV

