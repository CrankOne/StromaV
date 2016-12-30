/*
 * Copyright (c) 2016 Renat R. Dusaev <crank@qcrypt.org>
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

# include "metadata/dictionary.tcc"
# include "analysis/pipeline.hpp"
# include "analysis/evSource_batch.tcc"

static std::unordered_set<sV_MetadataTypeIndex> * _static_mdTypeIds = nullptr;

sV_MetadataTypeIndex
sV_generate_metadata_type_id( void * _this ) {
    typedef std::remove_pointer<decltype(_static_mdTypeIds)>::type
            IDsContainer;
    if( !_static_mdTypeIds ) {
        _static_mdTypeIds = new IDsContainer();
    }
    IDsContainer & ids = *_static_mdTypeIds;
    sV_MetadataTypeIndex idCandidate;
    uint16_t nSense = USHRT_MAX;
    do {
        idCandidate = double(USHRT_MAX)*(double(rand())/RAND_MAX);
        //XXX
        //std::cout << "New type ID generated:" << idCandidate << std::endl;
        nSense--;
    } while( ids.end() != ids.find(idCandidate) && nSense );
    if( !nSense ) {
        emraise( overflow, "Can not generate unique metadata type ID." );
    }
    return idCandidate;
}



namespace sV {

namespace test {

//
// Define event ID and metadata types dictionary class corresponding for it:
struct TstEventID {
    size_t testingEventIDValue;
};

typedef MetadataDictionary<TstEventID> MetadataDict;

//
// Define couple of metadata structures:
typedef std::pair<size_t, const char *> TstMetadata1;

struct TstMetadata2 {
    size_t n;
    size_t byteOffset;
};

//
// Implement testing metadata types
class TstMetadataType1 : public iMetadataType<TstEventID, TstMetadata1> {
public:
    typedef iMetadataType<TstEventID, TstMetadata1>::DataSource DataSource;
protected:
    TstMetadata1 & _V_acquire_metadata( DataSource & ) const override;
};  // class TstMetadataType1

class TstMetadataType2 : public iMetadataType<TstEventID, TstMetadata2> {
public:
    typedef iMetadataType<TstEventID, TstMetadata2>::DataSource DataSource;
protected:
    TstMetadata2 & _V_acquire_metadata( DataSource & ) const override;
};  // TstMetadataType2

//
// Define corresponding data sources with random access:
class TstEventSource1 : public TstMetadataType1::DataSource {
public:
    static const char evs[6][32];
private:
    const char (* _evIter)[32];
    sV::events::TestingMessage _reentrantPayload;
    Event _reentrantEvent;
protected:
    virtual bool _V_is_good() override {
        return _evIter && ('\0' != (*_evIter)[0]);
    }
    virtual void _V_next_event( Event *& evePtrRef ) override {
        if( !_evIter ) { emraise(badState, "Event iterator not initialized."); }
        evePtrRef = &_reentrantEvent; {
            _reentrantPayload.Clear();
            _reentrantPayload.set_content( *_evIter );
        }
        evePtrRef->mutable_experimental()
                 ->mutable_payload()
                 ->PackFrom( _reentrantPayload );
        ++_evIter;
    }
    virtual Event * _V_initialize_reading() override {
        _evIter = evs;
        Event * evePtr;
        _V_next_event( evePtr );
        return evePtr;
    }
    virtual void _V_finalize_reading() override {
        _evIter = nullptr;
    }

    //
    // Random access:
    //

    virtual bool _V_event_read_single( const EventID & eid ) override {
        // TODO: use metadata here instead of direct reading!
        //_evIter = &(evs[eid.testingEventIDValue]);
        _TODO_  // TODO
    }
    virtual bool _V_event_read_range( const EventID & lower,
                                      const EventID & upper ) override{
        // TODO: source must implement some kind of iteration among this range
        _TODO_  // TODO
        return true;
    }
public:
    TstEventSource1() : TstMetadataType1::DataSource(),
                        _evIter(nullptr) {
    }
};  // class class TstEventSource1
const char TstEventSource1::evs[6][32] = {
    "one",      "two",      "three",
    "four",     "five",     ""  // sentinel
};


class TstEventSource2 : public TstMetadataType2::DataSource {
    virtual bool _V_event_read_single( const EventID & ) override;
    virtual bool _V_event_read_range( const EventID & lower,
                                      const EventID & upper ) override;
};  // class class TstEventSource1

}  // namespace test
}  // namespace sV

//
// Test suite

# define BOOST_TEST_NO_MAIN
//# define BOOST_TEST_MODULE Data source with metadata
# include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE( Data_source_suite )

BOOST_AUTO_TEST_CASE( InitialValidity ) {
    sV::test::MetadataDict tstDict;
    sV::test::TstEventSource1 src1;
    //TstEventSource2 src2;
    BOOST_REQUIRE( !src1.is_good() );
    BOOST_REQUIRE( src1.initialize_reading() );
    BOOST_REQUIRE( src1.is_good() );
}

BOOST_AUTO_TEST_SUITE_END()

