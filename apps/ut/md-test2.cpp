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

# include "analysis/evSource_sectional.tcc"
# include "md-test-common.hpp"

namespace sV {
namespace mdTest2 {

// (Einsturzende Neubauten, "Nagorny Karabach", 2007)
// Note: pity, but std::alnum() does not recognize unicode ä, ö, ü symbols
// here. Since this text is just for testing I won't fix the code but
// yet sorry, my german friends.
const char _static_srcEN[][1024] = {
R"(Die Stadt liegt unter Nebel
Ich bin auf meinem Berg
In meinem schwarzen Garten
zwischen Himmeln eingeklemmt
in der Enklave meiner Wahl
in der ich mich versteck
in Nagorny Karabach

Vormals tiefe Wälder
Bergketten, vielleicht Eis
eine messinggelbe Sonne
verbricht ein Paradies
meine Sys- und Diastole
dazwischen der Moment
getragen von den Vögeln
die hier zugange sind
in der Enklave meines Herzens
in der ich mich verlier
in Nagorny Karabach)",

R"(Ich steig den Berg herunter
geh ins eine oder andere Tal
es ist geflaggt in allen Farben
in Bergisch-Karabach

Zwei grosse schwarze Raben
fressen die Pflaumen aus dem Baum
Ob die andre Stadt mich lieb hat...?)",

R"(In der Enklave meiner Wahl
in der ich mich verberg'
in Nagorny Karabach

Komm mich mal besuchen
ich hab' unendlich Zeit
und der Blick der ist vom Feinsten
über Wolken und die Stadt
in Nagorny Karabach
Nagorny Karabach
Nagorny Karabach)"
};

//
// Metadata types

// A second "EventID" is somewhat more complicated --- it is location of the
// word identifying fragment, number of stanza inside fragment, line number
// inside stanza, and the word number inside it.
struct EventID {
    unsigned char stanzaNo, lineNo;
    size_t wordNo;

    bool operator==(const EventID & eid) const {
        return ( eid.stanzaNo == stanzaNo )
            && ( eid.lineNo == lineNo )
            && ( eid.wordNo == wordNo )
            ;
    }
};
std::ostream & operator<<(std::ostream & os, const EventID & eid ) {
    os << "{s#" << (int) eid.stanzaNo
       << ", l#" << (int) eid.lineNo
       << ", w#" << eid.wordNo
       << "}";
    return os;
}
// Corresponding metadata type has to denote the word with its start and
// length.
struct MetadataEntry {
    EventID eid;
    size_t offset, length;
};
// The metadata for a fragment is just a list here (but might be a DB handle).
class Metadata : public std::list<MetadataEntry> {
public:
    const MetadataEntry & query_word_loc( const EventID & eid ) const {
        for( const auto & it : *this ) {
            //std::cout << " XXX " << it.eid << " =?= " << eid << std::endl;
            if( it.eid == eid ) {
                return it;
            }
        }
        emraise( noSuchKey, "Word with given ID not found in this metadata." );
    }
};

namespace auxTest2 {

void
extract_test2_metadata( Metadata & md, const char * const s ) {
    MetadataEntry me = {{ 0, 0, 0 }, 0, 0};
    bool doesIterateWord = false,
         done = false;
    std::cout << "[0-0] ";
    for( const char * c = s; !done; c++ ) {
        if( std::isalnum(*c) ) {
            if( !doesIterateWord ) {
                doesIterateWord = true;
                me.offset = c - s;
            }
        } else {
            if( doesIterateWord ) {
                me.length = (c - s) - me.offset;
                ++me.eid.wordNo;
                md.push_back( me );
                doesIterateWord = false;
                std::cout << std::string( s + me.offset, me.length )
                          << "(" << me.eid.wordNo << ") ";
            }
            if( '\n' == *c ) {
                ++me.eid.lineNo;
                me.eid.wordNo = 0;
                if( '\n' == *(c-1) ) {
                    me.eid.lineNo = 0;
                    ++me.eid.stanzaNo;
                }
                std::cout << std::endl
                          << "[" << (int) me.eid.stanzaNo
                          << "-" << (int) me.eid.lineNo
                          << "]" << " ";
            }
        }
        done = ('\0' == *c);
    }
    std::cout << std::endl;
}

}  // namespace aux

// Considering source identifier as a just pointer to fragment, let us define
// the trats type:
typedef sV::MetadataTypeTraits<EventID, Metadata, uint8_t> MetadataTraits;

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
    // Metadata access for RA source
    virtual Event * _V_md_event_read_single(
                            const Metadata & md,
                            const EventID & wordID ) override {
        auto mde = md.query_word_loc( wordID );
        std::string word( _content + mde.offset, mde.length );
        test::copy_word_to_event( word, _rE );
        return &_rE;
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

    virtual const char * _V_textual_id(const SourceID *sidPtr) const override {
        static const char fn[][32] = { "<first>", "<second>", "<third>" };
        if( !sidPtr ) {
            return "<null>";
        } else {
            return fn[*sidPtr - 1];
        }
    }
public:
    DataSource( SourceID sid, const char * const c ) :
                aux::iEventSequence( aux::iEventSequence::randomAccess
                                   | aux::iEventSequence::identifiable ),
                MetadataTraits::iEventSource( sid ), //< will init own dict
                _content(c) {
                    _words = test::extract_words_positions( _content );
                    _it = _words.begin();
                }
    DataSource( SourceID sid, const char * const c,
                MetadataTraits::MetadataTypesDictionary & mdDictRef ) :
                    aux::iEventSequence( aux::iEventSequence::randomAccess
                                       | aux::iEventSequence::identifiable ),
                    MetadataTraits::iEventSource(sid, mdDictRef),
                    _content(c) {
                        _words = test::extract_words_positions( _content );
                        _it = _words.begin();
                    }
    const char * const content() const { return _content; }
};  // DataSource

// - metadata type implementation describing necessary routines of how
//   metadata has to be applied:
class MetadataType : public MetadataTraits::iSpecificMetadataType {
protected:
    //
    // Metadata handling
    virtual bool _V_is_complete( const SpecificMetadata & ) const override {
        // Note: this method is reserved for further additions and extensions
        // made in metadata data type.
        return true;
    }

    virtual bool _V_extract_metadata(
                            const SourceID * sidPtr,
                            DataSource & ds,
                            SpecificMetadata *& mdPtrRef,
                            std::list<MetadataStore *> stores) const override {
        if( !sidPtr || !*sidPtr ) {
            emraise( badState, "Fragment number is not set when metadata "
                "acquisition routine invoked." );
        }
        if( !mdPtrRef ) {
            mdPtrRef = new Metadata();
        }
        auxTest2::extract_test2_metadata(
                            *mdPtrRef,
                            static_cast<mdTest2::DataSource&>(ds).content() );
        return true;
    }

    virtual SpecificMetadata * _V_merge_metadata(
                    const std::list<SpecificMetadata *> & ) const override {
        _TODO_  // TODO
    }
    
    virtual bool _V_append_metadata( DataSource & s,
                                     SpecificMetadata & md ) const override {
        // Since our testing metadata instances are always complete, there
        // is no possibility to get here.
        _FORBIDDEN_CALL_;
    }

    virtual void _V_cache_metadata(
                            const SourceID &,
                            const SpecificMetadata &,
                            std::list<MetadataStore *> & ) const override {
        _TODO_  // TODO
    }
public:
    MetadataType() : MetadataTraits::iSpecificMetadataType("Testing2") {}
};  // MetadataType

}  // namespace mdTest2
}  // namespace sV

//
// Test suite

# define BOOST_TEST_NO_MAIN
//# define BOOST_TEST_MODULE Data source with metadata
# include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE( Metadata_suite )

BOOST_AUTO_TEST_CASE( SectionalSource,
            * boost::unit_test::depends_on("Metadata_suite/BatchSource") ) {
    using namespace sV::mdTest2;
    using namespace sV::test;

    //Metadata md;
    //auxTest2::extract_test2_metadata( md, _static_srcEN[0] );

    // This part has may be run at user code at somewhat "init" section:
    MetadataTraits::MetadataTypesDictionary dict;
    MetadataType mdt;
    dict.register_metadata_type( mdt );
    DataSource * s[3] = {
            new DataSource( 1, _static_srcEN[0], dict ),
            new DataSource( 2, _static_srcEN[1], dict ),
            new DataSource( 3, _static_srcEN[2], dict )
        };

    {
        // First of all, let's test look-up capabilities for one word that
        // definetely located at the provided source:
        EventID eid { /* Stanza no ............... */ 4,
                      /* Line no in stanza ....... */ 1,
                      /* Word no in line ......... */ 3 };
        const std::string expectedWord = "Pflaumen",
                          obtainedWord = get_word_from_event(
                                    *s[1]->event_read_single( eid ) );
        BOOST_REQUIRE( expectedWord == obtainedWord );
    }
    {
        // This case is special --- upon now or routines has to utilize
        // cached metadata information for fragment. TODO: automatic check
        // that upon this time we do not re-caching metadata.
        EventID eid { /* Stanza no ............... */ 4,
                      /* Line no in stanza ....... */ 1,
                      /* Word no in line ......... */ 6 };
        const std::string expectedWord = "Baum",
                          obtainedWord = get_word_from_event(
                                    *s[1]->event_read_single( eid ) );
        BOOST_REQUIRE( expectedWord == obtainedWord );
    }
    {
        // Now we force two data sources to acquire metadata in
        // order to make them cached in the store.
        s[1]->metadata();
        s[2]->metadata();
    }
    // Clear sources.
    delete s[0]; s[0] = nullptr;
    delete s[1]; s[1] = nullptr;
    delete s[2]; s[2] = nullptr;

    // Now we have metadata indexing two of three fragments available in store
    // and can gain access it by using interim batch events ource representing
    // all the indexed sources. Let's check that acquizition may be performed
    // as if it is a continious array.
    auto & mdt2 = static_cast<const MetadataType &>(
                                    dict.metadata_type( "Testing2" ));
    BOOST_REQUIRE( &mdt == &mdt2 );
    auto & batchHandle = mdt2.batch_handle();
    /*TODO: src = */batchHandle.event_read_range( {2, 0, 0}, {3, 0, 0} );

    // Finaly, we make the first fragment's metadata to be cached by explicit
    // acquizition of the words:
    // TODO
    // and by obtaining the data using batch source (check, whether sectional
    // and virtual event sources can co-exist):
    // TODO
}

BOOST_AUTO_TEST_SUITE_END()

