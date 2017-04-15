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

# ifndef H_STROMA_V_EVENT_DISPLAY_DETECTOR_PLACEMENT_H
# define H_STROMA_V_EVENT_DISPLAY_DETECTOR_PLACEMENT_H

# include "sV_config.h"

# ifdef ALIGNMENT_ROUTINES

# include "iDetector.hpp"

# include <cassert>

namespace sV {
namespace alignment {

class DetectorsSet;

/**@class DetectorPlacement
 * @brief Class associating detector instance with its identifiers.
 *
 * The main purpose of this class is to encapsulate lifetime of particular
 * detector instance providing association media for sV::alignment::DetectorsSet
 * class instance.
 *
 * At some point represents detector that wasn't actually constructed,
 * but still need to be pre-initialized somewhere.
 *
 * @ingroup alignment
 * */
class DetectorPlacement {
public:
    union Position {
        float r[3];
        struct { float x, y, z; } byName;
    };
    union Rotation {
        float angle[3];
        struct { float x, y, z; } byName;
    };
    struct ConstructorInfo {
        char * constructorName,
             * detectorName;
    };
private:
    Position _position;
    Rotation _rotation;
    union {
        ConstructorInfo * constructorInfo;
        iDetector * instancePtr;
    } _detector;
    bool _isConstructed;
protected:
    void set_detector_instance( iDetector * );
    const char * constructor_name() const {
            return _detector.constructorInfo->constructorName; }
    void _free_constructorInfo();
    DetectorPlacement() = delete;
public:
    //DetectorPlacement() : _position{0,0,0}, _rotation{0,0,0},
    //                      _detector.constructorInfo( nullptr ),
    //                      _isConstructed(false) {}
    DetectorPlacement( const std::string & ctrName,
                       const std::string & dtrName );

    /// Rotation getter (when constructed, raises badState).    
    void rotation( const Rotation & );
    /// Rotation getter.
    const Rotation & rotation() const { return _rotation; }

    /// Position setter (when constructed, raises badState).
    void position( const Position & );
    /// Position getter.
    const Position & position() const { return _position; }

    bool is_detector_constructed() const { return _isConstructed; }

    iDetector * detector() {
            assert(is_detector_constructed());
            return _detector.instancePtr; }
    const iDetector * detector() const {
            assert(is_detector_constructed());
            return _detector.instancePtr; }

    const char * detector_name() const {
        if( !is_detector_constructed() ) {
            return _detector.constructorInfo->detectorName;
        } else {
            return detector()->detector_name().c_str();
        }
    }

    friend class ::sV::alignment::DetectorsSet;
};  // class DetectorPlacement

}  // namespace alignment
}  // namespace sV

# endif // ALIGNMENT_ROUTINES

# endif  // H_STROMA_V_EVENT_DISPLAY_DETECTOR_PLACEMENT_H

