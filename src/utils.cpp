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
# include <wordexp.h>  // XXX: goo_dict/dict.hpp
# include "utils.hpp"

# include <cstdlib>

namespace sV {
namespace aux {

static const std::string
        srxDouble  = "([\\-\\+]?(?:(?:[[:d:]]+\\.[0-9]*)|(?:[0-9]*\\.[0-9]+)|(?:[0-9]+))(?:[eE](?:\\-|\\+)?[0-9]+)?)",
        srxVector_ = "(?:(?:\\s*" + srxDouble + "\\s*,\\s*)+" + srxDouble + "\\s*)",
        srxVector = "(?:\\{" + srxVector_ + "\\})"
        ;

static const std::regex rx(srxVector),
                        rxDouble(srxDouble);

std::list<double>
parse_double_fp_vector( const std::string & s ) {
    //static std::regex rx(srxVector);
    std::list<double> res;
    std::list<std::string> sres;
    
    if(std::regex_match( s, rx )) {
        std::copy( std::sregex_token_iterator(s.begin(), s.end(), rxDouble, 1),
                   std::sregex_token_iterator(),
                   std::back_inserter(sres) );
    } else {
        emraise( malformedArguments,
            "Couldn't parse string \"%s\" as vector of doubles.",
            s.c_str() );
    }

    for( auto l : sres ) {
        res.push_back(strtod(l.c_str(), NULL));
    }

    return res;
}

# ifdef GEANT4_MC_MODEL
G4ThreeVector
parse_g4_three_vector( const std::string & s, double factor ) {
    std::list<double> toCnv = parse_double_fp_vector( s );
    if( 3 != toCnv.size() ) {
        emraise( malformedArguments,
                 "Argument is expected to be a 3-dim vector of real numbers (got \"%s\")!",
                 s.c_str() );
    }
    G4ThreeVector res;
    uint8_t d = 0;
    for( auto cmp : toCnv ) {
        res[d++] = cmp*factor;
    }
    return res;
}
# endif


// TODO: to be replaced by goo's ::goo::dict::Configuration::tokenize_string()
// or ::goo::dict::Configuration::free_tokens()
// when goo/appParameters branch will be merged to goo/master:
size_t
goo_XXX_tokenize_argstring( const std::string & str, char **& argvTokens ) {
    ::wordexp_t pwords;
    int rc = wordexp( str.c_str(), &pwords, WRDE_UNDEF | WRDE_SHOWERR | WRDE_NOCMD );
    if( rc ) {
        if( WRDE_BADCHAR == rc) {
            emraise( badParameter, "Bad character met." );
        } else if( WRDE_BADVAL == rc ) {
            emraise( badArchitect, "Reference to undefined shell variable met." );
        } else if( WRDE_CMDSUB == rc ) {
            emraise( badState, "Command substitution requested." );
        } else if( WRDE_NOSPACE == rc ) {
            emraise( memAllocError, "Attempt to allocate memory failed." );
        } else if( WRDE_SYNTAX == rc ) {
            emraise( interpreter, "Shell syntax error." );
        }
    }
    const Size nWords = pwords.we_wordc;
    argvTokens = (char**) malloc( sizeof(char*)*nWords );
    for( size_t n = 0; n < nWords; ++n ) {
        argvTokens[n] = strdup( pwords.we_wordv[n] );
    }
    wordfree( &pwords );
    return nWords;
}

// TODO: to be replaced by goo's ::goo::dict::Configuration::tokenize_string()
// or ::goo::dict::Configuration::free_tokens()
// when goo/appParameters branch will be merged to goo/master:
void
goo_XXX_free_tokens( size_t argcTokens, char ** argvTokens ) {
    for( size_t n = 0; n < argcTokens; ++n ) {
        free( argvTokens[n] );
    }
    free( argvTokens );
}

}  // namespace aux
}  // namespace sV

