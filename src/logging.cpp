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
# else
# include "app/abstract.hpp"
# endif

# include <TGText.h>
# include <TGCommandPlugin.h>
# include <TGTextView.h>
# include <TGFont.h>
# include <TGClient.h>
# include <TGResourcePool.h>

# include <goo_utility.hpp>
# include <goo_ansi_escseq.h>
# include <goo_utility.h>

# include <regex>
# include <iostream>

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
}  // namespace ::sV::aux



namespace logging {

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
            reinterpret_cast<::sV::aux::MorozoffCommandPl *>( _cmdlPluginPtr)
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
    reinterpret_cast<::sV::aux::MorozoffCommandPl *>( plPtr )
                        ->text_view_ptr()
                        ->SetFont( labelfont );
}


//
// iLoggingFamily
///////////////

std::unordered_map<std::string, iLoggingFamily *> * iLoggingFamily::_families = nullptr;

iLoggingFamily::iLoggingFamily( const std::string & nm, LogLevel l ) : _name(nm), _lvl(l) {
}

void
iLoggingFamily::log_level( LogLevel l ) {
    _lvl = l;
}

iLoggingFamily &
iLoggingFamily::get_instance( const std::string & nm ) {
    if( !_families ) {
        _families = new std::unordered_map<std::string, iLoggingFamily *>();
    }
    auto it = _families->find( nm );
    if( _families->end() == it ) {
        if( !AbstractApplication::exists() ) {
            sV_logw( "Unable to acquire \"class\" parameter for logging "
                    "family \"%s\" since app. instance wasn't initialized "
                    "up to this time, so \"Common\" class will be set.\n",
                    nm.c_str() );
            it = _families->emplace( nm, generic_new<iLoggingFamily>(
                    "Common" ) ).first;
        } else {
            it = _families->emplace( nm, generic_new<iLoggingFamily>(
                    goo::app<AbstractApplication>().cfg_option<std::string>(
                            "logging.sections." + nm + "class" ) ) ).first;
        }
    }
    return *(it->second);
}

//
// CommonLoggingFamily
/////////////////////

CommonLoggingFamily::CommonLoggingFamily(
                const goo::dict::Dictionary & dct,
                LogLevel l ) :
        iLoggingFamily( "Common", l ) {
    bzero( _streamPtrs, sizeof(_streamPtrs) );
}

std::ostream &
CommonLoggingFamily::stream_for( LogLevel l ) {
    int8_t n = l + 2;
    if( _streamPtrs[n] ) {
        return *_streamPtrs[n];
    } else if( l < 0 ) {
        return std::cerr;
    } else {
        return std::cerr;
    }
}

// Default prefixes.
static const char _static_logPrefixes[][64] = {
    "[%7s" ESC_BLDRED "EE" ESC_CLRCLEAR "] %s: ",
    "[%7s" ESC_BLDYELLOW "WW" ESC_CLRCLEAR "] %s: ",
    "[%7s" ESC_BLDBLUE "L1" ESC_CLRCLEAR "] %s: ",
    "[%7s" ESC_CLRCYAN "L2" ESC_CLRCLEAR "] %s: ",
    "[%7s" ESC_CLRBLUE "L3" ESC_CLRCLEAR "] %s: ",
    "[%7s" ESC_CLRCYAN "??" ESC_CLRCLEAR "] %s: ",
};

std::string
CommonLoggingFamily::get_prefix_for_loglevel( LogLevel l ) const {
    const char * fmt;
    char prefixBf[128];
    switch(l) {
        case      error: fmt = _static_logPrefixes[0]; break;
        case    warning: fmt = _static_logPrefixes[1]; break;
        case      quiet: fmt = _static_logPrefixes[2]; break;
        case    laconic: fmt = _static_logPrefixes[3]; break;
        case loquacious: fmt = _static_logPrefixes[4]; break;
        default: fmt = _static_logPrefixes[5];
    };
    snprintf( prefixBf, sizeof(prefixBf), fmt, hctime(), family_name().c_str() );
    return prefixBf;
}

StromaV_LOGGING_CLASS_DEFINE( CommonLoggingFamily, "Common" ) {
    return goo::dict::Dictionary( NULL, "Common logging family class." );
}

//
// Logger
////////

Logger::Logger( const std::string & familyName,
                const std::string & prfx ) :
                        _family( iLoggingFamily::get_instance( familyName ) ) {
    _prefix=prfx;
    // TODO: process $(this)/$(PID)/whatever...
}

Logger::~Logger() {}

void
Logger::log_msg(    LogLevel lvl,
                    const char * fmt, ... ) const {
    if( log_level() < lvl ) {
        return;
    }
    int final_n, n = strlen(fmt);
    va_list ap;
    while( 1 ) {
        strcpy( _bf, fmt );
        va_start( ap, fmt );
            final_n = vsnprintf( _bf, sizeof( _bf ), fmt, ap );
        va_end(ap);
        if( final_n < 0 || final_n >= (int) sizeof(_bf) ) {
            n += abs(final_n - n + 1);
        } else {
            break;
        }
    }
    char bf[GOO_EMERGENCY_BUFLEN+32];
    snprintf( bf, sizeof(bf), "%s %s:%s",
        log_family().get_prefix_for_loglevel(lvl).c_str(), _prefix.c_str(), _bf );
    const_cast<iLoggingFamily &>(log_family())
        .stream_for(lvl) << bf;
}

}  // namespace ::sV::logging
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

void
Logger::message( LogLevel level, const std::string & msg, bool noprefix ) {
    if( level <= log_level() ) {
        char prefixBf[128];
        const char emptyPrefix[] = "";
        if( !noprefix ) {
            snprintf( prefixBf, sizeof(prefixBf),
                      get_prefix_for_loglevel(level), hctime() );
        }
        const char * prefix = noprefix ? emptyPrefix : prefixBf;
        std::ostream & os = ostream_for( level );
        os << prefix << msg;
        os.flush();
    }
}
# endif
