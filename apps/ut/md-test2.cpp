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
# include "metadata/type_cached.tcc"
# include "md-test-common.hpp"

namespace sV {
namespace mdTest2 {

// Local log-streaming source for unit (supp. dev info):
static std::ostream & los = std::cout;

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

    bool operator>(const EventID & eid) const {
        return (
                this->stanzaNo > eid.stanzaNo
             || (this->stanzaNo == eid.stanzaNo &&
                    (this->lineNo > eid.lineNo
                 || ( this->lineNo == eid.lineNo &&
                        this->wordNo > eid.wordNo ))
                )
            );
    }

    bool operator<(const EventID & eid) const {
        return (
                this->stanzaNo < eid.stanzaNo
             || (this->stanzaNo == eid.stanzaNo &&
                    (this->lineNo < eid.lineNo
                 || ( this->lineNo == eid.lineNo &&
                        this->wordNo < eid.wordNo ))
                )
            );
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
class Test2Metadata : public std::list<MetadataEntry> {
public:
    // IMPORTANT NOTE:
    // Raising the exception is not a best way to report about the fact
    // that particular event can not be found at this metadata since one may
    // need to consider this situation as notmal while querying metadata
    // instances inside of a store object(s).
    const MetadataEntry & query_word_loc( const EventID & eid ) const {
        if( contains(eid) ) {
            for( const auto & it : *this ) {
                los << "[==] " << it.eid << " =?= " << eid << std::endl;
                if( it.eid == eid ) {
                    return it;
                }
            }
        }
        emraise( noSuchKey, "Word with given ID not found in this metadata." );
    }

    // aux method returning true when event is owned by the source.
    bool contains( const EventID & eid ) const {
        const EventID first = front().eid,
                      last = back().eid;
        return (first.stanzaNo <= eid.stanzaNo && last.stanzaNo >= eid.stanzaNo )
            && (first.lineNo <= eid.lineNo && last.lineNo >= eid.lineNo )
            && (first.wordNo <= eid.wordNo && last.wordNo >= eid.wordNo )
            ;
    }
};

bool operator==(const MetadataEntry & mde, const EventID & eid) {
    return mde.eid == eid;
}

namespace auxTest2 {

void
extract_test2_metadata( Test2Metadata & md,
                        const char * const s,
                        unsigned char initialStanzaNo ) {
    MetadataEntry me = {{ initialStanzaNo, 0, 0 }, 0, 0};
    bool doesIterateWord = false,
         done = false;
    los << "[==] [" << (int) initialStanzaNo << "-0] ";
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
                los << std::string( s + me.offset, me.length )
                    << "(" << me.eid.wordNo << ") ";
            }
            if( '\n' == *c ) {
                ++me.eid.lineNo;
                me.eid.wordNo = 0;
                if( '\n' == *(c-1) ) {
                    me.eid.lineNo = 0;
                    ++me.eid.stanzaNo;
                }
                los << std::endl << "[==] "
                    << "[" << (int) me.eid.stanzaNo
                    << "-" << (int) me.eid.lineNo
                    << "]" << " ";
            }
        }
        done = ('\0' == *c);
    }
    los << std::endl;
}

}  // namespace aux

// Considering source identifier as a just pointer to fragment, let us define
// the trats type:
typedef sV::MetadataTypeTraits<EventID, Test2Metadata, uint8_t> MetadataTraits;

//
// Defining the routines:

// This object has to be available from various locations and normally is
// placed inside singleton.
MetadataTraits::MetadataTypesDictionary * _static_MTDPtr = nullptr;

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
                            const Test2Metadata & md,
                            const EventID & wordID ) override {
        auto mde = md.query_word_loc( wordID );
        std::string word( _content + mde.offset, mde.length );
        test::copy_word_to_event( word, _rE );
        return &_rE;
    }

    virtual std::unique_ptr<aux::iEventSequence> _V_md_event_read_range(
                            const Test2Metadata & md,
                            const EventID & lower,
                            const EventID & upper ) override {
        auto mdeFromIt = std::find( md.begin(), md.end(), lower ),
               mdeToIt = std::find( md.begin(), md.end(), upper ),
                     c = mdeFromIt
                       ;
        if( mdeFromIt == md.end() || mdeToIt == md.end() ) {
            std::cerr << " |Requested from: " << lower << std::endl
                      << " |            to: " << upper << std::endl
                      << " |fragent begins: " << md.front().eid << std::endl
                      << " | fragment ends: " << md.back().eid << std::endl
                      ;
            emraise( uTestFailure, "Wrong event ID for source. Previous "
                "routines found incorrect source:." )
        }
        ++mdeToIt;
        std::list<std::string> words;
        do {
            words.push_back( std::string( _content + c->offset, c->length ) );
        } while( ++c != mdeToIt );
        return std::unique_ptr<sV::aux::iEventSequence>(
                                        new test::ExtractedWords( words ));
    }
    virtual std::unique_ptr<aux::iEventSequence> _V_md_event_read_list(
                            const Test2Metadata & md,
                            const std::list<EventID> & eidsList ) override {
        # if 1
        _TODO_  // TODO
        # else
        std::list<std::string> words;
        for( const auto & nw : eidsList ) {
            auto wp = md.find(nw)->second;
            words.push_back( std::string(_content + wp.first, wp.second) );
        }
        return std::unique_ptr<sV::aux::iEventSequence>(
                                        new test::ExtractedWords( words ));
        # endif
    }

    virtual std::string _V_textual_id(const SourceID *sidPtr) const override {
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
class MetadataType : public MetadataTraits::iMetadataType {
public:
    sV_METADATA_IMPORT_SECT_TRAITS( EventID, Test2Metadata, SourceID );
protected:
    //
    // Metadata handling
    virtual bool _V_is_complete( const Test2Metadata & ) const override {
        // Note: this method is reserved for further additions and extensions
        // made in metadata data type.
        return true;
    }

    virtual bool _V_extract_metadata(
                            const SourceID * sidPtr,
                            MetadataTraits::iEventSource & ds,
                            Test2Metadata *& mdPtrRef,
                            std::list<iMetadataStore *> stores) const override {
        if( !sidPtr || !*sidPtr ) {
            emraise( badState, "Fragment number is not set when metadata "
                "acquisition routine invoked." );
        }

        // Source ID usually carries external information that has to be used
        // for proper identifiaction of additional information of metadata
        // that can not be obtained from provided section or fragment. In this
        // testing case we have to obtain initial stanza number. The easiest
        // way is to define it manually since this mechanism is not related to
        // routines we're currently testing.
        static unsigned char _static_initialStanzaNos[] = {1, 3, 5};
        unsigned char initialStanzaNo = _static_initialStanzaNos[*sidPtr-1];

        if( !mdPtrRef ) {
            mdPtrRef = new Test2Metadata();
        }
        auxTest2::extract_test2_metadata(
                            *mdPtrRef,
                            static_cast<mdTest2::DataSource&>(ds).content(),
                            initialStanzaNo );
        return true;
    }

    virtual Test2Metadata * _V_merge_metadata(
                    const std::list<Test2Metadata *> & ) const override {
        _TODO_  // TODO
    }
    
    virtual bool _V_append_metadata( MetadataTraits::iEventSource & s,
                                     Test2Metadata & md ) const override {
        // Since our testing metadata instances are always complete, there
        // is no possibility to get here.
        _FORBIDDEN_CALL_;
    }

    virtual void _V_cache_metadata(
                        const SourceID & sid,
                        const Test2Metadata & mdRef,
                        std::list<iMetadataStore *> & stores_ ) const override {
        stores_.front()->put_metadata( sid, mdRef );
    }

    virtual void _V_get_subrange(
                    const EventID & low, const EventID & up,
                    const SourceID & sid,
                    typename Traits::SubrangeMarkup & muRef ) const override {
        _TODO_  // TODO
    }
public:
    MetadataType() : MetadataTraits::iMetadataType("Testing2") {}
};  // class MetadataType


class Store : public MetadataTraits::iDisposableSourceManager,
              public MetadataTraits::iEventQueryableStore,
              public MetadataTraits::iRangeQueryableStore {
public:
    sV_METADATA_IMPORT_SECT_TRAITS( mdTest2::EventID,
                                    mdTest2::Test2Metadata,
                                    uint8_t );
private:
    std::map<SourceID, Test2Metadata *> _mdatCache;
public:
    virtual Test2Metadata * get_metadata_for(
                                        const SourceID & sid) const override {
        auto it = _mdatCache.find(sid);
        if( _mdatCache.end() == it ) {
            return nullptr;
        }
        return it->second;
    }

    virtual void put_metadata( const SourceID & sid,
                                  const Test2Metadata & mdRef ) override {
        // well, proper way would be: allocate new metadata object and copy it,
        // but who cares? Anywhay, metadata instances will has to be cleaned
        // somewhere and stores is a best place to do it in this UT.
        _mdatCache[sid] = const_cast<Test2Metadata *>(&mdRef);
    }

    virtual iEventSource * source( const SourceID & sid ) override {
        return new DataSource( sid, _static_srcEN[sid-1], *_static_MTDPtr );
    }

    virtual void free_source( iEventSource * srcPtr ) override {
        delete srcPtr;
    }

    virtual bool source_id_for( const EventID & eid,
                                      SourceID & sid) const override {
        MetadataEntry mde = {{0, 0, 0}, 0, 0};
        SourceID foundSID;
        for( const auto & p : _mdatCache ) {
            Test2Metadata & md = *(p.second);
            // See note at the Test2Metadata::query_word_loc() implem.
            try {
                mde = md.query_word_loc( eid );
                foundSID = p.first;
            } catch( goo::Exception & e ) {
                if( goo::Exception::noSuchKey != e.code() ) {
                    throw;
                }
            }
        }
        if( !(mde.eid == eid) ) {
            // Normal way to report that current store instance was unable to
            // locate word.
            return false;
        }
        sid = foundSID;
        return true;
    }
    void erase_metadata_for( const SourceID & sid ) override {
        _TODO_  // TODO
    }

    void collect_source_ids_for_range(
                                const EventID & from,
                                const EventID & to,
                                std::list<SubrangeMarkup> & output ) const override {
        if( _mdatCache.empty() ) {
            emraise( uTestFailure, "Empty metadata store." );
        }
        // TODO: fix --- all sources that does not contain neither "from"
        // neither "to" will be omitted.
        for( const auto & mdePair : _mdatCache ) {
            if( mdePair.second->contains( from ) ) {
                output.push_back( MetadataTraits::SubrangeMarkup {
                    {0, 0, 0},
                    {0, 0, 0},
                    mdePair.first,
                    nullptr, nullptr } ); // < Note, that we hadn't set md
                                          // here to check run-time md
                                          // acquizition.
                los << "[==] set for begin." << std::endl;
            } else if( mdePair.second->contains( to ) ) {
                output.push_back( MetadataTraits::SubrangeMarkup {
                    {0, 0, 0},
                    {0, 0, 0},
                    mdePair.first,
                    mdePair.second, nullptr } ); // < Here, md ptr is set.
                los << "[==] set for end." << std::endl;
            }
        }
        for( SubrangeMarkup & entry : output ) {
            bool hasStarted = false,
                 hasEnded = false;
            // Retreive metadata for source:
            const Test2Metadata * mdPtr = _mdatCache.find( entry.sid )->second;
            // Retreive range for mdPtr:
            // * Since its merely a list just iterate it until either end of a
            //   list, or end of a super-range will be reached to find end.
            // * Iteration has to start from beginning.
            EventID foundFrom, foundTo;
            for( const auto & mde : *mdPtr ) {
                if( !hasStarted ) {
                    // Check start condition:
                    if( !( mde.eid < from) ) {
                        foundFrom = mde.eid;
                        hasStarted = true;
                        los << "[==] starts: " << mde.eid
                            << "..." << std::endl;
                    }
                } else {
                    if( !( mde.eid < to) ) {
                        foundTo = mde.eid;
                        hasEnded = true;
                        los << "[==] ...ends: " << mde.eid << std::endl;
                        break;  // Done
                    }
                }
            }
            if( hasStarted && !hasEnded ) {
                foundTo = mdPtr->back().eid;
                hasEnded = true;
                los << "[==] ...ends: " << mdPtr->back().eid << std::endl;
            } /*else if( hasEnded ) {
                break;
            }*/
            if( !hasStarted || !hasEnded ) {
                emraise( uTestFailure, "Range integrity failed: begin %s, end %s.",
                    ( hasStarted ? "found" : "not found" ),
                    ( hasEnded ? "found" : "not found" ) );
            }
            entry.from = foundFrom;
            entry.to = foundTo;
        }
    }
public:
    Store() {}
};  // class Store

}  // namespace mdTest2
}  // namespace sV

//
// Test suite

# define BOOST_TEST_NO_MAIN
//# define BOOST_TEST_MODULE Data source with metadata
# include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE( Metadata_suite )

BOOST_AUTO_TEST_CASE( SectionalSource,
            * boost::unit_test::depends_on("Metadata_suite/BulkSource") ) {
    using namespace sV::mdTest2;
    using namespace sV::test;

    //Metadata md;
    //auxTest2::extract_test2_metadata( md, _static_srcEN[0], 0 );

    // This part has may be run at user code at somewhat "init" section:
    MetadataTraits::MetadataTypesDictionary dict;
    _static_MTDPtr = &dict;
    MetadataType mdt;
    dict.register_metadata_type( mdt );

    Store store;
    mdt.add_store( store );

    DataSource * s[3] = {
            new DataSource( 1, _static_srcEN[0], dict ),
            new DataSource( 2, _static_srcEN[1], dict ),
            new DataSource( 3, _static_srcEN[2], dict )
        };

    BOOST_TEST_MESSAGE( "[==] Dict, type, store and sources constructed and "
        "seems being ready to operate..." );

    {
        // First, let's test look-up capabilities for one word that
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
        // that at this time we do not re-caching metadata?
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
    BOOST_TEST_MESSAGE( "[==] By-event selection tests passed. Freeing "
                        "sources..." );
    // Clear sources.
    delete s[0]; s[0] = nullptr;
    delete s[1]; s[1] = nullptr;
    delete s[2]; s[2] = nullptr;

    BOOST_TEST_MESSAGE( "[==] ...initial sources cleared. "
        "Testing operations with "
        "sectional data..." );

    // Now we have metadata indexing two of three fragments available in store
    // and can gain access it by using interim batch events source representing
    // all the indexed sources.
    auto & mdt2 = static_cast<const MetadataType &>(
                                    dict.metadata_type( "Testing2" ));
    BOOST_REQUIRE( &mdt == &mdt2 );
    auto & batchHandle = mdt2.batch_handle();

    // Try to obtain a word using this batch abstraction:
    BOOST_REQUIRE( get_word_from_event(
                *batchHandle.event_read_single({3, 1, 6}) ) == "Tal" );

    // Let's check that acquizition may be performed as if it is a continious
    // array.
    {
        const std::string & expectedSequence = "die Pflaumen aus dem Baum"
                                            " Ob die andre Stadt mich lieb hat"
                                            " In der Enklave meiner Wahl"
                                            " in der "
                                            ;
        auto src = batchHandle.event_read_range( {4, 1, 2}, {5, 1, 2} );
        std::stringstream ss;
        for( auto eventPtr = src->initialize_reading();
             src->is_good(); src->next_event(eventPtr) ) {
            ss << get_word_from_event(*eventPtr) << " ";
        }
        //std::cout << "'" << expectedSequence << "'" << std::endl
        //          << "'" << ss.str() << "'" << std::endl;
        BOOST_REQUIRE( ss.str() == expectedSequence );
    }
    //std::cout << ">>>" << std::endl;

    // Finaly, we make the first fragment's metadata to be cached by explicit
    // acquizition of the words:
    // TODO
    // and by obtaining the data using batch source (check, whether sectional
    // and virtual event sources can co-exist):
    // TODO
}

BOOST_AUTO_TEST_SUITE_END()

