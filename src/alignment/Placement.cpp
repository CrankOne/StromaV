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

# include "alignment/Placement.hpp"

# ifdef ALIGNMENT_ROUTINES

# include <cstring>
# include <goo_exception.hpp>

namespace sV {
namespace alignment {

DetectorPlacement::DetectorPlacement( const std::string & ctrName,
                                      const std::string & dtrName ) : _isConstructed(false) {
    bzero( &_position, sizeof(_position) );
    bzero( &_rotation, sizeof(_rotation) );
    _detector.constructorInfo = new ConstructorInfo;
    _detector.constructorInfo->constructorName = strdup( ctrName.c_str() );
    _detector.constructorInfo->detectorName    = strdup( dtrName.c_str() );
}

void
DetectorPlacement::_free_constructorInfo() {
    free( _detector.constructorInfo->constructorName );
    free( _detector.constructorInfo->detectorName );
    delete _detector.constructorInfo;
}

void
DetectorPlacement::set_detector_instance( iDetector * dtrPtr ) {
    assert( !_isConstructed );
    _free_constructorInfo();
    _detector.instancePtr = dtrPtr;
    _isConstructed = true;
}

void
DetectorPlacement::rotation( const Rotation & rot ) {
    if( _isConstructed ) {
        emraise( badState, "Detector is already constructed at this placement." )
    }
    _rotation = rot;
}

void
DetectorPlacement::position( const Position & pos ) {
    if( _isConstructed ) {
        emraise( badState, "Detector is already constructed at this placement." )
    }
    _position = pos;
}

}  // namespace alignment
}  // namespace sV

# endif  // ALIGNMENT_ROUTINES

