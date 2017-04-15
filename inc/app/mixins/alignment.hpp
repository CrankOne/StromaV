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

# ifndef H_STROMA_V_ALIGNMENT_APPLICATION_MIXIN_H
# define H_STROMA_V_ALIGNMENT_APPLICATION_MIXIN_H

# include "sV_config.h"

# ifdef ALIGNMENT_ROUTINES

# include "app/abstract.hpp"

# include <boost/property_tree/ptree_fwd.hpp>

# include <unordered_map>

namespace sV {

namespace alignment {
class DetectorConstructorsDict;
class DetectorsSet;
class TrackingGroup;
class DetectorPlacement;
}  // namespace alignment

namespace mixins {

/**@class AlignmentApplication
 *
 * @brief Application mixin that keeps track of all the available
 * alignment entities (detectors and tracking).
 *
 * @ingroup app
 * @ingroup alignment
 */
class AlignmentApplication : public virtual AbstractApplication {
private:
    static alignment::DetectorConstructorsDict * _detCtrsDict;
    alignment::DetectorsSet * _detectorsSet;
    std::unordered_map<std::string, alignment::TrackingGroup *> _trackingGroups;
protected:
    typedef std::unordered_map<alignment::TrackingGroup *, std::list<std::string> >
            TrackingMembersNames;
protected:
    virtual std::list<alignment::DetectorPlacement> _get_placements( boost::property_tree::ptree & );
    virtual TrackingMembersNames _parse_groups( boost::property_tree::ptree & );
    virtual void _parse_experimental_layout_settings(
                        std::ifstream &,
                        std::list<alignment::DetectorPlacement> &,
                        TrackingMembersNames & );
public:
    AlignmentApplication( po::variables_map * );
    ~AlignmentApplication();

    /// Returns dictionary of detector construction functions.
    static alignment::DetectorConstructorsDict & constructors();

    /// Returns registered detectors list.
    alignment::DetectorsSet & detectors();

    /// Prints parameters tree (useful for JSON-parsing debug).
    void print_ptree( boost::property_tree::ptree const &, std::ostream & );

    const std::unordered_map<std::string, alignment::TrackingGroup *> & tracking_groups() const
        { return _trackingGroups; }
};

}  // namespace mixins

}  // namespace sV

# endif  // ALIGNMENT_ROUTINES

# endif  // H_STROMA_V_ALIGNMENT_APPLICATION_MIXIN_H

