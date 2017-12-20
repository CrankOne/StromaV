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

# ifndef H_STROMA_V_SVBP_READER_BUCKET_READER_H
# define H_STROMA_V_SVBP_READER_BUCKET_READER_H

# include "sV_config.h"

# ifdef RPC_PROTOCOLS

# include "analysis/pipeline.hpp"

# include <goo_mixins/iterable.tcc>

namespace sV {
namespace buckets {

/**@brief Performs basic reading from events bucket.
 * @class BucketReader
 *
 * Provides basic implementation for reading events from bucket. This class
 * keeps track of single bucket associated with current instance and implements
 * basic iEventSequence interface for iterating it within sV's events pipeline
 * framework.
 *
 * This rudimentary implementation makes no assumption about bucket contents
 * and does not involve usage of any supplementary information within. For
 * supp. info handling reader, see SuppInfoBucketReader.
 *
 * @ingroup analysis
 * @ingroup buckets
 *
 * @see SuppInfoBucketReader
 */
class BucketReader : public virtual sV::aux::iEventSequence {
public:
    /// Parent interface class.
    typedef sV::aux::iEventSequence Parent;
    /// Unified events type.
    typedef typename mixins::PBEventApp::UniEvent Event;
    /// Bucket event id. Used for custom iterator class.
    template<typename BucketT>
    struct BucketEventID {
        BucketT * authorityPtr;
        size_t nEvent;
        BucketEventID( BucketT * bPtr, size_t nEve=0 ) :
                                            authorityPtr(bPtr), nEvent(nEve) {}
        BucketEventID & operator++() { ++nEvent; return *this; }
    };
    /// Template declaration for i/o iterator related to bucket reader.
    template<typename EventT, typename BucketT>
    class _BucketIterator : public goo::iterators::Iterator<
                                        goo::iterators::BaseBidirIterator,
                                        goo::iterators::BaseOutputIterator,
                                        goo::iterators::DirectionComparableIterator,
                                        _BucketIterator<EventT, BucketT>,
                                        BucketEventID<BucketT>, EventT, size_t> {
    private:
        BucketEventID<BucketT> _sym;
    public:
        _BucketIterator( BucketT * bPtr, size_t nEve=0 ) :
                    _sym( bPtr, nEve ) {}

        BucketEventID<BucketT> & sym() { return _sym; }
        const BucketEventID<BucketT> & sym() const { return _sym; }
    };
    /// Reading iterator.
    typedef _BucketIterator<const events::Event *, const events::Bucket>
                                                            ConstBucketIterator;
private:
    /// Reading bucket instance pointer.
    events::Bucket * _cBucket;
    /// Current iterator for reading associated bucket instance.
    mutable ConstBucketIterator _it;
protected:
    /// Reads true if iterator points to existing event instance inside a bucket.
    virtual bool _V_is_good() override;
    /// Reads next event using internal iterator.
    virtual void _V_next_event( Event *& ) override;
    /// Resets internal iterator and read first event returning pointer to it.
    virtual Event * _V_initialize_reading() override;
    /// Sets internal iterator to the end of associated buffer.
    virtual void _V_finalize_reading() override;
    /// This immutable bucket getter may be overriden by user classes that may
    /// implement some caching procedures.
    virtual const events::Bucket & _V_bucket() const;
    /// Mutable bucket getter. Just returns mutable bucket pointer.
    events::Bucket * _mutable_bucket_ptr() { return _cBucket; }
public:
    /// Ctr. Accepts a ptr to reentrant bucket message instance.
    BucketReader( events::Bucket * bucketPtr );
    /// (const) Returns reference to current bucket.
    const events::Bucket & bucket() const { return _V_bucket(); }
    /// Returns number of events in a bucket.
    virtual size_t n_events() const;
    /// Returns true when bucket pointer was associated with this handle.
    bool is_bucket_set() const { return _cBucket; }
    /// Sets bucket pointer to be treated with this handle.
    virtual void set_bucket_ptr( events::Bucket * ptr ) { _cBucket = ptr; }
    /// Returns iterator pointing to first event.
    ConstBucketIterator begin() const
        { return ConstBucketIterator( &(bucket()) ); }
    /// Returns iterator pointing to the end of events.
    ConstBucketIterator end() const
        { return ConstBucketIterator( &(bucket()), bucket().events_size() ); }
    /// Event getter operator.
    const events::Event & operator[]( size_t n ) { return bucket().events(n); }
    /// Wil set internal bucket iterator to beginning.
    void reset_bucket_iterator() const;
};  // BucketReader

}  // namespace ::sV::buckets
}  // namespace sV

# endif  // RPC_PROTOCOLS

# endif  // H_STROMA_V_SVBP_READER_BUCKET_READER_H

