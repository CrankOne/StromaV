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

# ifndef H_STROMA_V_ALIGNMENT_TRACKING_VOLUME_AUX_H
# define H_STROMA_V_ALIGNMENT_TRACKING_VOLUME_AUX_H

# include "../config.h"

# ifdef ALIGNMENT_ROUTINES

# include "analysis/xform.tcc"
# include "HitGraphics.hpp"
# include "app/app.h"

# include <unordered_map>
# include <list>
# include <cstring>
# include <goo_types.h>
# include <goo_exception.hpp>
# include <iterator>

namespace sV {
namespace alignment {

template<uint8_t D> class TrackReceptiveVolume;

struct AbstractTrackReceptiveVolume {
public:
    /// Should return false, if further consideration of point
    /// is to be interrupted. Second argument is always of type
    /// TrackReceptiveVolume::LocalCoordinates while first is
    /// pointer to callee object (this). Third argument is left for
    /// user data.
    typedef bool (*CacheUpdater)( AbstractTrackReceptiveVolume *, void *, void * );
private:
    TrackReceptiveVolume<2> * _2DCastCache;
    TrackReceptiveVolume<3> * _3DCastCache;
    std::list< std::pair<CacheUpdater, void*> >
                        _pointRecachingFunctionsInsertion,
                        _pointRecachingFunctionsRemoval;
protected:
    AbstractTrackReceptiveVolume() : _2DCastCache(nullptr),
                                     _3DCastCache(nullptr) {}

    virtual void _update_caches_on_insertion( void * );
    virtual void _update_caches_on_removing( void * );
public:
    virtual uint8_t n_dimensions() const = 0;
    virtual void open_new_set() = 0;
    virtual size_t n_sets() const = 0;
    virtual void n_sets( size_t ) = 0;

    virtual void add_updater_on_insert( CacheUpdater updater, void * userdata=nullptr )
            { _pointRecachingFunctionsInsertion.push_back( {updater, userdata} ); }

    virtual void add_updater_on_remove( CacheUpdater updater, void * userdata=nullptr )
            { _pointRecachingFunctionsRemoval.push_back( {updater, userdata} ); }

    template<uint8_t D> TrackReceptiveVolume<D> & as()
            { (void)(_2DCastCache); static_assert(2 == D || 3 == D, "Bad dimensions number"); }
    template<uint8_t D> const TrackReceptiveVolume<D> & as() const
            { (void)(_3DCastCache); static_assert(2 == D || 3 == D, "Bad dimensions number"); }
};  // struct AbstractTrackReceptiveVolume

/**@class TrackReceptiveVolume
 * @brief Represents volume in which track points can occur.
 *
 * Operates in terms of normed local coordinates (detector frame).
 * Acts like a circular buffer with runtime-variable capacitance for
 * sets of points. Each «set» should represent a result of track point
 * reconstruction — for some real detectors there can be more than one
 * track point or track point candidate.
 *
 * Design:
 * - Hits/track points reconstruction procedures:
 *  1. Upon beginning of container fillment, user code must invoke
 *     open_new_set() method.
 *  2. Each tracking entity (real detector or any abstract volume that
 *     can issue a track point) rents the instance of this class.
 *  3. On reconstruction done by user code it calls update_point() method
 *     each time it finds a hit or track point.
 * - Track line reconstruction procedures:
 *     Set filled by last event is available by most_recent_set(). Track
 *  line reconstruction code should use its track points (or, by choosing
 *  best hit) for further reconstruction.
 *
 *  The point of this container is to minimize heap operation procedures
 *  with small object and reduce segmentation lacks (as possible) by
 *  introducing reentrant objects. Considering this points it is desirable
 *  to provide sufficient reserved capacity.
 * */
template<uint8_t D>
class TrackReceptiveVolume : public AbstractTrackReceptiveVolume,
                             public aux::XForm<D, float> {
public:
    /// Hit point.
    struct LocalCoordinates {
        float amplitude, r[D];
    };

    /// Set of points.
    typedef std::list<LocalCoordinates *> PointSet;

    /// Array of sets type.
    typedef std::list<PointSet *> SetsStack;
private:
    /// Storage of free allocated point sets.
    std::list<PointSet *> _freeSets;

    /// Storage of free reentrant coordinates.
    std::list<LocalCoordinates *> _freePoints;

    /// Ordered list of point sets (unordered). Front - recent.
    SetsStack _pointSets;

    typename PointSet::iterator _cPoint;
protected:
    /// Shifts internal circular buffer by 1 or allocates new set.
    /// When there are free sets "rotation" will acquire one from
    /// free sets pool. Otherwise the elder one (from back) will
    /// be taken from occupied.
    void _rotate_sets() {
        PointSet * newPsPtr;
        if( !_freeSets.empty() ) {
            newPsPtr = _freeSets.back();
            _freeSets.pop_back();
            assert( newPsPtr );
        } else {
            assert( !_pointSets.empty() );  // n_sets() needed to be called.
            newPsPtr = _pointSets.back();
            _pointSets.pop_back();
            _clear_set( newPsPtr );
            assert( newPsPtr );
        }
        _pointSets.push_front( newPsPtr );
    }

    /// Sets up spare (or newly allocated) point.
    void _next_point() {
        LocalCoordinates * newPointPtr;
        if( _freePoints.empty() ) {
            newPointPtr = new LocalCoordinates;
        } else {
            newPointPtr = _freePoints.back();
            _freePoints.pop_back();
        }
        if( _pointSets.empty() ) {
            emraise( badState, "Points storage with point sets number "
                     "uninitialized up to point updating." );
        }
        _pointSets.front()->push_front( newPointPtr );
        _cPoint = _pointSets.front()->begin();
    }

    /// Returns spare (or newly allocates) point set ptr.
    /// Barely allocates new set.
    PointSet * _new_point_set() {
        return new PointSet;
    }

    /// Physically deletes set instance by ptr.
    void _delete_set( PointSet * s ) {
        delete s;
    }

    /// Erases all points in a set; does not insert it to free sets pool.
    void _clear_set( PointSet * ps ) {
        if( ps->empty() ) { return; }
        for( auto it = ps->begin(); ps->end() != it; ++it ) {
            //sV_loge("=== WWW === XXX XXX XXX XXX %p\n", this); // XXX
            _freePoints.push_back( *it );
            _update_caches_on_removing( *it );
        }
        ps->clear();
    }

    /// Actually deletes all spare points (from heap).
    void _delete_free_points() {
        for( auto * p : _freePoints ) {
            delete p;
        }
        _freePoints.clear();
    }

    /// Actually deletes all spare sets (from heap).
    void _delete_free_sets() {
        for( auto * s : _freeSets ) {
            _delete_set( s );
        }
        _freeSets.clear();
    }
public:
    /// Overrides ancestor interface's number-of-dimensions method.
    virtual uint8_t n_dimensions() const override { return D; }

    /// Ctr allocates internal storage of given length. Note, that
    /// nSets could be, in principle, set to 0, but it will lead to
    /// unpredicted behaviour other than n_sets(...) methods will be
    /// invoked.
    TrackReceptiveVolume(size_t poolLength=1024, size_t nSets=1) {
        for( size_t i = 0; i < poolLength; ++i ) {
            _freePoints.push_back( new LocalCoordinates );
        }
        n_sets(nSets);
    }

    /// Frees all dynamically-allocated objects.
    ~TrackReceptiveVolume() {
        _delete_free_points();
        _delete_free_sets();
        for( auto * s : _pointSets ) {
            PointSet & ps = *s;
            for( auto * p : ps ) {
                delete p;
            }
            ps.clear();
            delete s;
        }
        _pointSets.clear();
    }

    /// Changes number of point sets to be stored. On expansion
    /// new sets will be added to free sets pool. On truncation,
    /// the free sets pool will be truncated first; then the elder
    /// ones will be removed from occupied sets if needed. If
    /// required capacity is the same, nothing will be performed.
    virtual void n_sets( size_t n ) override {
        size_t oldSize = n_sets();
        # ifndef NDEBUG
        if( !n ) {
            sV_loge( "(Debug) Zero capacity for TrackReceptiveVolume cache required. "
                         "Setting it to 1. Check your code.\n" );
            n = 1;
        }
        # endif
        if( oldSize == n ) {
            return;  // keep capacity, do nothing
        } else if( oldSize > n ) {  // reduce list capacity (truncate)
            # if 1
            size_t remCntr = oldSize - n;
            //sV_log3( "#0 : remCntr = %zu; n = %zu; "
            //             "_freeSets.size() = %zu "
            //             "_pointSets.size() = %zu\n",
            //             remCntr, n, _freeSets.size(),
            //             _pointSets.size() );  // XXX
            for( auto it  = _freeSets.begin();
                      it != _freeSets.end() && remCntr; remCntr--, ++it ) {
                //sV_loge("=== 1 === XXX XXX XXX XXX %p\n", this); // XXX
                //_clear_set( *it );
                _delete_set( *it );
            }
            //sV_log3( "#1 : remCntr = %zu; n = %zu; "
            //             "_freeSets.size() = %zu "
            //             "_pointSets.size() = %zu\n",
            //             remCntr, n, _freeSets.size(),
            //             _pointSets.size() );  // XXX
            if( !remCntr ) {  // if there are still some to be left in free sets pool:
                auto it = _freeSets.begin();
                std::advance( it, n );
                _freeSets.erase( _freeSets.begin(), it );
                // sV_log3( "#2 : remCntr = %zu; n = %zu; "
                //          "_freeSets.size() = %zu "
                //          "_pointSets.size() = %zu\n",
                //          remCntr, n, _freeSets.size(),
                //          _pointSets.size() );  // XXX
            } else {
                _freeSets.clear();
                for( auto it = _pointSets.rbegin();
                     _pointSets.rend() != it && remCntr; remCntr--, ++it ) {
                    //sV_loge("=== === XXX XXX 2 XXX XXX\n"); // XXX
                    _clear_set( *it );
                    _delete_set( *it );
                }
                _pointSets.resize( n );
                // sV_log3( "#3 : remCntr = %zu; n = %zu; "
                //          "_freeSets.size() = %zu "
                //          "_pointSets.size() = %zu\n",
                //          remCntr, n, _freeSets.size(),
                //          _pointSets.size() );  // XXX
            }
            # else
            auto it = _pointSets.begin();
            std::advance( it, n );
            auto fromIt = it;
            for( ; it != _pointSets.end(); ++it ) {
                _clear_set( *it );
                _freeSets.push_back( *it );
            }
            _pointSets.erase( fromIt, _pointSets.end() );
            # endif
        } else {  // increase list capacity
            const size_t toAllocate = n - oldSize;
            for( size_t i = 0; i < toAllocate; ++i ) {
                _freeSets.push_back( _new_point_set() );
            }
        }
        // XXX:
        //sV_log3( "Changing point set capacity for track receptive volume %p "
        //                "from %zu to %zu.\n", this, oldSize, n );
    }

    /// Returns number of  to be stored in container.
    virtual size_t n_sets() const override
        { return _pointSets.size() + _freeSets.size(); }

    /// Fills new point in current set. Returns pointer to internal
    /// local coordinates instance that can be further used for caching.
    const LocalCoordinates * update_point( const LocalCoordinates & cs ) {
        if( _pointSets.empty() ) {
            open_new_set();
            assert( !_pointSets.empty() );
        }
        _next_point();
        LocalCoordinates & c = **_cPoint;
        memcpy( &c, &cs, sizeof(LocalCoordinates) );
        _update_caches_on_insertion(&c);
        return &c;
    }

    /// Shifts sets buffer.
    virtual void open_new_set() override {
        _rotate_sets();
    }

    bool most_recent_set_is_empty() const {
        return _pointSets.empty() || _pointSets.front()->empty();
    }

    /// Getter for all point sets:
    const SetsStack & sets() const { return _pointSets; }
};  // class TrackReceptiveVolume

template<> TrackReceptiveVolume<2> & AbstractTrackReceptiveVolume::as<2>();
template<> TrackReceptiveVolume<3> & AbstractTrackReceptiveVolume::as<3>();
template<> const TrackReceptiveVolume<2> & AbstractTrackReceptiveVolume::as<2>() const;
template<> const TrackReceptiveVolume<3> & AbstractTrackReceptiveVolume::as<3>() const;

}  // namespace alignment
}  // namespace sV

# endif  // ALIGNMENT_ROUTINES

# endif  // H_STROMA_V_ALIGNMENT_TRACKING_VOLUME_AUX_H

