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

# include "analysis/evSource_bulk.tcc"
# include "md-test-common.hpp"

namespace sV {
namespace mdTest1 {

//
// Some data for tests.

// Well-known potic texts is convinient to use. We well define a metadata
// struct containing number of words in line and line positions in text array.
// Almost all of them has irregular rythmics and blank lines allowing app to
// test MD for robustness.

// Matsuo Bashō
const char _static_srcHaiku[] = 
R"(From time to time
The clouds give rest
To the moon-beholders.)";

// (Dylan Thomas, "Do not go gently into that good night", 1947)
const char _static_srcThomas[] =
R"(Do not go gentle into that good night,
Old age should burn and rave at close of day;
Rage, rage against the dying of the light.

Though wise men at their end know dark is right,
Because their words had forked no lightning they
Do not go gentle into that good night.

Good men, the last wave by, crying how bright
Their frail deeds might have danced in a green bay,
Rage, rage against the dying of the light.

Wild men who caught and sang the sun in flight,
And learn, too late, they grieved it on its way,
Do not go gentle into that good night.

Grave men, near death, who see with blinding sight
Blind eyes could blaze like meteors and be gay,
Rage, rage against the dying of the light.

And you, my father, there on the sad height,
Curse, bless, me now with your fierce tears, I pray.
Do not go gentle into that good night.
Rage, rage against the dying of the light.)";

//
// Metadata types

// A first "EventID" type describes word position just by the order.
typedef size_t EventID;
// Corresponding metadata type is just an index [word no] -> [wordBgn wordEnd].
typedef std::unordered_map<EventID, std::pair<size_t, size_t> > Metadata;
// Since we have only one source instances, we do not need the sourceID type
// and can here define the traits type:
typedef sV::MetadataTypeTraits<EventID, Metadata> MetadataTraits;

//
// Defining the routines:

// - data stream supporting metadata indexing:
class DataSource : public MetadataTraits::iEventSource {
private:
    /// Since our data has relatively small size in this testing unit, we can
    /// just keep all of it in RAM.
    const char * const _content;
    /// This helper vector will be used upon iterating.
    std::vector< std::pair<size_t, size_t> > _words;
    /// This internal iterator refers to "last event".
    DECLTYPE(_words)::const_iterator _it;
    bool _isGood;
    /// Reentrant event object. Most of the real data sources has it.
    Event _rE;
protected:
    //
    // Common event sequence interface
    // (we do not use it)
    virtual bool _V_is_good() override {
        return _isGood;
    }

    virtual void _V_next_event( Event *& eventPtrRef ) override {
        if( _it != _words.end() ) {
            std::string word( _content + _it->first, _it->second );
            test::copy_word_to_event( word, *eventPtrRef );
            ++_it;
        } else {
            _isGood = false;
        }
    }

    virtual Event * _V_initialize_reading() override {
        _it = _words.begin();
        Event * evPtr = &_rE;
        _isGood = true;
        _V_next_event( evPtr );
        return evPtr;
    }
    virtual void _V_finalize_reading() override {
        _it = _words.end();
        _isGood = false;
    }

    //
    // Metadata access
    virtual Event * _V_md_event_read_single(
                            const Metadata & md,
                            const EventID & wordNo ) override {
        auto it = md.find( wordNo );
        if( md.end() == it ) {
            emraise( noSuchKey, "Has no word #%zu.", wordNo );
        }
        std::pair<size_t, size_t> wp = it->second;
        std::string word( _content + wp.first, wp.second );
        test::copy_word_to_event( word, _rE );
        return &_rE;
    }

    virtual std::unique_ptr<aux::iEventSequence> _V_md_event_read_range(
                            const Metadata & md,
                            const EventID & lower,
                            const EventID & upper ) override {
        std::list<std::string> words;
        for( size_t i = lower; !(i > upper); ++i ) {
            auto wp = md.find(i)->second;
            words.push_back( std::string(_content + wp.first, wp.second) );
        }
        return std::unique_ptr<sV::aux::iEventSequence>(
                                        new test::ExtractedWords( words ));
    }
    virtual std::unique_ptr<aux::iEventSequence> _V_md_event_read_list(
                            const Metadata & md,
                            const std::list<EventID> & eidsList ) override {
        std::list<std::string> words;
        for( const auto & nw : eidsList ) {
            auto wp = md.find(nw)->second;
            words.push_back( std::string(_content + wp.first, wp.second) );
        }
        return std::unique_ptr<sV::aux::iEventSequence>(
                                        new test::ExtractedWords( words ));
    }
public:
    DataSource( const char * const c ) :
                aux::iEventSequence( aux::iEventSequence::randomAccess ),
                MetadataTraits::iEventSource(), //< will init own dict inside
                _content(c) {
                _words = test::extract_words_positions( _content );
                    _it = _words.begin();
                }
    DataSource( const char * const c,
                MetadataTraits::TypesDictionary & mdDictRef ) :
                    aux::iEventSequence( aux::iEventSequence::randomAccess ),
                    MetadataTraits::iEventSource(mdDictRef),
                    _content(c) {
                    _words = test::extract_words_positions( _content );
                        _it = _words.begin();
                    }
    const char * content() const { return _content; }
};  // DataSource

// - metadata type implementation describing necessary routines of how
//   metadata has to be applied:
class MetadataType : public MetadataTraits::iMetadataType {
protected:
    SpecificMetadata & _V_acquire_metadata(
                            MetadataTraits::iEventSource & s ) override;
public:
    MetadataType() : MetadataTraits::iMetadataType("Testing1") {}
};  // MetadataType

// Implementation of metadata getting method.
MetadataType::SpecificMetadata &
MetadataType::_V_acquire_metadata( MetadataTraits::iEventSource & s_ ) {
    ::sV::mdTest1::DataSource & s = dynamic_cast<::sV::mdTest1::DataSource &>(s_);
    size_t wordNo = 0;
    Metadata & md = *(new Metadata);
    // ^^^ It's ok here. Will be automatically cleaned. TODO: add to docs.
    auto poss = test::extract_words_positions( s.content() );
    for( auto pIt = poss.begin(); poss.end() != pIt; ++pIt, ++wordNo ) {
        md.emplace( wordNo, *pIt );
    }
    return md;
}

}  // namespace mdTest1
}  // namespace sV

//
// Test suite

# define BOOST_TEST_NO_MAIN
//# define BOOST_TEST_MODULE Data source with metadata
# include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE( Metadata_suite )

BOOST_AUTO_TEST_CASE( BulkSource ) {
    using namespace sV::mdTest1;
    using namespace sV::test;

    // Run extraction routine expecting no side effects
    BOOST_REQUIRE( !extract_words_positions( _static_srcHaiku ).empty());

    MetadataType mdt;
    DataSource s( _static_srcThomas );
    s.metadata_types_dict().register_metadata_type( mdt );

    //for( sV::events::Event * eventPtr = s.initialize_reading();
    //     s.is_good();
    //     s.next_event( eventPtr ) ) {
    //    std::cout << get_word_from_event( *eventPtr ) << "-";
    //}
    //std::cout << std::endl;

    size_t nWords[] = { 0, 1, 2, 102, 4, 167 };
    const unsigned short nWordsLen = sizeof(nWords)/sizeof(size_t);
    const char expectedOutput[][32] = {
        "Do", "not", "go", "gentle", "into",
        "light"  //< we have to check accessibility of the last word
    };

    // Basic, event-by-event reading:
    for( unsigned short i = 0; i < nWordsLen; ++i ) {
        //std::cout << "Expected: '" << expectedOutput[i] << "' got: '"
        //          << get_word_from_event(
        //                                    *s.event_read_single( nWords[i] ) )
        //          << "'." << std::endl;
        BOOST_REQUIRE(std::string(expectedOutput[i]) ==
            get_word_from_event( *s.event_read_single( nWords[i] )));
    }

    // Read range
    {
        std::string expectedPhrase( _static_srcThomas, 37 ),
                    obtainedPhrase;
        //std::cout << phrase << std::endl;
        auto seqPtr = s.event_read_range( 0, 7 );
        for( sV::events::Event * eventPtr = seqPtr->initialize_reading();
             seqPtr->is_good(); seqPtr->next_event(eventPtr) ) {
            obtainedPhrase += get_word_from_event( *eventPtr ) + " ";
        }
        obtainedPhrase = obtainedPhrase.substr(0, obtainedPhrase.size()-1);
        //std::cout << "#1 '" << expectedPhrase << "' =?= '"
        //                    << obtainedPhrase << "'" << std::endl;
        BOOST_REQUIRE( obtainedPhrase == expectedPhrase );
    }

    // Read set
    {
        std::list<EventID> eids;
        std::string expectedPhrase, obtainedPhrase;
        for( unsigned short i = 0; i < nWordsLen; ++i ) {
            unsigned short idx = nWordsLen - i - 1;
            eids.push_back( nWords[idx] );
            expectedPhrase += expectedOutput[idx];
            expectedPhrase += " ";
        }
        auto seqPtr = s.event_read_list( eids );
        for( sV::events::Event * eventPtr = seqPtr->initialize_reading();
             seqPtr->is_good(); seqPtr->next_event(eventPtr) ) {
            obtainedPhrase += get_word_from_event( *eventPtr ) + " ";
        }
        //std::cout << "#2 '" << expectedPhrase << "' =?= '"
        //                    << obtainedPhrase << "'" << std::endl;
        BOOST_REQUIRE( obtainedPhrase == expectedPhrase );
    }
    BOOST_TEST_MESSAGE( "[==] Bulk source test done." );
}

BOOST_AUTO_TEST_SUITE_END()

