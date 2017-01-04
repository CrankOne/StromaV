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
namespace test {

// (Einsturzende Neubauten, "Nagorny Karabach", 2007)
const char _static_srcPoe[][512] = {
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
    unsigned char fragmentNo;
    unsigned char stanzaNo, lineNo;
    size_t wordNo;

    bool operator==(const EventID & eid) const {
        return ( eid.fragmentNo == fragmentNo )
            && ( eid.stanzaNo == stanzaNo )
            && ( eid.lineNo == lineNo )
            && ( eid.wordNo == wordNo )
            ;
    }
};
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
            if( it.eid == eid ) {
                return it;
            }
        }
        emraise( noSuchKey, "Word with given ID not found in fragment." );
    }
};

// Considering source identifier as a just pointer to fragment, let us define
// the trats type:
typedef sV::MetadataTypeTraits<EventID, Metadata, const char *> MetadataTraits;

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
            copy_word_to_event( word, *eventPtrRef );
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
                            const EventID & wordID ) override {
        auto mde = md.query_word_loc( wordID );
        std::string word( _content + mde.offset, mde.length );
        copy_word_to_event( word, _rE );
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
public:
    DataSource( const char * const c ) :
                aux::iEventSequence( aux::iEventSequence::randomAccess
                                   | aux::iEventSequence::identifiable ),
                MetadataTraits::iEventSource(), //< will init own dict inside
                _content(c) {
                    _words = extract_words_positions( _content );
                    _it = _words.begin();
                }
    DataSource( const char * const c,
                MetadataTraits::MetadataTypesDictionary & mdDictRef ) :
                    aux::iEventSequence( aux::iEventSequence::randomAccess
                                       | aux::iEventSequence::identifiable ),
                    MetadataTraits::iEventSource(mdDictRef),
                    _content(c) {
                        _TODO_  // TODO
                    }
    const char * const content() const { return _content; }
};  // DataSource

// - metadata type implementation describing necessary routines of how
//   metadata has to be applied:
class MetadataType : public MetadataTraits::iSpecificMetadataType {
protected:
    //virtual SpecificMetadata & _V_acquire_metadata_for(
    //                        MetadataTraits::iEventSource & s ) const override;
public:
    MetadataType() : MetadataTraits::iSpecificMetadataType("Testing2") {}
};  // MetadataType

}  // namespace test
}  // namespace sV

