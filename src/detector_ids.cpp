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

# include "detector_ids.hpp"

# include "app/abstract.hpp"
# include <goo_exception.hpp>

# include <iomanip>
# include <sstream>
# include <cassert>

namespace sV {
namespace aux {

iDetectorIndex * iDetectorIndex::_self = nullptr;

iDetectorIndex::iDetectorIndex( iDetectorIndex * self_ ) {
    assert( !_self );
    assert( self_ );
    //fprintf( stderr, "XXX %p %p\n", _self, self_ );  // XXX
    _self = self_;
}

iDetectorIndex &
iDetectorIndex::mutable_self() {
    if( !_self ) {
        emraise( badArchitect,
                 "Detector indexing object is not defined." );
    }
    return *_self;
}

const iDetectorIndex &
iDetectorIndex::self() {
    if( !_self ) {
        emraise( badArchitect,
                 "Detector indexing object is not defined." );
    }
    return *_self;
}

AFR_DetMjNo
iDetectorIndex::mj_code( const char * strNm ) const {
    return _V_mj_code( strNm );
}

const char *
iDetectorIndex::name( AFR_DetMjNo n ) const {
    return _V_name(n);
}

}  // namespace aux

# ifdef DSuL
std::shared_ptr<DetectorSelector>
DetectorSelector::compile_new( const char * expression ) {
    _TODO_  // TODO
}

bool
DetectorSelector::matches( const AFR_DetSignature ) const {
    _TODO_  // TODO
}
# endif

}  // namespace sV

# if 1
AFR_DetMjNo detector_major_by_name( const char * /*mjName*/ )
    { _FORBIDDEN_CALL_; }
AFR_DetFamID family_id_by_name_dft( const char * /*famName*/ )
    { _FORBIDDEN_CALL_; }
AFR_DetMjNo compose_detector_major(
                            AFR_DetFamID /*fmID*/,
                            const struct sV_DSuL_MVarIndex * /*mvIdx*/ )
    { _FORBIDDEN_CALL_; }
void decode_minor_to_indexes( AFR_DetMnNo /*minorNo*/,
                              struct sV_DSuL_MVarIndex * /*mvIdx*/ )
    { _FORBIDDEN_CALL_; }
# endif

//
// Detector encoding tests
/////////////////////////

# define BOOST_TEST_NO_MAIN
//# define BOOST_TEST_MODULE Detector number encoding
# include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE( Det_ID_Encoding_suite )

BOOST_AUTO_TEST_CASE( EncodingTypesSizes ) {
    BOOST_REQUIRE(
               sizeof( AFR_DetMjNo ) + sizeof( AFR_DetMnNo ) 
            == sizeof(AFR_DetSignature)
        );
    BOOST_REQUIRE(
            sizeof( AFR_UniqueDetectorID )
            == sizeof(AFR_DetSignature)
        );
}

BOOST_AUTO_TEST_SUITE_END()

