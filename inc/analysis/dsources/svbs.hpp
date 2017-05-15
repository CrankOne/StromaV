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

# ifndef H_STROMA_V_SVBP_READER_H
# define H_STROMA_V_SVBP_READER_H

# include "sV_config.h"

# ifdef RPC_PROTOCOLS

# include "analysis/pipeline.hpp"
# include "utils.h"

# include <goo_mixins/iterable.tcc>

# include <fstream>

namespace sV {

namespace buckets {

namespace aux {

struct BucketEventID {
    events::Bucket * authorityPtr;
    size_t nEvent;

    BucketEventID( events::Bucket * bPtr, size_t nEve=0 ) :
                                        authorityPtr(bPtr), nEvent(nEve) {}

    BucketEventID & operator++() {
        ++nEvent;
        return *this;
    }
};

template<typename EventT>
class _BucketIterator : public goo::iterators::Iterator<
                                    goo::iterators::BaseBidirIterator,
                                    goo::iterators::BaseOutputIterator,
                                    goo::iterators::DirectionComparableIterator,
                                    _BucketIterator<EventT>,
                                    BucketEventID, EventT, size_t> {
private:
    BucketEventID _sym;
public:
    _BucketIterator( events::Bucket * bPtr, size_t nEve=0 ) :
                _sym( bPtr, nEve ) {}

    BucketEventID & sym() { return _sym; }
    const BucketEventID & sym() const { return _sym; }
};

}  // namespace ::sV::buckets::aux

typedef aux::_BucketIterator<const events::Event *> ConstBucketIterator;
typedef aux::_BucketIterator<events::Event *> BucketIterator;

/**@brief Performs basic reading from events bucket.
 * @class BucketReader
 *
 * Provides basic implementation for reading events from bucket.
 */
class BucketReader : public virtual sV::aux::iEventSequence {
public:
    typedef sV::aux::iEventSequence Parent;
    typedef typename sV::AnalysisPipeline::Event Event;
private:
    events::Bucket * _cBucket;
    ConstBucketIterator _it;
protected:
    virtual bool _V_is_good() override;
    virtual void _V_next_event( Event *& ) override;
    virtual Event * _V_initialize_reading() override;
    virtual void _V_finalize_reading() override;
public:
    /// Ctr. Accepts a ptr to reentrant bucket message instance.
    BucketReader( events::Bucket * reentrantBucketPtr );

    /// Returns reference to current bucket.
    events::Bucket & bucket();

    /// (const) Returns reference to current bucket.
    const events::Bucket & bucket() const;

    /// Returns number of events in a bucket.
    virtual size_t n_events() const;

    /// Returns iterator pointing to first event.
    ConstBucketIterator begin() const {
        return ConstBucketIterator( _cBucket );
    }

    /// Returns iterator pointing to the end of events.
    ConstBucketIterator end() const {
        return ConstBucketIterator( _cBucket, bucket().events_size() );
    }

    /// Event getter operator.
    const events::Event & operator[](size_t n) {
        return _cBucket->events(n);
    }
};  // BucketReader


/**@brief A bucket reader with metadata.
 * @class MDatBucketReader
 *
 * This class introduces access to buckets metadata. The purpose is to pack the
 * metadata unpacking code into template interface method.
 */
template<typename MDat>
class MDatBucketReader : public BucketReader {
protected:
    virtual const MDat & _V_metadata() const {
        if( bucket().has_metainfo() ) {
            emraise( notFound, "Bucket carries no metadata." );
        }
        return bucket().metainfo();
    }
public:
    virtual const MDat & metadata() const {
        return _V_metadata();
    }
};


# if 0
/// Manages compressed buckets.
class CompressedBucketsStreamReader : public BucketsStreamReader {
private:
    /// Reentrant compressed bucket instance.
    events::DeflatedBucket * _cDefltdBucket;
    /// Internal cache of used decompressors.
    std::unordered_map<iDecompressor::CompressionAlgo, iDecompressor *> _decompressors;
};  // class CompressedBucketsStreamReader


/// Steeres buckets file reading.
class BucketsFileReader : public CompressedBucketsStreamReader,
                          public sV::AbstractApplication::ASCII_Entry {
private:
    std::vector<goo::filesystem::Path> _filenames;
    std::ifstream _file;
    size_t _nEventsMax;
    PBarParameters * _pbParameters;  ///< set to nullptr when unused
public:
    BucketsFileReader( const std::list<goo::filesystem::Path> & filenames,
                       size_t maxEvents=0,
                       bool enableProgressbar=false );
    virtual ~BucketsFileReader();
};  // class BucketsFileReader
# endif

}  // namespace ::sV::buckets

}  // namespace sV

# endif  // RPC_PROTOCOLS

# endif  // H_STROMA_V_SVBP_READER_H

