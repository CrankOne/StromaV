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

# ifndef H_STROMA_V_EVENT_DISPLAY_DETECTOR_INTERFACE_H
# define H_STROMA_V_EVENT_DISPLAY_DETECTOR_INTERFACE_H

# include "../config.h"

# ifdef ALIGNMENT_ROUTINES

# include <string>

namespace sV {
namespace alignment {

class DrawableDetector;
class ReceptiveDetector;
class CompoundDetector;
template<uint8_t D> class TrackReceptiveVolume;

/**@class iDetector
 * @brief Abstract base class for all the detectors representations.
 *
 * Provides basic introspection and run-time typecasting caches
 * (as dynamic_cast<>s can often lead to significant performance
 * lacks one need them).
 *
 * Has no abstract functions.
 */
class iDetector {
private:
    const std::string _familyName;
    const std::string _detectorName;
    const bool _isDrawable,
               _isReceptive,
               _isCompound
               ;

    DrawableDetector * _drawableThis;
    ReceptiveDetector * _receptiveThis;
    CompoundDetector * _compoundThis;
protected:
    iDetector() = delete;
    iDetector( const std::string & famName,
               const std::string & detName,
               bool isDrawable,
               bool isReceptive,
               bool isCompound );
public:
    virtual ~iDetector(){}

    /// Returns family name (e.g. ECAL, HCAL, GEM, etc).
    const std::string & family_name() const { return _familyName; }

    /// Returns particular detector instance name (e.g. GEM01, ECAL0, etc.)
    const std::string & detector_name() const { return _detectorName; }

    /// True, when particular detector descendant can be casted to
    /// sV::alignment::ReceptiveDetector.
    bool is_receptive() const { return _isDrawable; }
    /// True, when particular detector descendant can be casted to
    /// sV::alignment::DrawableDetector.
    bool is_drawable() const { return _isReceptive; }
    /// True, when particular detector descendant can be casted to
    /// sV::alignment::CompoundDetector.
    bool is_compound() const { return _isCompound; }
    
    DrawableDetector & drawable();
    const DrawableDetector & drawable() const;

    ReceptiveDetector & receptive();
    const ReceptiveDetector & receptive() const;

    CompoundDetector & compound();
    const CompoundDetector & compound() const;
};  // class DetectorPlacement

}  // namespace alignment
}  // namespace sV

# endif // ALIGNMENT_ROUTINES

# endif  // H_STROMA_V_EVENT_DISPLAY_DETECTOR_INTERFACE_H

