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

namespace sV {
namespace alignment {

/**@class ReceptiveDetector
 * @brief Detector representation instance reacting on hits.
 *
 * Provides basics for keeping information about particular hits
 * in event.
 *
 * @ingroup alignment
 */
class ReceptiveDetector : public virtual iDetector {
public:
    typedef ::sV::events::DetectorSummary Hit;
protected:
    const Hit * _lastHitPtr;
    virtual bool _V_treat_new_hit( const Hit & ) = 0;
    virtual bool _V_reset_hits() = 0;
public:
    ReceptiveDetector( const std::string & fn,
                       const std::string & dn );

    /// Returns true when re-drawing need.
    bool hit( const Hit & hit_ ) { return _V_treat_new_hit( *(_lastHitPtr = &hit_) ); }
    const Hit & common_hit_data() { return *_lastHitPtr; }

    bool has_hit() const { return _lastHitPtr; }

    bool reset_hits() { return _V_reset_hits(); }
};  // class ReceptiveDetector

/**@brief Aux receptive detector instance performing unpacking routines for
 * certain summary payload.
 */
template<typename ConcreteHitT>
class CachedPayloadReceptiveDetector : public ReceptiveDetector {
protected:
    ConcreteHitT _reentrantHit;
protected:
    virtual bool _V_treat_new_hit( const Hit & hit_ ) override {
        _TODO_  // TODO
    }
public:
    CachedPayloadReceptiveDetector( const std::string & fn,
                                    const std::string & dn );
    const ConcreteHitT & hit() const { return _reentrantHit; }
};  // class CachedPayloadReceptiveDetector

/**@brief Aux receptive detector assembly instance performing unpacking
 * routines for certain summary payloads.
 */
template<typename ConcreteHitT>
class ReceptiveAssembly : public virtual
            CachedPayloadReceptiveDetector<::sV::events::AssemblySummary> {
protected:
    virtual bool _V_treat_new_hit( const Hit & hit_ ) override {
        _TODO_  // TODO
    }
public:
    ReceptiveAssembly( const std::string & fn,
                       const std::string & dn );
    const ConcreteHitT & hit() const { return _reentrantHit; }
};  // class CachedPayloadReceptiveDetector

}  // namespace alignment
}  // namespace sV

# endif  // H_STROMA_V_EVENT_DISPLAY_RECIPTIVE_DETECTOR_MIXIN_H

# endif // ALIGNMENT_ROUTINES

