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

# include "alignment/iDetector.hpp"
# include "alignment/DrawableDetector.hpp"
# include "alignment/ReceptiveDetector.hpp"
# include "alignment/CompoundDetector.hpp"

# ifdef ALIGNMENT_ROUTINES

# include <goo_exception.hpp>
# include <cassert>

namespace sV {
namespace alignment {

iDetector::iDetector( const std::string & famName,
                      const std::string & detName,
                      bool isDrawable,
                      bool isReceptive,
                      bool isCompound ) :
                                    _familyName(famName),
                                    _detectorName(detName),
                                    _isDrawable(isDrawable),
                                    _isReceptive(isReceptive),
                                    _isCompound(isCompound),
                                    _drawableThis( nullptr ),
                                    _receptiveThis( nullptr ),
                                    _compoundThis( nullptr ) {}

DrawableDetector &
iDetector::drawable() {
    if( !is_drawable() ) {
        emraise( badCast, "Detector %p (%s:%s) is not drawable.",
                 this, family_name().c_str(), detector_name().c_str() );
    }
    if( !_drawableThis ) {
        _drawableThis  = dynamic_cast<DrawableDetector *>( this);
        assert( _drawableThis );
    }
    return *_drawableThis;
}

const DrawableDetector &
iDetector::drawable() const {
    if( !is_drawable() ) {
        emraise( badCast, "Detector %p (%s:%s) is not drawable.",
                 this, family_name().c_str(), detector_name().c_str() );
    }
    if( !_drawableThis ) {
        iDetector * mutableThis = const_cast<iDetector *>(this);
        mutableThis->_drawableThis =
                dynamic_cast<DrawableDetector *>(mutableThis);
        assert( _drawableThis );
    }
    return *_drawableThis;
}


ReceptiveDetector &
iDetector::receptive() {
    if( !is_receptive() ) {
        emraise( badCast, "Detector %p (%s:%s) is not receptive.",
                 this, family_name().c_str(), detector_name().c_str() );
    }
    if( !_receptiveThis ) {
        _receptiveThis  = dynamic_cast<ReceptiveDetector *>( this);
        assert( _receptiveThis );
    }
    return *_receptiveThis;
}

const ReceptiveDetector &
iDetector::receptive() const {
    if( !is_receptive() ) {
        emraise( badCast, "Detector %p (%s:%s) is not receptive.",
                 this, family_name().c_str(), detector_name().c_str() );
    }
    if( !_receptiveThis ) {
        iDetector * mutableThis = const_cast<iDetector *>(this);
        mutableThis->_receptiveThis =
                dynamic_cast<ReceptiveDetector *>(mutableThis);
        assert( _receptiveThis );
    }
    return *_receptiveThis;
}


CompoundDetector &
iDetector::compound() {
    if( !is_compound() ) {
        emraise( badCast, "Detector %p (%s:%s) is not compound.",
                 this, family_name().c_str(), detector_name().c_str() );
    }
    if( !_compoundThis ) {
        _compoundThis  = dynamic_cast<CompoundDetector *>( this );
        assert( _compoundThis );
    }
    return *_compoundThis;
}

const CompoundDetector &
iDetector::compound() const {
    if( !is_compound() ) {
        emraise( badCast, "Detector %p (%s:%s) is not compound.",
                 this, family_name().c_str(), detector_name().c_str() );
    }
    if( !_compoundThis ) {
        iDetector * mutableThis = const_cast<iDetector *>( this );
        mutableThis->_compoundThis =
                dynamic_cast<CompoundDetector *>(mutableThis);
        assert( _compoundThis );
    }
    return *_compoundThis;
}

}  // namespace alignment
}  // namespace sV

# endif  // ALIGNMENT_ROUTINES

