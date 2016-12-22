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

# ifndef H_STROMA_V_XFORM_H
# define H_STROMA_V_XFORM_H

# include <cstdint>
# include <cmath>
# include <ostream>
# include <cmath>
# include <goo_xform.tcc>

/**@file xform.tcc
 * @brief A yet another implementation of rectangular region automatically
 * adjusting its limits to cover provided points.
 * */

namespace sV {
namespace aux {

# if 0
/**@class XForm
 * @brief D-dimensional scaler.
 *
 * Designed for lazy usage: basically one need initialize ranges in some way and then just
 * invoke norm() and renorm() to scale in and out the target vector instance.
 * */
template<uint8_t TD,
         typename FloatT=double>
class XForm {
public:
    /// Used for indexing ranges.
    enum RangeIdx { min = 0, max = 1 };
    constexpr static uint8_t D = TD;
    typedef FloatT Float;
private:
    bool _rangesSet;
    mutable bool _scalesValid;
    mutable double _scales[TD];
protected:
    /// Actual ranges storage.
    Float xRanges[2][TD];
public:
    /// Generic ctr.
    XForm( ) : _rangesSet(false), _scalesValid(false) {
        for( uint8_t d = 0; d < TD; d++ ) {
            xRanges[min][d] = NAN;
            xRanges[max][d] = NAN;
            _scales[d] = NAN;
        }
    }
    /// Ctr should be used for known ranges.
    XForm( Float * xMins, Float * xMaxs ) : _rangesSet(false), _scalesValid(false) {
        _rangesSet = true;
        for( uint8_t d = 0; d < TD; d++ ) {
            xRanges[min][d] = xMins[d];
            xRanges[max][d] = xMaxs[d];
            if( isnan(xRanges[min][d]) || isnan(xRanges[max][d]) ) {
                _rangesSet = false;
            }
        }
        recalculate_scales();
    }
    /** @brief Does (re-)calculating aux scales.
     *
     * Throws badState when ranges are unset;
     * Throws badValue when ranges are invalid (NAN or inf).
     */
    void recalculate_scales() const {
        if( !ranges_set() ) {
            emraise( badState, "XForm::recalculate_scales() -- no ranges set for %p.", (const void*) this );
        }
        for( uint8_t d = 0; d < TD; d++ ) {
            if(    !std::isfinite(xRanges[min][d])
                || !std::isfinite(xRanges[max][d])
                || xRanges[max][d] <= xRanges[min][d]) {
                emraise(badValue, "Bad range value for XForm %p, [%d], range: [%e, %e]",
                        (const void*) this, (int) d, xRanges[min][d], xRanges[max][d] );
            }
            _scales[d] = 1./(xRanges[max][d] - xRanges[min][d]);
        }
        _scalesValid = true;
    }

    /// Scales getter.
    const double * scales() const { return _scales; }

    /// Considers given vector as representative one for adjusting norming regions.
    virtual void update_ranges( const Float * x ) {
        for( uint8_t d = 0; d < TD; d++ ) {
            if( x[d] < xRanges[min][d] || isnan(xRanges[min][d]) ) {
                xRanges[min][d] = x[d];
            } else if( x[d] > xRanges[max][d] || isnan(xRanges[max][d]) ) {
                xRanges[max][d] = x[d];
            }
        }
        _rangesSet = true;
        _scalesValid = false;
    }

    /// Sets all ranges at once.
    virtual void set_ranges( const Float * xMins, const Float * xMaxs ) {
        for( uint8_t d = 0; d < TD; d++ ) {
            xRanges[min][d] = xMins[d];
            xRanges[max][d] = xMaxs[d];
            if( !(std::isfinite(xMins[d]) && std::isfinite(xMaxs[d]))
                    || xMins[d] >= xMaxs[d] ) {
                emraise( badValue, "XForm ranges for dimension #%d couldn't be set to [%e, %e] as "
                         "min >= max or one of bounds is not finite.",
                         (int) d, xMins[d], xMaxs[d] );
            }
        }
        _rangesSet = true;
        _scalesValid = false;
    }

    /// Returns true, when ranges are were set.
    bool ranges_set() const { return _rangesSet; }

    /// Shortuct getter for obtaining ranges.
    virtual Float get_limit( RangeIdx ridx, uint8_t d ) const {
        return xRanges[ridx][d];
    }

    /// Provides normalization (forward mapping).
    virtual void norm( Float * x ) const {
        if( !_scalesValid ) { recalculate_scales(); }
        for( uint8_t d = 0; d < TD; d++ ) {
            x[d] = (x[d] - xRanges[min][d])*_scales[d];
        }
    }

    /// Provides denormalization of normalized value (reverse mapping).
    virtual void renorm( Float * x ) const {
        if( !_scalesValid ) { recalculate_scales(); }
        for( uint8_t d = 0; d < TD; d++ ) {
            x[d] = xRanges[min][d] + x[d]/_scales[d];
        }
    }

    /// Checks, if given vector fits the expected range.
    virtual bool match( Float * x ) const {
        for( uint8_t d = 0; d < TD; d++ ) {
            if( x[d] >= xRanges[max][d] || x[d] <= xRanges[min][d] ) {
                return false;
            }
        }
        return true;
    }

    template<typename OperandFloatT>
    void extend_with( const XForm<TD, OperandFloatT> & o ) {
        # if 0
        printf("Extending: [(%e, %e)-(%e, %e)] <=> [(%e, %e)-(%e, %e)]\n",
                xRanges[min][0],     o.xRanges[min][0],
                xRanges[max][0],     o.xRanges[max][0],
                xRanges[min][1],     o.xRanges[min][1],
                xRanges[max][1],     o.xRanges[max][1] );
        # endif

        update_ranges( o.xRanges[min] );
        update_ranges( o.xRanges[max] );
    }
};

template<uint8_t TD, typename FloatT>
std::ostream & operator<<( std::ostream & os, const XForm<TD, FloatT> & xf ) {
    typedef XForm<TD, FloatT> XF;
    os << "{" << std::endl
       << "   type : " << "\"XForm<" << (int) TD << ">\"," << std::endl
       << "    ptr : " << &xf << "," << std::endl
       << " ranges : [" << std::endl;
    for( uint8_t d = 0; d < TD; ++d ) {
        os << "  [" << xf.get_limit(XF::min, d) << ", " << xf.get_limit(XF::max, d) << "]" << "," << std::endl;
    }
    os << "]," << std::endl;
    os << "}";
    return os;
}
# else

// TODO: rename it to avoid misunderstanding as this IS NOT an
// X-Form actually.
template<uint8_t TD,
         typename FloatT=double>
class XForm : public ::goo::aux::NormedMultivariateRange<goo::aux::Range<FloatT>, TD> {
public:
    //using ::goo::aux::NormedMultivariateRange<FloatT, TD>::AbstractRange::LimitIndex;
    enum RangeIdx { min = 0, max = 1 };
public:
    XForm() {
        for( uint8_t i = 0; i < TD; ++i ) {
            this->set_range_instance( i, *(new goo::aux::Range<FloatT>()) );
        }
    }
    ~XForm() {
        for( uint8_t i = 0; i < TD; ++i ) {
            delete &(this->range(i));
        }
    }
    // This methods are added to provide backward compatibility:
    void recalculate_scales() const { /* dop nothing */ }
    virtual void renorm( FloatT * x ) const { this->denorm( x ); }
    virtual void update_ranges( const FloatT * x ) { this->extend_to( x ); }
    virtual FloatT get_limit( RangeIdx ridx, uint8_t d ) const {
        auto & rangeRef = this->range(d);
        if( min == ridx ) {
            return rangeRef.lower();
        } else {
            return rangeRef.upper();
        }
    }
    virtual void set_ranges( const FloatT * xMins, const FloatT * xMaxs ) {
        this->set_limits( xMins, xMaxs );
    }
};

template<uint8_t TD, typename FloatT>
std::ostream & operator<<( std::ostream & /*os*/, const XForm<TD, FloatT> & /*xf*/ ) {
    _TODO_
}
# endif

}  // namespace aux
}  // namespace sV

# endif  // H_STROMA_V_XFORM_H

