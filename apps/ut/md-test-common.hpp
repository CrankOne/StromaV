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

# ifndef H_STROMA_V_UT_METADATA_COMMON_AUX_ROUTINES_H
# define H_STROMA_V_UT_METADATA_COMMON_AUX_ROUTINES_H

# include <string>
# include <utility>
# include <vector>
# include <cassert>
# include "event.pb.h"
# include "metadata/traits.tcc"

namespace sV {
namespace test {

std::vector< std::pair<size_t, size_t> >
extract_words_positions( const char * const s );

void
copy_word_to_event( const std::string & word,
                    sV::events::Event & event );

std::string
get_word_from_event( const sV::events::Event & );

// Aux class keeping words obtained using metadata
class ExtractedWords : public aux::iEventSequence {
private:
    std::list<std::string> _words;
    std::list<std::string>::const_iterator _it;
    Event _rE;
    bool _isGood;
protected:
    virtual bool _V_is_good() override { return _isGood; }
    virtual void _V_next_event( Event *& eventPtrRef ) override {
        if( _it != _words.end() ) {
            copy_word_to_event( *_it, *eventPtrRef );
            ++_it;
        } else {
            _isGood = false;
            eventPtrRef = nullptr;
        }
    }
    /// Has to return a pointer to a valid event. Can invoke _V_next_event()
    /// internally.
    virtual Event * _V_initialize_reading() override {
        Event * _evPtr = &_rE;
        _it = _words.begin();
        _isGood = true;
        _V_next_event( _evPtr );
        return _evPtr;
    }
    virtual void _V_finalize_reading() override {
        _it = _words.end();
        _isGood = false;
    }
public:
    ExtractedWords() : iEventSequence(0x0) {}
    ExtractedWords( const std::list<std::string> & words_ ) :
                                iEventSequence(0x0),
                                _words(words_) {}
    void push_back_word( const std::string & w ) {
        _words.push_back( w );
    }
};

}  // namespace test
}  // namespace sV

# endif  // H_STROMA_V_UT_METADATA_COMMON_AUX_ROUTINES_H

