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

# ifndef H_STROMA_V_EVENT_DISPLAY_COMPOUND_DETECTOR_MIXIN_H
# define H_STROMA_V_EVENT_DISPLAY_COMPOUND_DETECTOR_MIXIN_H

# include "sV_config.h"

# ifdef ALIGNMENT_ROUTINES

# include <list>

# include "iDetector.hpp"
# include "../detector_ids.h"

namespace sV {
namespace alignment {

/**@class CompoundDetector
 * @brief This detector representation is actually a composition of several
 * physical detectors.
 *
 * The meaning of «composition» here comes from DAQ system rather than from
 * real physical constitution of detector. It is not recommended to utilize
 * this mixing unless there is obvious need to treat a set of detectors as
 * a single entity.
 *
 * Obvious need examples:
 *  - We have different axes of Micromegas and GEMs separated in DAQ, however
 *    in analysis we often have to consider it as one detector entity.
 *  - We have ECAL separately read within its two parts: pershower and ECAL
 *    itself. Despite in future it is reasonable to consider it as
 *    longitudal-segmented box calorimeter, we have it as two parts in DDD DAQ.
 *
 * @ingroup alignment
 * */
class CompoundDetector : public virtual iDetector {
private:
    const std::list<std::string> _namesList;
protected:
    /// Returns side instance indexed by provided ID. Usually this.
    virtual iDetector & _V_get_instance( const AFR_UniqueDetectorID & )
            { return *this; }
    /// Returns side instance indexed by provided ID (const). Usually this.
    virtual const iDetector & _V_get_instance_const( const AFR_UniqueDetectorID & ) const
            { return *this; }
    /// Called after all available hits set.
    virtual void _V_hit_setting_finale() {}
public:
    CompoundDetector( const std::string & fn,
                      const std::string & dn,
                      const std::initializer_list<std::string> & names );
    
    iDetector & get_instance( const AFR_UniqueDetectorID & dID ) {
        return _V_get_instance( dID ); }
    const iDetector & get_instance( const AFR_UniqueDetectorID & dID ) const {
        return _V_get_instance_const( dID ); }

    const std::list<std::string> & detectors_nameslist() { return _namesList; }

    void hit_setting_finale() { _V_hit_setting_finale(); }
};  // class ReceptiveDetector

}  // namespace alignment
}  // namespace sV

# endif  // ALIGNMENT_ROUTINES

# endif  // H_STROMA_V_EVENT_DISPLAY_COMPOUND_DETECTOR_MIXIN_H


