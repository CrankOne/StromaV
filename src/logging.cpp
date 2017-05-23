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

# include "logging.hpp"

# ifdef STANDALONE_BUILD
# include <cstdlib>
# include <iostream>
# include <goo_ansi_escseq.h>
# endif

# include <TGText.h>
# include <TGCommandPlugin.h>
# include <TGTextView.h>
# include <TGFont.h>
# include <TGClient.h>
# include <TGResourcePool.h>

# include <goo_utility.hpp>

# include <regex>

namespace sV {
namespace aux {

struct MorozoffCommandPl : public TGCommandPlugin {
    TGTextView * text_view_ptr() { return fStatus; }
    void text_view_ptr( TGTextView * ntvPtr ) { fStatus = ntvPtr; }
};

# if 0
struct ASCIIEscSeqFilteringTGTextView : public TGTextView {
public:
    //new TGTextView(this, 10, 100, 1);
    ASCIIEscSeqFilteringTGTextView( const TGWindow *parent = 0, UInt_t w = 1, UInt_t h = 1, Int_t id = -1,
                                    UInt_t sboptions = 0, Pixel_t back = GetWhitePixel() ) :
            TGTextView( parent, w, h, id, sboptions, back ) {}

    /// As TGCommandPlugin uses only this method, we override only him:
    virtual Bool_t LoadFile(const char *fname, long startpos = 0, long length = -1) override;
};  // struct ASCIIEscSeqFilteringTGTextView
# endif

static const char _static_ASCIIEscSeqRemovingRxStr[] = R"((\x9B|\x1B\[)[0-?]*[ -\/]*[@-~])";
static const std::regex _static_ASCIIEscSeqRemovingRx(_static_ASCIIEscSeqRemovingRxStr);

StreamBuffer::StreamBuffer() /*: _cmdlPluginPtr(nullptr)*/ {
}

StreamBuffer::~StreamBuffer() {
}


static std::string
strip_ASCII_sequences( const std::string & origString ) {
    std::string strippedColorStr;
    std::regex_replace(std::back_inserter(strippedColorStr),
                       origString.begin(), origString.end(),
                       sV::aux::_static_ASCIIEscSeqRemovingRx, "" );
    return strippedColorStr;
}

int
StreamBuffer::sync() {
    std::string origString = this->str();
    this->str("");  // clear it immediately
    std::string strippedColorStr = strip_ASCII_sequences( origString );

    //std::cout << "colored: " << origString
    //          << ", stripped: " << strippedColorStr
    //          ;  // XXX ^^^

    for(auto ostreamPair : _targets) {
        if( ostreamPair.second ) {
            *ostreamPair.first << origString;
        } else {
            *ostreamPair.first << strippedColorStr;
        }
        ostreamPair.first->flush();
    }

    # if 0
    if( _cmdlPluginPtr ) {  // when ROOT widget is bound produce output for it.
        // One need to keep here the content remaining after last
        // newline character. When next output will come to sync(),
        // we need to prepend output with it.
        size_t lastNewlinePos = strippedColorStr.rfind('\n');
        if( lastNewlinePos == std::string::npos ) {
            // newline character not found; append cmdl-buffer and thats it.
            _cmdlBuffer += strippedColorStr;
        } else {
            // newline character found; do the things
            _cmdlBuffer += std::string( strippedColorStr.begin() + lastNewlinePos,
                                        strippedColorStr.end() );
            strippedColorStr.erase( strippedColorStr.begin() + lastNewlinePos,
                                    strippedColorStr.end() );
            TGText tgt( strippedColorStr.c_str() );

            // TODO: monkey-patch!
            reinterpret_cast<MorozoffCommandPl *>( _cmdlPluginPtr)
                        ->text_view_ptr()
                        ->AddText( &tgt );
        }
    }
    # endif

    return 0;
}

void
StreamBuffer::add_target( std::ostream * streamPtr, bool enableColoring ) {
    _targets.push_back( DECLTYPE(_targets)::value_type(streamPtr, enableColoring) );
}

void
set_font_of_TGCommandPlugin( TGCommandPlugin * plPtr, const std::string & fontIDStr ) {
    const TGFont * font = (TGClient::Instance())->GetFont( fontIDStr.c_str() );
    if( !font ) {
        font = (TGClient::Instance())->GetResourcePool()->GetDefaultFont();
    }
    FontStruct_t labelfont = font->GetFontStruct();
    reinterpret_cast<MorozoffCommandPl *>( plPtr )
                        ->text_view_ptr()
                        ->SetFont( labelfont );
}

//
// Logger
////////

Logger::~Logger() {
    if( own_stream() ) {
        delete _loggingStream;
    }
}

std::ostream &
Logger::own_log_stream() const {
    if( !_loggingStream ) {
        _loggingStream = new std::stringstream();
    }
    return *_loggingStream;
}

void
Logger::log_message( LogLevel lvl, const char * fmt, ... ) const {
    int final_n, n = strlen(fmt);
    if( log_level() < lvl ) {
        return;
    }

    va_list ap;
    while( 1 ) {
        strcpy( _bf, fmt );
        va_start( ap, fmt );
        final_n = vsnprintf( _bf, sizeof( _bf ), fmt, ap );
        va_end(ap);
        if( final_n < 0 || final_n >= sizeof(_bf) ) {
            n += abs(final_n - n + 1);
        } else {
            break;
        }
    }

    //va_list ap;
    //va_start( ap, fmt );
    //snprintf( _bf, sizeof(_bf), fmt, ap );
    //va_end( ap );

    own_log_stream() << _bf;
}

}  // namespace aux
}  // namespace sV

# if 0
/** \note: currently the guerilla patch affects nothing. It seems, ROOT
 * collects somewhere the reference to old TGTextView and keeps using it.
 * We need to find a way to override the "plugin" completely.
 */
void
StreamBuffer::bind_cmdl_plugin( TGCommandPlugin * iPtr, bool doMonkeyPatch ) {
    _cmdlPluginPtr = iPtr;
    if( doMonkeyPatch ) {
        // Do guerilla patching substituting our ASCII-filtering TGTextView
        // implementation here:
        auto plPtr = reinterpret_cast<MorozoffCommandPl *>( _cmdlPluginPtr );
        auto oldOne = plPtr->text_view_ptr();
        auto newOne = new ASCIIEscSeqFilteringTGTextView(
                    _cmdlPluginPtr,
                    oldOne->GetWidth(), oldOne->GetHeight(),
                    oldOne->GetId()
                );
        plPtr->text_view_ptr( newOne );
        delete oldOne;  // causes segfault
    }
}

Bool_t
ASCIIEscSeqFilteringTGTextView::LoadFile( const char * fname, long startpos, long length ) {
    FILE *fp;
    if (!(fp = fopen(fname, "r"))) {
        return kFALSE;
    }
    fclose(fp);
 
    ShowTop();
    Clear();
    fText->Load(fname, startpos, length);

    // Now, get the damn string:
    std::string str( strip_ASCII_sequences(fText->AsString().Data()) );
    fText->Clear();
    fText->LoadBuffer( str.c_str() );

    Update();
    return kTRUE;
}

//}  // namespace aux
//}  // namespace sV

# ifdef STANDALONE_BUILD
int
main(int argc, char * argv[]) {
    {
        std::string beforeStrip = "Some " ESC_BLDGREEN "text" ESC_BLDRED " to demonstrate coloring" ESC_CLRCLEAR ".";
        std::cout << "  Before: " << beforeStrip << std::endl
                  << "   After: ";
        std::regex_replace(std::ostreambuf_iterator<char>(std::cout),
                            beforeStrip.begin(), beforeStrip.end(),
                            sV::aux::_static_ASCIIEscSeqRemovingRx, "" );
        std::cout << std::endl;
    }
    {
        sV::aux::StreamBuffer customBuffer;

        customBuffer.add_target( &(std::cout), false );

        std::ostream os( &customBuffer );
        os << ESC_CLRGREEN "Green" ESC_CLRCLEAR " " ESC_CLRBLUE   "blue"   ESC_CLRCLEAR << std::endl
           << ESC_CLRRED   "Red" ESC_CLRCLEAR   " " ESC_CLRYELLOW "yellow" ESC_CLRCLEAR << std::endl
           ;
    }
    return EXIT_SUCCESS;
}
# endif
# endif
