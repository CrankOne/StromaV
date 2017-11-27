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

# include <goo_exception.hpp>
# include <strings.h>
# include "app/ascii_display.hpp"

namespace sV {
namespace aux {

// Display::ASCII_Entry
////////////////

ASCII_Display::ASCII_Entry::ASCII_Entry( ASCII_Display * displayPtr
                                       , NLines nLines ) :
                Parent::Consumer( *displayPtr ) {
    if( owner() ) {
        static_cast<ASCII_Display *>(owner())->_register_me( this, nLines );
    }
}

ASCII_Display::ASCII_Entry::~ASCII_Entry( ) {
    if( owner() ) {
        static_cast<ASCII_Display *>(owner())->_erase_me( this );
    }
}


// Display
/////////

void
ASCII_Display::_register_me( const ASCII_Entry * entryPtr, NLines nLines ) {
    char ** lines = (char**) malloc( (nLines+1)*sizeof(char*) );
    {
        char ** cLine;
        for( cLine = lines; lines + nLines != cLine; ++cLine ) {
            *cLine = (char*) malloc( LineLength*sizeof(char) );
            bzero( *cLine, LineLength*sizeof(char) );
        }
        *cLine = NULL;
    }
    auto insertionResult = _entries.emplace( entryPtr, lines );
    if( !insertionResult.second ) {
        emraise( badArchitect, "Repeated insertion of entry %p.", entryPtr );
    }
}

void
ASCII_Display::_erase_me( const ASCII_Entry * entryPtr ) {
    auto it = _entries.find( entryPtr );
    if( _entries.end() == it ) {
        emraise( badArchitect, "Couldn't find the entry %p.", entryPtr );
    }
    _entries.erase( it );
}

char **
ASCII_Display::get_buffer_of( const ASCII_Entry * entryPtr ) {
    if( !is_ready() ) {
        // Indicates bad style.
        emraise( badState, "Can not allow premature display printing buffer "
            "acquizition for entry %p. It is suposed by design (to avoid "
            "performance issues): one need acquire buffer only when display is "
            "ready.", entryPtr
        );
    }
    auto it = _entries.find( entryPtr );
    if( _entries.end() == it ) {
        emraise( badArchitect, "Couldn't find the entry %p.", entryPtr );
    }
    return it->second;
}

void
ASCII_Display::_V_sr_use( ASCIIDisplayParameters & ) {
    size_t overallLineNo = 0;
    for( auto it = _entries.begin(); _entries.end() != it; ++it ) {
        for( char * const * line = it->second; *line; ++line, ++overallLineNo ) {
            printf( "\033[1K%s\n", *line );
        }
    }
    printf( "\033[%zuA", overallLineNo );
}

ASCII_Display::ASCII_Display( bool enabled ) :
                Parent( *(new ASCIIDisplayParameters) ),
                _enabled( enabled ) {
}

ASCII_Display::~ASCII_Display() {
    delete &(this->parameters());
}

void
ASCII_Display::enable_ASCII_display() {
    _enabled = true;
}

void
ASCII_Display::disable_ASCII_display() {
    _enabled = false;
}

}  // namespace aux
}  // namespace sV

