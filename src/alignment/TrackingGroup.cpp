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

# include "alignment/TrackingGroup.hpp"

# ifdef ALIGNMENT_ROUTINES

# include "app/abstract.hpp"
# include <boost/property_tree/ptree.hpp>

namespace sV {
namespace alignment {

TrackingGroup::TrackingGroup( const std::string & name_ ) : _name(name_) {
    # if 0
    auto insertionResult = TrackingGroupFactory::groups().emplace( name_, this );
    if( insertionResult.second ) {
        emraise( badArchitect, "Repeatative insertion of tracking group \"%s\". "
                 "Tracking group members have to be unique.", name_.c_str() );
    }
    # endif
}

const TrackingGroup::Volume &
TrackingGroup::volume( const Placement * plPtr ) const {
    assert( plPtr );
    auto it = _pl2vol.find( plPtr );
    if( _pl2vol.end() == it ) {
        emraise( notFound, "Couldn't find volume for \"%s\":%p placement.",
                 plPtr->detector_name(), plPtr );
    }
    return *(it->second);
}

const TrackingGroup::Placement &
TrackingGroup::placement( const TrackingGroup::Volume * volPtr ) const {
    assert( volPtr );
    auto it = _vol2pl.find( volPtr );
    if( _vol2pl.end() == it ) {
        emraise( notFound, "Couldn't find placement for %p volume.",
                 volPtr );
    }
    return *(it->second);
}

void
TrackingGroup::include( const Placement & pl ) {
    if( !pl.is_detector_constructed() ) {
        emraise(badState, "Tracking groups initialization must be invoked after all "
                "the detectors are constructed." );
    }
    const iDetector * iDetPtr = pl.detector();
    if( !pl.detector()->is_receptive() ) {
        // If detector is not even receptive, it can not represent a
        // tracking volume, so omit it.
        sV_loge( "Detector instance \"%s\" can not be added into tracking group "
            "\"%s\" as is not even a receptive detector (ignored).\n",
            pl.detector_name(), this->name() );
        return;
    }
    auto atvPtr = dynamic_cast
        <const ::sV::alignment::AbstractTrackReceptiveVolume *>
        ( iDetPtr );

    if( !atvPtr ) {
        // Omit non-tracking detectors.
        sV_loge( "Detector instance %s can not be added into tracking group "
            "\"%s\" as it does not represents track point receptive volume (ignored).\n",
            pl.detector_name(), this->name() );
        return;
    }

    _volumes.push_back( atvPtr );
    _pl2vol.emplace( &pl, atvPtr );
    _vol2pl.emplace( atvPtr, &pl );

    sV_log2( "Detector \"" ESC_CLRGREEN "%s:%s" ESC_CLRCLEAR "\" by address %p (ptr to volume) "
            "added into tracking group \"" ESC_CLRYELLOW "%s" ESC_CLRCLEAR "\" (%p).\n",
            pl.detector_name(), pl.detector()->family_name().c_str(), atvPtr,
            name(), this );
}

void
TrackingGroup::persistency( size_t n ) {
    if( !n ) { n = 1; }
    for( auto volPtr : _volumes ) {
        // TODO: keep const validity!
        const_cast<Volume*>(volPtr)->n_sets( n );
    }
}

// Drawable group factory
////////////////////////

std::unordered_map<std::string, TrackingGroupFactory::GroupConstructor> *
TrackingGroupFactory::_grpCtrs = nullptr;

void
TrackingGroupFactory::register_constructor( const std::string & name, GroupConstructor ctr ) {
    typedef typename std::remove_pointer<DECLTYPE(_grpCtrs)>::type DictType;
    if( !_grpCtrs ) {
        _grpCtrs = new DictType;
    }
    auto insertionResult = _grpCtrs->insert( DictType::value_type(name, ctr) );
    if( !insertionResult.second ) {
        emraise( nonUniq, "Couldn't insert drawable tracking group ctr \"%s\"",
                 name.c_str() );
    }
}

TrackingGroup *
TrackingGroupFactory::new_drawable_tracking_group(
            const std::string & name, const PTree & parameters ) {
    const std::string & algorithmName = parameters.get<std::string>("reconstructionMethod");
    if( !_grpCtrs ) {
        sV_loge( "Couldn't resolve drawable tracking group \"%s:%s\" as "
                     "this build does not support any tracking groups at all.\n",
                     name.c_str(),
                     algorithmName.c_str() );
        return nullptr;
    }
    auto it = _grpCtrs->find( algorithmName );
    if( _grpCtrs->end() == it ) {
        sV_loge( "Has no drawable tracking group \"%s:%s\".\n",
                     name.c_str(),
                     algorithmName.c_str() );
        return nullptr;
    }
    return it->second( name, parameters );
}

void
TrackingGroupFactory::delete_drawable_tracking_group( TrackingGroup * /*ptr*/ ) {
    //delete ptr;  // TODO: causes segfaults sometimes
}

# if 0  //XXX: use alignment::app
//std::unordered_map<std::string, TrackingGroup *> *
//TrackingGroupFactory::_createdGroups = nullptr;  //XXX: use alignment::app
std::unordered_map<std::string, TrackingGroup *> &
TrackingGroupFactory::groups() {
    typedef std::remove_pointer<DECLTYPE(_createdGroups)>::type TT;
    if( !_createdGroups ) {
        _createdGroups = new TT;
    }
    return *_createdGroups;
}
# endif

}  // namespace alignment
}  // namespace sV

# endif  // ALIGNMENT_ROUTINES

