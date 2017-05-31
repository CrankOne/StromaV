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

# include "app/abstract.hpp"

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

# if defined(TEMPLATED_LOGGING) && TEMPLATED_LOGGING
#   include "ctemplate/template.h"
# endif

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

iLoggingFamily::iLoggingFamily( const std::string & nm,
                                LogLevel l ) : _name(nm), _lvl(l) {
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

const char CommonLoggingFamily::templateNames[8][64] = {
            "common-log-tpl-error",  "common-log-tpl-warning",
            "common-log-tpl-log-msg",
            "common-log-tpl-log-msg-1", "common-log-tpl-log-msg-2", "common-log-tpl-log-msg-3"
            "common-log-tpl-debug-msg", "common-log-tpl-other"};

// Default prefixes.
const char CommonLoggingFamily::logPrefixes[8][32] = {
    ESC_BLDRED      "EE" ESC_CLRCLEAR,
    ESC_BLDYELLOW   "WW" ESC_CLRCLEAR,
    ESC_BLDYELLOW   "--" ESC_CLRCLEAR,
    ESC_BLDBLUE     "L1" ESC_CLRCLEAR,
    ESC_CLRCYAN     "L2" ESC_CLRCLEAR,
    ESC_CLRBLUE     "L3" ESC_CLRCLEAR,
    ESC_BLDVIOLET   "DD" ESC_CLRCLEAR,
    ESC_CLRCYAN     "??" ESC_CLRCLEAR,
};

CommonLoggingFamily::CommonLoggingFamily(
                const goo::dict::Dictionary & dct,
                LogLevel l ) :
        iLoggingFamily( "Common", l ) {
    bzero( _streamPtrs, sizeof(_streamPtrs) );
    if(!dct.parameters().empty()) {
        CommonLoggingFamily::_V_configure( dct );
    }
}

void
CommonLoggingFamily::_V_configure( const goo::dict::Dictionary & dct ) {
    # if defined(TEMPLATED_LOGGING) && TEMPLATED_LOGGING
    ctemplate::StringToTemplateCache( "common-log-tpl-error",
        dct["ePrefix"].as<std::string>(),
        ctemplate::DO_NOT_STRIP );
    ctemplate::StringToTemplateCache( "common-log-tpl-warning",
        dct["wPrefix"].as<std::string>(),
        ctemplate::DO_NOT_STRIP );
    ctemplate::StringToTemplateCache( "common-log-tpl-log-msg",
        dct["msgPrefix"].as<std::string>(),
        ctemplate::DO_NOT_STRIP );
    ctemplate::StringToTemplateCache( "common-log-tpl-log-msg-1",
        dct["l1Prefix"].as<std::string>(),
        ctemplate::DO_NOT_STRIP );
    ctemplate::StringToTemplateCache( "common-log-tpl-log-msg-2",
        dct["l2Prefix"].as<std::string>(),
        ctemplate::DO_NOT_STRIP );
    ctemplate::StringToTemplateCache( "common-log-tpl-log-msg-3",
        dct["l3Prefix"].as<std::string>(),
        ctemplate::DO_NOT_STRIP );
    ctemplate::StringToTemplateCache( "common-log-tpl-debug-msg",
        dct["dPrefix"].as<std::string>(),
        ctemplate::DO_NOT_STRIP );
    ctemplate::StringToTemplateCache( "common-log-tpl-other",
        dct["other"].as<std::string>(),
        ctemplate::DO_NOT_STRIP );
    # endif  // defined(TEMPLATED_LOGGING) && TEMPLATED_LOGGING
    set_customized();
}

std::ostream &
CommonLoggingFamily::_V_stream_for( LogLevel l ) {
    int8_t n = l + 2;
    if( _streamPtrs[n] ) {
        return *_streamPtrs[n];
    } else if( l < 0 ) {
        return std::cerr;
    } else {
        return std::cerr;
    }
}

std::string
CommonLoggingFamily::get_prefix_for_loglevel( LogLevel l ) const {
    const char * fmt;
    switch(l) {
        case      error: fmt = logPrefixes[0]; break;
        case    warning: fmt = logPrefixes[1]; break;
        case      quiet: fmt = logPrefixes[2]; break;
        case    laconic: fmt = logPrefixes[3]; break;
        case loquacious: fmt = logPrefixes[4]; break;
        default: fmt = logPrefixes[5];
    };
    return fmt;
}

void
CommonLoggingFamily::_V_message( LogLevel lvl,
                              const std::string & instanceID,
                              const std::string & msg ) const {
    char bf[GOO_EMERGENCY_BUFLEN+32];
    snprintf( bf, sizeof(bf), "%7s/%s/%s %s:%s",
        hctime(),
        get_prefix_for_loglevel(lvl).c_str(),
        family_name().c_str(),
        instanceID.c_str(),
        msg.c_str() );

    const_cast<CommonLoggingFamily *>(this)->stream_for(lvl) << bf;
}

# if defined(TEMPLATED_LOGGING) && TEMPLATED_LOGGING
void
CommonLoggingFamily::_V_message( ctemplate::TemplateDictionary & ldct, LogLevel lvl ) const {
    ldct.SetValue( "hctime",    hctime() );
    ldct.SetValue( "family",    family_name() );
    std::string errorText;
    ctemplate::ExpandTemplate( templateNames[(int) lvl + 2],
                               ctemplate::DO_NOT_STRIP,
                               &ldct,   &errorText );
    const_cast<CommonLoggingFamily *>(this)->stream_for(lvl) << errorText;
}
# endif  //defined(TEMPLATED_LOGGING) && TEMPLATED_LOGGING

StromaV_LOGGING_CLASS_DEFINE_MCONF( CommonLoggingFamily, "Common" ) {
    goo::dict::Dictionary dct( "CommonLogging", "Common logging family class." );
    dct.insertion_proxy()
            .p<std::string>("ePrefix",
                    "Template prefix for error messages.",
                "EE: {{message}}" )
            .p<std::string>("wPrefix",
                    "Template prefix for warning messages.",
                "WW: {{message}}" )
            .p<std::string>("msgPrefix",
                    "Template prefix for user notification messages that can "
                    "not be turned off (i.e. ones which reserved for "
                    "interactive input).",
                "{{message}}" )
            .p<std::string>("l1Prefix",
                    "Template prefix for laconic diagnostic messages.",
                "L1: {{message}}" )
            .p<std::string>("l2Prefix",
                    "Template prefix for verbose diagnostic messages.",
                "L2: {{message}}" )
            .p<std::string>("l3Prefix",
                    "Template prefix for loquacious diagnostic messages.",
                "L3: {{message}}" )
            .p<std::string>("dPrefix",
                    "Template prefix for debugging messages.",
                "DD: {{message}}" )
            .p<std::string>("other",
                    "Template prefix for uncathegorized messages.",
                "??: {{message}}" )
        ;
    goo::dict::DictionaryInjectionMap injM;
        injM( "ePrefix",        "logging.Common.ePrefix" )
            ( "wPrefix",        "logging.Common.wPrefix" )
            ( "msgPrefix",      "logging.Common.msgPrefix" )
            ( "l1Prefix",       "logging.Common.l1Prefix" )
            ( "l2Prefix",       "logging.Common.l2Prefix" )
            ( "l3Prefix",       "logging.Common.l3Prefix" )
            ( "dPrefix",        "logging.Common.dPrefix" )
            ( "other",          "logging.Common.other" )
            ;
    return std::make_pair( dct, injM );
}

//
// Logger
////////

Logger::Logger( const std::string & familyName,
                const std::string & prfx ) :
                        _family( iLoggingFamily::get_instance( familyName ) ) {
    _prefix=prfx;
}

Logger::~Logger() {}

void
Logger::log_msg( LogLevel lvl,
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
    log_family().message( lvl, _prefix, _bf );
}

# if defined(TEMPLATED_LOGGING) && TEMPLATED_LOGGING
void
Logger::log_msg( ctemplate::TemplateDictionary & ldct,
                 LogLevel lvl,
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
    ldct.SetFormattedValue( "this", "%p", this );
    ldct.SetValue( "message", _bf );

    log_family().message( ldct, lvl );
}
# endif

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
