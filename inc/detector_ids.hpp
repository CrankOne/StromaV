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

# ifndef H_STROMA_V_DETECTOR_IDENTIFIERS_MAPPER_CLASS_H
# define H_STROMA_V_DETECTOR_IDENTIFIERS_MAPPER_CLASS_H

# include "sV_config.h"

# include "app/analysis.hpp"
# include "app/app.h"
# include "analysis/xform.tcc"
# include "detector_ids.h"

# include <unordered_map>

namespace sV {
namespace aux {

/**@class iDetectorIndex
 * @brief Detector IDs indexing instance.
 *
 * As for sV itself, the class can not be instantiated. It is expected from
 * user's code to inherit and implement this singleton interface.
 * */
class iDetectorIndex {
private:
    static iDetectorIndex * _self;
protected:
    /// Returns major number code for particular detector name.
    virtual AFR_DetMjNo _V_mj_code( const char * ) const = 0;
    /// Returns detector name string for given major code.
    virtual const char * _V_name( AFR_DetMjNo ) const = 0;

    iDetectorIndex( iDetectorIndex * self_ );
public:
    static iDetectorIndex & mutable_self();
    static const iDetectorIndex & self();

    /// Returns major number code for particular detector name.
    AFR_DetMjNo mj_code( const char * ) const;
    /// Returns detector name string for given major code.
    const char * name( AFR_DetMjNo ) const;
};  // class iDetectorIndex

}  // namespace aux

# ifdef DSuL

/**@class Selector
 * @brief A C++ wrapper around C "sV_DSuL_Expression" struct.
 */
class DetectorSelector : protected sV_DSuL_Selector {
protected:
    DetectorSelector( sV_DSuL_Selector & compiledInstance );
public:
    static std::shared_ptr<DetectorSelector> compile_new( const char * expression );

    bool matches( const AFR_DetSignature ) const;

    bool operator()( const AFR_DetSignature s ) const {
        return matches(s); }
};  // class DetectorSelector

# endif

}  // namespace sV


# endif  // H_STROMA_V_DETECTOR_IDENTIFIERS_MAPPER_CLASS_H

