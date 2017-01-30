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

# include "alignment/DetectorsSet.hpp"
# include "alignment/DetCtrDict.hpp"
# include "alignment/DrawableDetector.hpp"
# include "alignment/ReceptiveDetector.hpp"
# include "alignment/CompoundDetector.hpp"
# include "detector_ids.hpp"

# ifdef ALIGNMENT_ROUTINES

# include "app/mixins/alignment.hpp"

//# include <goo_mixins.tcc>

# include <TEveManager.h>
# include <TEveViewer.h>
# include <TEveScene.h>
# include <TEveTrans.h>

# include <TEveElement.h>
//# include <TEveGeoShape.h>
//# include <TGeoMatrix.h>

//# define NO_WORKARAOUND_TWICE_TRANSFORM_BUG

namespace sV {
namespace alignment {

using ::sV::aux::iDetectorIndex;

void
DetectorsSet::_emplace( const std::string & famName,
                        const std::string & detName,
                        DetectorPlacement * placementPtr ) {
    _byFamily.emplace( famName, placementPtr );
    _byName.emplace(   detName, placementPtr );

    AFR_UniqueDetectorID id{0};
    //id.byNumber.major = detector_major_by_name( detName.c_str() );
    id.byNumber.major = iDetectorIndex::self().mj_code( detName.c_str() );
    if( id.byNumber.major ) {
        _byUniqueID.emplace( id.wholenum, placementPtr );
        _majorIds.emplace( placementPtr, id.byNumber.major );
        
    } else {
        sV_logw( "Couldn't determine ID for detector %s:%s. "
                     "It won't be included into hits dispatching receivers.\n",
                     famName.c_str(), detName.c_str() );
    }
}

DetectorPlacement &
DetectorsSet::_register_detector( const std::string & famName,
                                  const std::string & detName,
                                  const DetectorPlacement & uninitPlacement ) {
    _detectors.push_back( uninitPlacement );
    DetectorPlacement * newPlacementPtr = &(*_detectors.rbegin());

    _emplace( famName, detName, newPlacementPtr );

    return *newPlacementPtr;
}

void
DetectorsSet::construct_detector( const DetectorPlacement & placement ) {
    assert( !placement.is_detector_constructed() );
    DetectorConstructorsDict::DetectorConstructor constructor =
        goo::app<mixins::AlignmentApplication>()
                .constructors()
                .get_constructor( placement.constructor_name() );
    if( !constructor ) {
        sV_loge( "No constructor found for detector \"%s\" by constructor name \"%s\".\n",
                    placement.detector_name(),
                    placement.constructor_name() );
        return;
    }
    iDetector * constructedDetectorPtr = constructor( placement.detector_name() );
    DetectorPlacement & newPlacement = 
        goo::app<mixins::AlignmentApplication>()
            .detectors()
            ._register_detector( placement.constructor_name(),
                                 placement.detector_name(),
                                 placement )
            ;
    // After that, detector/constructor names aren't available
    // in placement.
    newPlacement.set_detector_instance( constructedDetectorPtr );

    if( constructedDetectorPtr->is_compound() ) {
        auto namesList = constructedDetectorPtr->compound().detectors_nameslist();
        for( auto it = namesList.cbegin(); namesList.cend() != it; ++it ) {
            // todo: what if we have detectors from different families in
            // compound?
            if( constructedDetectorPtr->detector_name() == *it ) { continue; }
            goo::app<mixins::AlignmentApplication>()
                .detectors()
                ._emplace( constructedDetectorPtr->family_name(), *it, &newPlacement )
                ;
        }
        goo::app<mixins::AlignmentApplication>()
                .detectors()
                ._compoundDetectors.push_back( &(constructedDetectorPtr->compound()) );
    }
}

void
DetectorsSet::draw_detectors( TEveManager * /*eveManagerPtr*/ ) {
    if( _detectors.empty() ) {
        return;
    }

    # if 0
    char bf[128];
    for( auto it  = _detectors.begin();
              it != _detectors.end(); ++it ) {
        if( it->detector()->is_drawable() ) {
            snprintf( bf, sizeof(bf)/sizeof(bf[0]),
                    "%s:%s", it->detector()->family_name().c_str(),
                    it->detector()->detector_name().c_str() );
            auto scene = eveManagerPtr->SpawnNewScene(bf, "A specific detector's scene.");
            scene->SetHierarchical( kTRUE );

            it->detector()->drawable().draw_detector( scene );
            {
                TEveTrans & trans = scene->RefMainTrans();
                // why the heck this rotation is divided by 2? Does
                // this root crap somehow multiplies it?
                // TODO: seems like that transformation is applied twice.
                // grep project for NO_WORKARAOUND_TWICE_TRANSFORM_BUG to
                // find place were it becomes crucial.
                # ifndef NO_WORKARAOUND_TWICE_TRANSFORM_BUG
                trans.SetRotByAngles( it->rotation().byName.z*M_PI/360.,
                                     -it->rotation().byName.y*M_PI/360.,
                                      it->rotation().byName.x*M_PI/360. );
                trans.SetPos( it->position().byName.x/2.,
                              it->position().byName.y/2.,
                              it->position().byName.z/2. );
                # else
                trans.SetRotByAngles( it->rotation().byName.z*M_PI/180.,
                                     -it->rotation().byName.y*M_PI/180.,
                                      it->rotation().byName.x*M_PI/180. );
                trans.SetPos( it->position().byName.x,
                              it->position().byName.y,
                              it->position().byName.z );
                # endif
            }
            eveManagerPtr->GetDefaultViewer()->AddScene( scene );
        }
    }
    # else
    char nameBuffer[64],
         descriptionBuffer[128]
         ;

    auto detectorScenesLstPtr = new TEveSceneList("Setup", "Detector scenes");
    gEve->AddToListTree( detectorScenesLstPtr, kTRUE );

    for( auto it  = _detectors.begin();
              it != _detectors.end(); ++it ) {
        if( it->detector()->is_drawable() ) {

            snprintf( nameBuffer, sizeof(nameBuffer)/sizeof(nameBuffer[0]),
                    "%s", it->detector()->detector_name().c_str() );

            snprintf( descriptionBuffer, sizeof(descriptionBuffer)/sizeof(descriptionBuffer[0]),
                    "%s:%s detector %p", it->detector()->family_name().c_str(),
                    it->detector()->detector_name().c_str(),
                    it->detector() );

            auto detectorGeometryScene = new TEveScene( nameBuffer, descriptionBuffer );
            detectorGeometryScene->SetHierarchical( kTRUE );
            // Add scene to global list of scenes:
            gEve->AddElement( detectorGeometryScene,
                              gEve->GetScenes() );
            // Add scene to dedicated list of scenes:
            detectorScenesLstPtr->AddElement( detectorGeometryScene );

            it->detector()->drawable().draw_detector( detectorGeometryScene );

            {
                TEveTrans & trans = detectorGeometryScene->RefMainTrans();
                // why the heck this rotation is divided by 2? Does
                // this root crap somehow multiplies it?
                // TODO: seems like that transformation is applied twice.
                // grep project for NO_WORKARAOUND_TWICE_TRANSFORM_BUG to
                // find place were it becomes crucial.
                # ifndef NO_WORKARAOUND_TWICE_TRANSFORM_BUG
                trans.SetRotByAngles( it->rotation().byName.z*M_PI/360.,
                                     -it->rotation().byName.y*M_PI/360.,
                                      it->rotation().byName.x*M_PI/360. );
                trans.SetPos( it->position().byName.x/2.,
                              it->position().byName.y/2.,
                              it->position().byName.z/2. );
                # else
                trans.SetRotByAngles( it->rotation().byName.z*M_PI/180.,
                                     -it->rotation().byName.y*M_PI/180.,
                                      it->rotation().byName.x*M_PI/180. );
                trans.SetPos( it->position().byName.x,
                              it->position().byName.y,
                              it->position().byName.z );
                # endif
            }

            // Let viewer associate the detector scene via its SceneInfo object
            // (association media).
            gEve->GetDefaultViewer()->AddScene( detectorGeometryScene );
        }
    }
    # endif
}

bool
DetectorsSet::reset_hits() {
    bool doUpdate = false;
    for( auto it = _detectors.begin(); _detectors.end() != it; ++it ) {
        // consider only receptive detectors:
        if( it->is_detector_constructed() && it->detector()->is_receptive() ) {
            bool detectorUpdated = it->detector()->receptive().reset_summary();
            // only change doUpdate if detector is also drawable:
            if( it->detector()->is_drawable() ) {
                doUpdate |= detectorUpdated;
            }
        }
    }
    return doUpdate;
}

/** Iterates through all the detectors known to this set. If particular detector
 * class was inherited from receptive instance, the hit setter method
 * ReceptiveDetector::hit() will be invoked with appropriate hit instance.
 *
 * Ehen detector instance is not only receptive but also drawable its
 * DrawableDetector::draw_hit() method will be invoked. If this method
 * returns true for at least one detector instance, the DetectorsSet::dispatch_hits()
 * will also return true at the end of the loop.
 */ bool
DetectorsSet::dispatch_hits( const ::sV::events::Displayable & msg ) {
    bool doUpdate = false;
    AFR_UniqueDetectorID cDetID{0};
    size_t nDrawn = 0;
    for( uint32_t i = 0; i < (uint32_t) msg.summaries_size(); ++i ) {
        const ::sV::events::DetectorSummary & cDetSummary = msg.summaries( i );
        cDetID.wholenum = cDetSummary.detectorid();
        auto it = _byUniqueID.find( cDetID.wholenum );
        if( _byUniqueID.end() == it ) {
            sV_loge( "Event display couldn't find detector instance "
                     "with unique ID 0x%x (%s). Summary won't be displayed.\n",
                     (int) cDetID.wholenum,
                     iDetectorIndex::self().name( cDetID.byNumber.major )
                );
            continue;
        }
        DetectorPlacement & placement_ = *(it->second);
        iDetector & detector = *placement_.detector();
        if( !detector.is_receptive() ) {
            sV_loge( "Detector with unique ID %d (%s:%s) is not receptive. "
                     "Skipping it.\n",
                     (int) cDetID.wholenum,
                     detector.family_name().c_str(),
                     detector.detector_name().c_str()
                );
            continue;
        }
        // Set new hit and mark it for re-drawing, if need.
        if( detector.receptive().common_summary( cDetSummary ) 
         && detector.is_drawable() ) {
            if( detector.drawable().draw_hit() ) {
                ++nDrawn;
                doUpdate = true;
            }
        }
    }
    // All summaries processed => all hits set => let compounds know:
    for( auto compDetPtr : _compoundDetectors ) {
        compDetPtr->hit_setting_finale();
    }
    sV_log3( "%zu hits drawn of %zu summaries provided by event.\n",
             nDrawn, msg.summaries_size() );
    return doUpdate;
}

const DetectorPlacement &
DetectorsSet::placement( const std::string & plName ) const {
    auto it = _byName.find( plName );
    if( _byName.end() == it ) {
        emraise( notFound, "Has no detector named \"%s\".", plName.c_str() );
    }
    return *(it->second);
}

AFR_DetMjNo
DetectorsSet::major_number( const DetectorPlacement * plPtr ) const {
    auto it = _majorIds.find( plPtr );
    if( _majorIds.end() == it ) {
        emraise( notFound, "Has no major number indexed for placement %p (%s).",
            plPtr, plPtr->detector_name() );
    }
    return it->second;
}

}  // namespace alignment
}  // namespace sV

# endif

