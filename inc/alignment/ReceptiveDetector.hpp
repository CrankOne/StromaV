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

# ifndef H_STROMA_V_EVENT_DISPLAY_RECIPTIVE_DETECTOR_MIXIN_H
# define H_STROMA_V_EVENT_DISPLAY_RECIPTIVE_DETECTOR_MIXIN_H

# include "../config.h"

# ifdef ALIGNMENT_ROUTINES

# include "alignment/iDetector.hpp"
# include "../detector_ids.h"
# include "../uevent.hpp"

# include "alignment/TrackingVolume.tcc"

# include <unordered_map>
# include <unordered_set>

namespace sV {
namespace alignment {

/**@class ReceptiveDetector
 * @brief Detector representation instance reacting on hits provided as
 *        summary instances.
 *
 * Provides basics for keeping information about particular hits
 * in event. Incapsulates caching logic for particular type of generalized
 *
 * @ingroup alignment
 */
class ReceptiveDetector : public virtual iDetector {
public:
    typedef ::sV::events::DetectorSummary PackedSummary;
protected:
    /// Ptr to current (packed) summary data.
    const PackedSummary * _summary;
    /// Has to unpack particular summary.
    virtual bool _V_recache_summary( const PackedSummary & ) = 0;
    /// Causes resetting of current cached instance.
    virtual bool _V_reset_summary() {
        bool had = _summary;
        _summary = nullptr;
        return had; }
public:
    ReceptiveDetector( const std::string & fn,
                       const std::string & dn );

    /// Returns true when re-drawing need.
    bool common_summary( const PackedSummary & summary_ ) {
            return _V_recache_summary( *(_summary = &summary_) ); }
    /// Returns (packed) summary data.
    const PackedSummary & common_summary() { return *_summary; }
    /// Returns true if current (packed) summary instance is set.
    virtual bool has_common_summary() const { return _summary; }
    bool reset_summary() { return _V_reset_summary(); }
};  // class ReceptiveDetector

/**@brief Aux receptive detector instance performing unpacking routines for
 * certain summary payload.
 */
template<typename SummaryT>
class CachedPayloadReceptiveDetector : public ReceptiveDetector {
protected:
    bool _empty;
    SummaryT _unpackedSummary;
protected:
    virtual bool _V_recache_summary( const PackedSummary & summary_ ) override {
        if( !summary_.has_summarydata() ) {
            _empty = true;
            return false;
        }
        if( summary_.summarydata().Is<SummaryT>() ) {
            summary_.summarydata().UnpackTo( &_unpackedSummary );
            _empty = false;
        } else {
            _empty = true;
            emraise( badParameter, "CachedPayloadReceptiveDetector instance "
                "%p got wrong summary type to unpack.", this );
        }
        return !_empty;
    }
    virtual bool _V_reset_summary() override {
        if( ReceptiveDetector::_V_reset_summary() ) {
            _unpackedSummary.Clear();
            return true;
        }
        return false;
    }
public:
    CachedPayloadReceptiveDetector( const std::string & fn,
                                    const std::string & dn ) :
            iDetector( fn, dn, false, true, false ),
            ReceptiveDetector(fn, dn), _empty(false) {}
    const SummaryT & summary() const { return _unpackedSummary; }
    virtual bool has_summary() const { return !_empty; }
};  // class CachedPayloadReceptiveDetector

/**@brief Aux receptive detector assembly instance performing unpacking
 *        routines for certain summary payloads.
 *
 * The assemblies of detectors are usually present within set of multiple
 * similar parts with one particular summary.
 */
template<typename ConcreteHitT>
class ReceptiveAssembly : public
            CachedPayloadReceptiveDetector<::sV::events::AssemblySummary> {
public:
    typedef CachedPayloadReceptiveDetector<::sV::events::AssemblySummary> Parent;
    typedef CachedPayloadReceptiveDetector<ConcreteHitT> ReceptivePart;
    typedef std::unordered_map<AFR_DetSignature, ReceptivePart *> PartsMap;
protected:
    /// Child class has to fill this map by its own.
    PartsMap _parts;
protected:
    virtual bool _V_recache_summary( const PackedSummary & ps ) final {
        if(!Parent::_V_recache_summary(ps)){
            return false;
        }
        std::unordered_set<AFR_DetSignature> met;
        bool set = false;
        for( const auto & pSummary
                : summary()
                .partialsummaries() ) {
            met.insert( pSummary.detectorid() );
            set |= _parts[pSummary.detectorid()]->common_summary(pSummary);
        }
        for( const auto & pEntryPair : _parts ) {
            if( met.end() == met.find( pEntryPair.first ) ) {
                pEntryPair.second->reset_summary();
            }
        }
        return set;
    }
public:
    ReceptiveAssembly( const std::string & fn,
                       const std::string & dn ) :
                iDetector(fn, dn, false, true, false),
                Parent( fn, dn ) {}
    const PartsMap & parts() const { return _parts; }
    PartsMap & mutable_parts() { return _parts; }
};  // class CachedPayloadReceptiveDetector

}  // namespace alignment
}  // namespace sV

# endif  // H_STROMA_V_EVENT_DISPLAY_RECIPTIVE_DETECTOR_MIXIN_H

# endif // ALIGNMENT_ROUTINES

