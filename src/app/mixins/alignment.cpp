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

/**@defgroup alignment Alignment
 * @brief Geometry and positioning of detectors.
 *
 * Despite of that we do not need sophisticated alignment algorithms
 * in NA64, some rudimentary API is quite desirable since we still
 * need to describe detectors geometrical placements for calibration,
 * event display and general analysis applications.
 * */

# include "app/mixins/alignment.hpp"

# ifdef ALIGNMENT_ROUTINES

# include <boost/property_tree/ptree.hpp>
# include <boost/property_tree/json_parser.hpp>
# include <boost/foreach.hpp>

# include "alignment/TrackingGroup.hpp"
# include "alignment/Placement.hpp"
# include "alignment/DetCtrDict.hpp"
# include "alignment/DetectorsSet.hpp"
# include "alignment/TrackingGroup.hpp"

namespace sV {
namespace mixins {

AlignmentApplication::AlignmentApplication( AbstractApplication::Config * vmPtr ) :
        AbstractApplication( vmPtr ),
        _detectorsSet(nullptr) {}

AlignmentApplication::~AlignmentApplication() {
    for( auto it = _trackingGroups.begin(); _trackingGroups.end() != it; ++it ) {
        alignment::TrackingGroupFactory::delete_drawable_tracking_group( it->second );
    }
}

alignment::DetectorConstructorsDict * AlignmentApplication::_detCtrsDict = nullptr;

AlignmentApplication::TrackingMembersNames
AlignmentApplication::_parse_groups( boost::property_tree::ptree & rootPT ) {
    TrackingMembersNames detectorsToPlace;
    {
        typedef boost::property_tree::basic_ptree<std::string,std::string> PTree;
        for( PTree::const_iterator it = rootPT.get_child("trackingGroups").begin();
             rootPT.get_child("trackingGroups").end() != it; ++it ) {

            // Will contain list of names to be added (order important).
            std::list<std::string> detectorNames;

            PTree detectorNamesTree = it->second.get_child( "members" );
            for( PTree::const_iterator dnIt = detectorNamesTree.begin();
                 detectorNamesTree.end() != dnIt; ++dnIt ) {
                detectorNames.push_back( dnIt->second.get_value<std::string>() );
            }

            auto groupPtr = alignment
                          ::TrackingGroupFactory
                          ::new_drawable_tracking_group( it->first, it->second );
            if( !groupPtr ) {
                continue;
            } else {
                sV_log3( "Tracking group \"%s\" constructed: %p.\n",
                             groupPtr->name(), groupPtr );
            }
            auto insertionResult = _trackingGroups
                    .insert( DECLTYPE(_trackingGroups)::value_type( it->first, groupPtr ) );
            if( !insertionResult.second ) {
                emraise( nonUniq, "Repeatative insertion of tracking group \"%s\". "
                 "Tracking group members have to be unique.", it->first.c_str() );
            }
            detectorsToPlace.insert( DECLTYPE(detectorsToPlace)::value_type( groupPtr, detectorNames ) );
        }
    }
    return detectorsToPlace;
}

void
AlignmentApplication::_parse_experimental_layout_settings(
                    std::ifstream & plStream,
                    std::list<alignment::DetectorPlacement> & placements,
                    TrackingMembersNames & trackingMembersNames ) {
    std::stringstream ss;
    // send your JSON above to the parser below, but populate ss first
    boost::property_tree::ptree rootPT;
    boost::property_tree::read_json(plStream, rootPT);
    //print_ptree(rootPT, std::cout);
    // Create tracking groups
    trackingMembersNames = _parse_groups( rootPT );
    // Get placements
    placements = _get_placements( rootPT );
}

void
AlignmentApplication::print_ptree( boost::property_tree::ptree const & pt,
                          std::ostream & oStream ) {
    using boost::property_tree::ptree;
    ptree::const_iterator end = pt.end();
    for( ptree::const_iterator it = pt.begin(); it != end; ++it ) {
        oStream << it->first << ": " << it->second.get_value<std::string>() << std::endl;
        print_ptree( it->second, oStream );
    }
}

alignment::DetectorConstructorsDict &
AlignmentApplication::constructors() {
    if( !_detCtrsDict ) {
        _detCtrsDict = new alignment::DetectorConstructorsDict;
    }
    return *_detCtrsDict;
}

alignment::DetectorsSet &
AlignmentApplication::detectors() {
    if( !_detectorsSet ) {
        _detectorsSet = new alignment::DetectorsSet;
    }
    return *_detectorsSet;
}

std::list<alignment::DetectorPlacement>
AlignmentApplication::_get_placements( boost::property_tree::ptree & rootPT ) {
    std::list<alignment::DetectorPlacement> placements;
    {
        typedef boost::property_tree::basic_ptree<std::string,std::string> PTree;
        for( PTree::const_iterator it = rootPT.get_child("detectors").begin();
             rootPT.get_child("detectors").end() != it; ++it ) {
            const std::string & ctrName = it->second.get<std::string>("family"),
                              & detName = it->first;
            alignment::DetectorPlacement placement( ctrName, detName );
            {  // get position
                PTree posPTree = it->second.get_child("position");
                alignment::DetectorPlacement::Position pos;
                uint8_t posElIdx = 0;
                for( PTree::const_iterator pIt  = posPTree.begin();
                     posPTree.end() != pIt && posElIdx < 3; ++pIt, ++posElIdx ) {
                    pos.r[posElIdx] = pIt->second.get_value<float>();
                    //std::cout << ":: " << pIt->second.get_value<float>() << std::endl;  // XXX
                }
                placement.position( pos );
            }
            {  // get rotation
                PTree rotPTree = it->second.get_child("rotation");
                alignment::DetectorPlacement::Rotation rot;
                uint8_t rotElIdx = 0;
                for( PTree::const_iterator rIt  = rotPTree.begin();
                     rotPTree.end() != rIt && rotElIdx < 3; ++rIt, ++rotElIdx ) {
                    rot.angle[rotElIdx] = rIt->second.get_value<float>();
                    //std::cout << ":: " << rIt->second.get_value<float>() << std::endl;  // XXX
                }
                placement.rotation( rot );
            }
            placements.push_back( placement );
        }
    }

    return placements;
}

}  // namespace mixins
}  // namespace sV

# endif

