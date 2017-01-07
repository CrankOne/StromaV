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

# include "analysis/evSource_RA.tcc"
# include "md-test-common.hpp"

namespace sV {
namespace test {

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
                std::pair<size_t, size_t> p( lastBgn - s, c - lastBgn );
                # if 0  // dev dbg
                std::cout << "XXX '" << std::string( lastBgn, c )
                          << "' => '" << std::string(s + p.first, p.second)
                          << "'" << std::endl;
                # endif
                r.push_back( p );
                lastBgn = nullptr;
            }
        }
    }
    return r;
}

void
copy_word_to_event( const std::string & word,
                    sV::events::Event & event ) {
    assert( !word.empty() );
    event.set_blob( word );
}

std::string
get_word_from_event( const sV::events::Event & event ) {
    return event.blob();
}

}  // namespace test
}  // namespace sV
