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

# ifndef H_STROMA_V_AUX_UTILITY_H
# define H_STROMA_V_AUX_UTILITY_H

# include "sV_config.h"

# include <regex>
# include <list>
# include <set>
# include <cstdarg>

# ifdef GEANT4_MC_MODEL
# include <G4ThreeVector.hh>
# endif

namespace sV {
namespace aux {

std::list<double>
parse_double_fp_vector( const std::string & );

# ifdef GEANT4_MC_MODEL
G4ThreeVector parse_g4_three_vector( const std::string &, double factor=1 );
# endif

// TODO: to be replaced by goo's ::goo::dict::Configuration::tokenize_string()
// or ::goo::dict::Configuration::free_tokens()
// when goo/appParameters branch will be merged to goo/master:
size_t goo_XXX_tokenize_argstring( const std::string &, char **& argvTokens );

// TODO: to be replaced by goo's ::goo::dict::Configuration::tokenize_string()
// or ::goo::dict::Configuration::free_tokens()
// when goo/appParameters branch will be merged to goo/master:
void goo_XXX_free_tokens( size_t argcTokens, char ** argvTokens );

}  // namespace aux
}  // namespace sV

/// Regular expression matching floating-point number decimal numbers (both,
/// in usual and in scientific notation).
# define STROMAV_FLTNUM_ASCII_REGEX "[-+]?[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?"

# endif  // H_STROMA_V_AUX_UTILITY_H


