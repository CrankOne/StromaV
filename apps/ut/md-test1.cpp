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

# include "metadata/traits.tcc"
# include "analysis/evSource_batch.tcc"

namespace sV {

namespace test {

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
const char _static_srcThomas[] = R"(
Do not go gentle into that good night,
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

std::vector< std::pair<size_t, size_t> >
extract_words_positions( const char * const s ) {
    const char * lastBgn = nullptr;
    std::vector< std::pair<size_t, size_t> > r;
    for( const char * c = s; *c != '\0'; ++c ) {
        if( std::isalnum(*c) ) {
            if( !lastBgn ) {
                lastBgn = c;
            }
        } else {
            if( lastBgn ) {
                std::pair<size_t, size_t> p( c - s, c - lastBgn );
                //std::cout << "XXX '" << std::string( lastBgn, c )
                //          << "'" << std::endl;
                r.push_back( p );
                lastBgn = nullptr;
            }
        }
    }
    return r;
}

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
    const char * const _content;
protected:
    virtual bool _V_md_event_read_single(
                            const Metadata & md,
                            const EventID & ) override {
        _TODO_  // TODO
    }

    virtual std::unique_ptr<aux::iEventSequence> _V_md_event_read_range(
                            const Metadata & md,
                            const EventID & lower,
                            const EventID & upper ) override {
        _TODO_  // TODO
    }
    virtual std::unique_ptr<aux::iEventSequence> _V_md_event_read_list(
                            const Metadata & md,
                            const std::list<EventID> & eidsList ) override {
        _TODO_  // TODO
    }
public:
    DataSource( const char * const c ) : _content(c) {}
    DataSource( const char * const c,
                MetadataTraits::MetadataTypesDictionary & mdDictRef ) :
                                    MetadataTraits::iEventSource(mdDictRef),
                                    _content(c) {}
    const char * const content() const { return _content; }
};  // DataSource

// - metadata type implementation describing necessary routines of how
//   metadata has to be applied:
class MetadataType : public MetadataTraits::iSpecificMetadataType {
protected:
    SpecificMetadata & _V_acquire_metadata(
                            MetadataTraits::iEventSource & s ) const override;
public:
    MetadataType() : MetadataTraits::iSpecificMetadataType("Testing1") {}
};  // MetadataType


static Metadata _static_Metadata;

// Implementation of metadata getting method.
MetadataType::SpecificMetadata &
MetadataType::_V_acquire_metadata( MetadataTraits::iEventSource & s_ ) const  {
    ::sV::test::DataSource & s = dynamic_cast<::sV::test::DataSource &>(s_);
    size_t wordNo = 0;
    Metadata & md = _static_Metadata;
    auto poss = extract_words_positions( s.content() );
    for( auto pIt = poss.begin(); poss.end() != pIt; ++pIt, ++wordNo ) {
        md.emplace( wordNo, *pIt );
    }
    return md;
}

}  // namespace test
}  // namespace sV

//
// Test suite

# define BOOST_TEST_NO_MAIN
//# define BOOST_TEST_MODULE Data source with metadata
# include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE( Data_source_suite )

BOOST_AUTO_TEST_CASE( InitialValidity ) {
    // Run extraction routine expecting no side effects
    BOOST_REQUIRE( !sV::test::extract_words_positions(
                                        sV::test::_static_srcHaiku ).empty());
    # if 0
    // This part has may be run at user code at somewhat "init" section:
    sV::test::MetadataTraits::MetadataTypesDictionary dict;
    sV::test::MetadataType mdt;
    dict.register_metadata_type( mdt );
    sV::test::DataSource s( sV::test::_static_srcThomas, dict );
    else
    sV::test::MetadataType mdt;
    sV::test::DataSource s( sV::test::_static_srcThomas );
    s.metadata_types_dict().register_metadata_type( mdt );
    # endif
    //sV::test::Metadata md = s.acquire_metadata(); // ?

    # if 0
    sV::test::MetadataDict tstDict;
    sV::test::TstEventSource1 src1;
    //TstEventSource2 src2;
    BOOST_REQUIRE( !src1.is_good() );
    BOOST_REQUIRE( src1.initialize_reading() );
    BOOST_REQUIRE( src1.is_good() );
    # endif
}

BOOST_AUTO_TEST_SUITE_END()

