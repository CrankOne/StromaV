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

# include "alignment/CompoundDetector.hpp"

# ifdef ALIGNMENT_ROUTINES

# include "goo_exception.hpp"
# include <cassert>

namespace sV {
namespace alignment {

// void  // XXX: see comment in header
// CompoundDetector::_add_alias( const std::string & newAlias ) {
//     _namesList.push_back( newAlias );
// }

CompoundDetector::CompoundDetector( const std::string & fn,
                                    const std::string & dn,
                                    const std::initializer_list<std::string> & names ) :
        iDetector(fn, dn, false, false, true ),
        _namesList(names) {
    assert( this->is_compound() );  // forgot to specify that detector is compound at final descendant
}

}  // namespace alignment
}  // namespace sV

# endif

