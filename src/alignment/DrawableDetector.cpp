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

# include "alignment/DrawableDetector.hpp"

# ifdef ALIGNMENT_ROUTINES

# include "app/app.h"

# include <cassert>

# include <TEveTrans.h>
# include <TEveText.h>
# include <TEveGeoShape.h>

namespace sV {
namespace alignment {

const DrawableDetector::Container *
DrawableDetector::top_ptr() const {
    assert( _top );
    return _top;
}

DrawableDetector::Container *
DrawableDetector::top_ptr() {
    assert( _top );
    return _top;
}

void
DrawableDetector::_set_label( const char * txt, float x,
                                                float y,
                                                float z ) {
    _label = new TEveText( txt );
    TEveTrans & lt = _label->RefMainTrans();
    lt.SetPos( x, y, z );
    top_ptr()->AddElement( _label );
}

const TAttBBox *
DrawableDetector::bbox() const {
    auto res = _V_bbox();
    if( !res ) {
        sV_logw( "Detector %p \"%s:%s\" has unimplemented bounding box "
                     "yielding method while its getter invoked (unimplemented?).\n",
                     this,
                     iDetector::detector_name().c_str(),
                     iDetector::family_name().c_str() );
    }
    return res;
}

}  // namespace alignment
}  // namespace sV

# endif  // ALIGNMENT_ROUTINES

