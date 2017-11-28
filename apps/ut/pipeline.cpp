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

# include "analysis/pipe.tcc"

# include <string>

# include <iostream>  // XXX

# define BOOST_TEST_NO_MAIN
//# define BOOST_TEST_MODULE Data source with metadata
# include <boost/test/unit_test.hpp>

namespace sV {
namespace test {

// Prerequisites.
// We define flags array as a sequence of messages that will be emitted by our
// testing source:
static const uint8_t msgSkip = 0x1
                   , msgAbort = 0x2
                   , msgFrbdn = 0x4
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

// The processor type forwards the according bit flags as its processing
// result.

class Processor {
private:
    int _pID;
public:
    Processor( int pID ) : _pID(pID) {}
    int operator()(Message & msg) {
        BOOST_CHECK( ! (msg.flags[_pID] & msgFrbdn) );
        return msg.flags[_pID];
    }
};


// Define pipeline type from template:

typedef sV::Pipeline< Message
                    , Processor
                    , int
                    , int > Pipe;


// Define arbiter class:

class TestingArbiter : public Pipe::IArbiter {
protected:
    size_t _nMsg;
    bool _skipNext, _abortProcessing;
public:
    TestingArbiter() : _nMsg(0), _skipNext(false), _abortProcessing(false) {}

    virtual bool consider_handler_result( int rc ) override {
        BOOST_CHECK( ! (rc & msgFrbdn) );
        BOOST_CHECK( ! _skipNext );
        BOOST_CHECK( ! _abortProcessing );
        if( rc & msgSkip ) {
            _skipNext = true;
        }
        if( rc & msgAbort ) {
            _abortProcessing = true;
        }
        return !_skipNext;
    }
    virtual int pop_result() override {
        int res = (int) _nMsg;
        _nMsg = 0;
        _skipNext = _abortProcessing = false;
        return res;
    }
    virtual bool next_message() override {
        if(!_abortProcessing) {
            ++_nMsg;
            return true;  // continue
        } else {
            _skipNext = false;
            return false;  // abort
        }
    }
};


// Define source wrapper simply iterating over gSrcMsgs array

class TestingSource : public Pipe::ISource {
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
    using sV::test::Pipe;

    sV::test::Processor p1(0), p2(1), p3(2), p4(3);

    sV::test::TestingArbiter ta;
    Pipe ppl( &ta );

    ppl.push_back( Pipe::Handler( &p1 ) );
    ppl.push_back( Pipe::Handler( &p2 ) );
    ppl.push_back( Pipe::Handler( &p3 ) );
    ppl.push_back( Pipe::Handler( &p4 ) );

    sV::test::TestingSource src;

    int n = ppl.process(src);
    BOOST_CHECK( 2 == n );  // shall be aborted on #8
    std::cout << "XXX " << n << std::endl;
    n = ppl.process( sV::test::gSrcMsgs[0] );
    BOOST_CHECK( 0 == n );
    std::cout << "XXX " << n << std::endl;

    BOOST_TEST_MESSAGE( "    ...basic pipeline processing passed." );
}

BOOST_AUTO_TEST_SUITE_END()

