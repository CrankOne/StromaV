/*
 * Copyright (c) 2016 Renat R. Dusaev <crank@qcrypt.org>
 * Author: Renat R. Dusaev <crank@qcrypt.org>
 * Author: Bogdan Vasilishin <togetherwithra@gmail.com>
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

# ifndef H_SV_DETS_NOT_SO_PRIMITIVE_SCORER_H
# define H_SV_DETS_NOT_SO_PRIMITIVE_SCORER_H

# include <cstdlib>
# include <cstring>
# include <cmath>
# include <cstdint>

# ifdef STANDALONE_BUILD
#   include <iostream>
# else
#   include "goo_exception.hpp"
# endif

namespace sV {
namespace aux {

// This snippet I wrote to test the small pairwise
// summation algorithm implementation. One can use
// it to figure out how the direct summation causes
// accumulation of the rounding error.

// Note: changes original array!
template<typename T,
         typename sizeT=size_t> T
pairwise_sum_( T * args, sizeT l ) {
    if(!l) return 0;
    for( sizeT factor = 1; factor < l; factor *= 2 ) {
        for( sizeT i = factor; i < l; i += factor ) {
            args[i-factor] += args[i];
        }
    }
    return args[0];
}

template<typename T,
         typename sizeT=size_t> T
pairwise_sum( const T * args, sizeT l ) {
    if(!l) return 0;
    sizeT s2 = floor(l/2.);
    T * cpy2 = new T [s2];
    bzero( cpy2, s2*sizeof(T) );
    //s2 -= l%2;
    for( sizeT i = 0; i < s2; ++i ) {
        cpy2[i] = args[2*i] + args[2*i+1];
    }
    if( l%2 ) {
        cpy2[s2-1] += args[l-1];
    }
    auto res = pairwise_sum_(cpy2, s2);
    delete cpy2;
    return res;
}

template<typename T,
         typename sizeT=size_t> T
kahan_sum( const T * args, sizeT l ) {
    double sum = 0., c = 0.;
    for( sizeT i = 0; i < l; ++i ) {
        double y = args[i] - c,
               t = sum + y;
        c = (t - sum) - y;
        sum = t;
    }
    return sum;
}

//
//
//

template<typename DataT=double,
         typename SizeT=uint64_t,
         DataT (*sumF)(const DataT *, SizeT)=kahan_sum >
class NotSoPrimitiveScorer {
public:
    typedef DataT Data;
    typedef SizeT Size;
protected:
    const Size _length;
    Data * const _valuesArrayPtr,
         * const _end;
    Data * _mutableCellsPtr,
         * _current;
protected:
    virtual void _V_archivate();
    virtual void _V_push_value( Data );
public:
    NotSoPrimitiveScorer( Size nNodes );
    NotSoPrimitiveScorer( const NotSoPrimitiveScorer & );
    ~NotSoPrimitiveScorer();

    /// Sums accumulated values.
    void archivate() { _V_archivate(); }

    /// Adds a value.
    void push_value( Data v ) { _V_push_value( v ); }

    /// Returns a sum.
    Data sum() const;

    /// Sets all scorer to zero.
    void reset();
};

// IMPLEMENTATION

template<typename DataT, typename SizeT, DataT (*sumF)(const DataT *, SizeT)> void
NotSoPrimitiveScorer<DataT, SizeT, sumF>::_V_archivate() {
    double rs = sumF( _mutableCellsPtr, _current - _mutableCellsPtr );
    *(_mutableCellsPtr++) = rs;
    _current = _mutableCellsPtr;
    if( _mutableCellsPtr >= _end ) {
        # ifdef STANDALONE_BUILD
        std::cerr << "Scorer memory pool depleted!" << std::endl;
        # else
        emraise( memAllocError, "Scorer memory pool depleted." );
        # endif
    }
}

template<typename DataT, typename SizeT, DataT (*sumF)(const DataT *, SizeT)> void
NotSoPrimitiveScorer<DataT, SizeT, sumF>::_V_push_value( Data v ) {
    if( _current == _end ) {
        archivate();
        _V_push_value( v );
    } else {
        *(_current++) = v;
    }
}

template<typename DataT, typename SizeT, DataT (*sumF)(const DataT *, SizeT)>
NotSoPrimitiveScorer<DataT, SizeT, sumF>::NotSoPrimitiveScorer( Size nNodes ) :
        _length(nNodes),
        _valuesArrayPtr( new Data [_length] ),
        _end( _valuesArrayPtr + _length     ),
        _mutableCellsPtr(nullptr),
        _current(nullptr) {
    _mutableCellsPtr = _current = _valuesArrayPtr;
    bzero( _valuesArrayPtr, sizeof(Data)*_length );
}

# if 0
template<typename DataT, typename SizeT, DataT (*sumF)(DataT *, SizeT)>
NotSoPrimitiveScorer<DataT, SizeT, sumF>::NotSoPrimitiveScorer( const NotSoPrimitiveScorer & o ) :
        _length(o._length),
        _currentNo(o._currentNo),
        _valuesArrayPtr(nullptr),
        _mutableCellsPtr(nullptr) {
    _valuesArrayPtr = _mutableCellsPtr = new Data [_length];
    bzero( _valuesArrayPtr, sizeof(Data)*_length );
    for( uint32_t i = 0; i < _length; ++i ) {
        _valuesArrayPtr[i] = o._valuesArrayPtr[i];
    }
}
# endif

template<typename DataT, typename SizeT, DataT (*sumF)(const DataT *, SizeT)>
NotSoPrimitiveScorer<DataT, SizeT, sumF>::~NotSoPrimitiveScorer() {
    if( _valuesArrayPtr ) {
        delete [] _valuesArrayPtr;
    }
}

template<typename DataT, typename SizeT, DataT (*sumF)(const DataT *, SizeT)> DataT
NotSoPrimitiveScorer<DataT, SizeT, sumF>::sum() const {
    return sumF( _mutableCellsPtr, _current - _mutableCellsPtr ) +
           sumF( _valuesArrayPtr,  _mutableCellsPtr - _valuesArrayPtr )
           ;

}

template<typename DataT, typename SizeT, DataT (*sumF)(const DataT *, SizeT)> void
NotSoPrimitiveScorer<DataT, SizeT, sumF>::reset() {
    _mutableCellsPtr = _current = _valuesArrayPtr;
    bzero( _valuesArrayPtr, sizeof(Data)*_length );
}

}  // namespace aux
}  // namespace sV

# endif  // H_sV_DETS_NOT_SO_PRIMITIVE_SCORER_H

