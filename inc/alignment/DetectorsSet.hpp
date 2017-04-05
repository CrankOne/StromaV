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

# ifndef H_STROMA_V_EVENT_DISPLAY_DETECTORS_SET_H
# define H_STROMA_V_EVENT_DISPLAY_DETECTORS_SET_H

# include "sV_config.h"

# ifdef ALIGNMENT_ROUTINES

# include "../detector_ids.h"
# include "alignment/Placement.hpp"
# include "../uevent.hpp"

# include <unordered_map>
# include <map>
# include <list>

class TEveManager;

namespace sV {
namespace alignment {

class Application;
class CompoundDetector;

/**@class DetectorsSet
 * @brief Class managing detectors representations for alignment tasks.
 *
 * Keeps track of the detector instances via their placement objects
 * (delegated association). Manages detectors isntances lifetime and
 * initialization, routes hits and provide cached getters: by name,
 * by unique identifiers, by their family (constructor names).
 *
 * There are few general features known for this class:
 *   - Abstract detector instances (virtual base for all subsequent mixins),
 *     see sV::alignment::iDetector.
 *   - Detectors that can be visualized as static geometrical objects,
 *     see sV::alignment::DrawableDetector.
 *   - Detectors that have to somehow visualize hits (non-static geometry),
 *     see sV::alignment::ReceptiveDetector.
 *   - Detectors that must be considered as «organic whole» from some point,
 *     but actually consists of two or more detectors in terms of physical
 *     DAQ system; see sV::alignment::CompoundDetector for reference.
 *
 * @ingroup alignment
 */
class DetectorsSet {
public:
    typedef iDetector * (* DetectorConstructor)( const std::string & detectorName );
protected:
    std::unordered_multimap<std::string, DetectorPlacement *>   _byFamily;
    std::unordered_map<std::string, DetectorPlacement *>        _byName;
    std::unordered_map<AFR_DetSignature, DetectorPlacement *>   _byUniqueID;

    std::list<CompoundDetector *>                               _compoundDetectors;
    std::list<DetectorPlacement>                                _detectors;

    /// Reverse index of detectors (placement ptr -> major):
    std::unordered_map<const DetectorPlacement *, AFR_DetMjNo> _majorIds;
private:
    /// Lowest level inserter --- operates with all the maps.
    void _emplace( const std::string & famName,
                   const std::string & detName,
                   DetectorPlacement * );
    /// Placement inserter providing placement re-creation.
    DetectorPlacement & _register_detector( const std::string & famName,
                                            const std::string & detName,
                                            const DetectorPlacement & uninitPlacement );
public:
    /// Should be called inside of application instance --- draws detector geometry.
    virtual void draw_detectors( TEveManager * );
    /// Should be called inside of application instance --- draws hits. Returns true,
    /// if setup accepted the hits to draw.
    bool dispatch_hits( const ::sV::events::Displayable & );
    /// Reset hits. Returns true when at least one detector needs to be redrawn.
    bool reset_hits();

    /// Changes the internal state of DetectorPlacement instance.
    static void construct_detector( const DetectorPlacement & placement );

    /// Placements getter.
    const std::list<DetectorPlacement> & placements_list() const { return _detectors; }

    /// Returns reference to detector placement intermediate instance.
    const DetectorPlacement & placement( const std::string & ) const;

    /// Returns detector major ientification number by its placement ptr.
    AFR_DetMjNo major_number( const DetectorPlacement * ) const;
};  // class DetectorsSet

}  // namespace alignment
}  // namespace sV

# endif  // ALIGNMENT_ROUTINES

# endif  // H_STROMA_V_EVENT_DISPLAY_DETECTORS_SET_H



