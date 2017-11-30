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

# include "analysis/pipeline/basic.tcc"

# include <string>

# define BOOST_TEST_NO_MAIN
//# define BOOST_TEST_MODULE Data source with metadata
# include <boost/test/unit_test.hpp>

namespace sV {
namespace test {

// Prerequisites.
// We define flags array as a sequence of messages that will be emitted by our
// testing source:
static const uint8_t msgSkip = 0x1
                   , msgAbort = 0x1 << 1
                   , msgFrbdn = 0x1 << 2
                   ;

struct Message {
    int id;
    uint8_t flags[4];
} gSrcMsgs[] = {
    // Test entire chain propagation:
    { 1, { 0x0,                 0x0,                0x0,                0x0      } },
    // Test skipping:
    { 2, { 0x0,                 0x0,                0x0,                msgSkip  } },
    { 3, { 0x0,                 0x0,                msgSkip,            msgFrbdn } },
    { 4, { msgSkip,             msgFrbdn,           msgFrbdn,           msgFrbdn } },
    // Test abortion:
    { 5, { 0x0,                 msgAbort,           msgFrbdn,           msgFrbdn } },
    { 6, { msgFrbdn,            msgFrbdn,           msgFrbdn,           msgFrbdn } },
    //
    { 0, { msgFrbdn,            msgFrbdn,           msgFrbdn,           msgFrbdn } },
};

int pIDS[][6] = {
    {1,  2,  3,  4,  5,  1},
    {1,  2,  3,      5,  1, -1},
    {1,  2,  3,      1, -1, -1},
    {1,  2,          1, -1, -1, -1},
};

// The processor type forwards the according bit flags as its processing
// result.

class Processor {
private:
    int _pID;
    std::vector<int> _idsHistory;
public:
    Processor( int pID ) : _pID(pID) {}
    int operator()(Message & msg) {
        _idsHistory.push_back( msg.id );
        BOOST_CHECK( ! (msg.flags[_pID] & msgFrbdn) );
        return msg.flags[_pID];
    }
    const std::vector<int> & ids_history() const { return _idsHistory; }
};


// Define pipeline type from template:

typedef sV::PipelineTraits< Message
                          , Processor
                          , int
                          , int > Traits;


// Define arbiter class:

class TestingArbiter : public Traits::IArbiter {
protected:
    size_t _nMsg;
    bool _skipNext, _abortProcessing;
protected:
    virtual bool _V_consider_handler_result( int rc ) override {
        BOOST_CHECK( ! (rc & msgFrbdn) );
        BOOST_CHECK( ! _skipNext );
        BOOST_CHECK( ! _abortProcessing );
        if( rc & msgSkip ) {
            _skipNext = true;
        }
        if( rc & msgAbort ) {
            _abortProcessing = true;
        }
        return !(_skipNext | _abortProcessing);
    }
    virtual int _V_pop_result() override {
        int res = (int) _nMsg;
        _nMsg = 0;
        _skipNext = _abortProcessing = false;
        return res;
    }
    virtual bool _V_next_message() override {
        _skipNext = false;
        if(!_abortProcessing) {
            ++_nMsg;
            return true;  // continue
        } else {
            return false;  // abort
        }
    }
public:
    TestingArbiter() : _nMsg(0), _skipNext(false), _abortProcessing(false) {}
};


// Define source wrapper simply iterating over gSrcMsgs array

class TestingSource : public Traits::ISource {
private:
    Message * _latest;
public:
    TestingSource() : _latest(gSrcMsgs) {}
    virtual Message * next() override {
        if( _latest->id ) {
            return _latest++;
        }
        return nullptr;
    }
    void reset() { _latest = gSrcMsgs; }
};

}  // namespace test
}  // namespace sV

//
// Test suite

BOOST_AUTO_TEST_SUITE( Pipeline_suite )

BOOST_AUTO_TEST_CASE( LinearPipelineTC ) {
    sV::test::Processor p1(0), p2(1), p3(2), p4(3);

    sV::test::TestingArbiter ta;
    sV::Pipeline<sV::test::Traits> ppl( &ta );

    ppl.push_back( &p1 );
    ppl.push_back( &p2 );
    ppl.push_back( &p3 );
    ppl.push_back( &p4 );

    sV::test::TestingSource src;

    int n = ppl.process(src);
    BOOST_CHECK( 4 == n );  // shall be aborted on #4
    n = ppl.process( sV::test::gSrcMsgs[0] );
    BOOST_CHECK( 0 == n );

    BOOST_TEST_MESSAGE( "    ...basic pipeline processing passed." );

    int idx1 = 0, idx2 = 0;
    for( const auto & h : ppl ) {
        for( auto nEv : h.processor().ids_history() ) {
            BOOST_CHECK( sV::test::pIDS[idx1][idx2] == nEv );
            ++idx2;
        }
        idx2 = 0;
        ++idx1;
    }
}

BOOST_AUTO_TEST_SUITE_END()

