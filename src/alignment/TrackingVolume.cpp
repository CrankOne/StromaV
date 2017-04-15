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

# include "sV_config.h"

# ifdef ALIGNMENT_ROUTINES

# include <cassert>
# include "alignment/TrackingGroup.hpp"

namespace sV {
namespace alignment {

void
AbstractTrackReceptiveVolume::_update_caches_on_insertion( void * pointPtr ) {
    for( auto it  = _pointRecachingFunctionsInsertion.begin();
         _pointRecachingFunctionsInsertion.end() != it; ++it ) {
        if( !it->first( this, pointPtr, it->second ) ) {
            break;
        }
    }
}

void
AbstractTrackReceptiveVolume::_update_caches_on_removing( void * pointPtr ) {
    for( auto it  = _pointRecachingFunctionsRemoval.begin();
         _pointRecachingFunctionsRemoval.end() != it; ++it ) {
        if( !it->first( this, pointPtr, it->second ) ) {
            break;
        }
    }
}

template<> TrackReceptiveVolume<2> &
AbstractTrackReceptiveVolume::as<2>() {
    if( !_2DCastCache ) {
        if( ! (_2DCastCache = dynamic_cast<TrackReceptiveVolume<2>*>(this)) ) {
            emraise(badCast, "Cast of 3D track receptive volume to 2D")
        }
    }
    return *_2DCastCache;
}

template<> TrackReceptiveVolume<3> &
AbstractTrackReceptiveVolume::as<3>() {
    if( !_3DCastCache ) {
        if( ! (_3DCastCache = dynamic_cast<TrackReceptiveVolume<3>*>(this)) ) {
            emraise(badCast, "Cast of 2D track receptive volume to 3D")
        }
    }
    return *_3DCastCache;
}

template<> const TrackReceptiveVolume<2> &
AbstractTrackReceptiveVolume::as<2>() const {
    return const_cast<AbstractTrackReceptiveVolume*>(this)->as<2>();
}

template<> const TrackReceptiveVolume<3> &
AbstractTrackReceptiveVolume::as<3>() const {
    return const_cast<AbstractTrackReceptiveVolume*>(this)->as<3>();
}

//
// Testing
/////////

# ifdef STANDALONE_BUILD
# ifdef NDEBUG
# error "This code implies that C-style assert() is available. Re-compile withoud NDEBUG macro."
# endif
void
test_pointset( FILE * outstream,
               const size_t nSets,
               const size_t nPointsInStorage ) {
    aux::TrackReceptiveVolume<2> trv2d;
    trv2d.n_sets( nSets );
    aux::AbstractTrackReceptiveVolume & interfaceRef = trv2d;

    float *** sequence = new float ** [nSets];
    for( size_t i = 0; i < nSets; ++i ) {
        size_t nPoints = 2 + 4*double(rand())/RAND_MAX;
        // /////////////
        interfaceRef.open_new_set();  // initializes set
        // /////////////
        sequence[i] = new float * [ nPoints ];
        sequence[i][0] = new float [1];
        *sequence[i][0] = nPoints;
        for( size_t n = 1; n < nPoints; ++n ) {
            sequence[i][n] = new float [2];
            sequence[i][n][0] = rand()/double(RAND_MAX);
            sequence[i][n][1] = rand()/double(RAND_MAX);
            // /////////////
            interfaceRef.as<2>().update_point(
                        aux::TrackReceptiveVolume<2>::LocalCoordinates
                        { 0., sequence[i][n][0], sequence[i][n][1]} );
            // /////////////
        }

        // Ensure, this set is filled as expected:
        const aux::TrackReceptiveVolume<2>::PointSet & ps = interfaceRef.as<2>().most_recent_set();
        fprintf( outstream, "New set: %zu - 1 == %zu:\n", nPoints, ps.size() );
        assert( nPoints - 1 == ps.size() );
        size_t n = 1;
        for( auto it  = ps.rbegin();
                  it != ps.rend(); ++it, ++n ) {
            fprintf( outstream, "%zu : %f <-> %f ; %f <-> %f\n", n,
                    (*it)->r[0], sequence[i][n][0],
                    (*it)->r[1], sequence[i][n][1] );
            assert( (*it)->r[0] == sequence[i][n][0] );
            assert( (*it)->r[1] == sequence[i][n][1] );
        }
    }

    // Check last 100 sets:
    size_t i = nSets - 1;
    for( auto it = interfaceRef.as<2>().sets().begin(); it != interfaceRef.as<2>().sets().end(); ++it, --i ) {
        const aux::TrackReceptiveVolume<2>::PointSet & psRef = **it;
        size_t n = 1;
        for( auto pIt = psRef.rbegin(); pIt != psRef.rend(); ++pIt, ++n ) {
            fprintf( outstream, "%zu (check) : %f <-> %f ; %f <-> %f\n", n,
                    (*pIt)->r[0], sequence[i][n][0],
                    (*pIt)->r[1], sequence[i][n][1] );
            assert( (*pIt)->r[0] == sequence[i][n][0] );
            assert( (*pIt)->r[1] == sequence[i][n][1] );
            //printf( "%zu %f %f\n", i, (*pIt)->r[0], sequence[i][n][0] );
            delete [] sequence[i][n];
        }
        delete [] sequence[i];
    }
    delete [] sequence;
    sequence = nullptr;
}
# endif  // STANDALONE_BUILD

# if 0
PointSet::PointSet() :
            _nativePSPtr(new TEvePointSet()),
            _persistency(0) {
    _nativePSPtr->SetOwnIds( kTRUE );
    //_nativePSPtr->SetNextPoint( 0., 0., -depthPreshower/2 );  // TODO?
    // some default settings:
    _nativePSPtr->SetMarkerColor(2);  // (red)
    _nativePSPtr->SetMarkerSize(9);   // (pretty large)
    _nativePSPtr->SetMarkerStyle(5);  // (3-cross)
}
# endif

}  // namespace alignment
}  // namespace sV

# ifdef STANDALONE_BUILD
int
main(int argc, char * argv[]) {
    sV::alignment::test_pointset( stdout, 256, 128 );
    sV::alignment::test_pointset( stdout, 64,  128 );
    sV::alignment::test_pointset( stdout, 128, 128 );
    fprintf( stdout, "Ok.\n" );
    return 0;
}
# endif

# endif  // ALIGNMENT_ROUTINES

