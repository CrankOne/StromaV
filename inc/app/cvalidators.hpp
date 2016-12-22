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

# ifndef H_STROMA_V_APPLICATION_BOOST_PO_CUSTOM_VALIDATORS_H
# define H_STROMA_V_APPLICATION_BOOST_PO_CUSTOM_VALIDATORS_H

# include <iostream>
# include <cstdint>
# include <regex>
# include <boost/lexical_cast.hpp>
# include <boost/optional.hpp>
# include <boost/program_options.hpp>

namespace sV {
namespace aux {

# if 0
//
// Spatial vector as a D-component parameter
//

template<uint8_t D, typename ComponentT>
struct CoordinateParameter {
public:
    typedef ComponentT Component;
    static const uint8_t d = D;
    Component x[D];
    static void validate(boost::any& v,
            const std::vector<std::string>& values,
            CoordinateParameter<D, ComponentT>*, int);
};

template<uint8_t D, typename ComponentT> void
CoordinateParameter<D, ComponentT>::validate(
            boost::any& v,
            const std::vector<std::string>& values,
            CoordinateParameter *,
            int ) {
    CoordinateParameter res;
    // Extract tokens from values string vector and populate Model struct.
    if( values.size() != D ) {
        throw boost::program_options::validation_error(
            "Empty vector provided.");
    }
    for( size_t n = 0; n < D; ++n ) {
        res.x[n] = boost::lexical_cast<ComponentT>(values.at(n));
    }
    v = res;
}
# endif


//
// Arbitrary length vector (need to be validated as one line in cfg file)
//

template<typename ComponentT>
struct ListParameter {
public:
    typedef ComponentT Component;
    std::vector<Component> values;
    //static void validate(boost::any& v, const std::vector<std::string>& values, ListParameter<Component>*, int);
};

template<typename ComponentT> void
/*ListParameter<ComponentT>::*/validate( boost::any& v,
          const std::vector<std::string>& values,
          ListParameter<ComponentT>*, int) {
    ListParameter<ComponentT> l;
    for(std::vector<std::string>::const_iterator it = values.begin();
            it != values.end();
            ++it) {
        std::stringstream ss(*it);
        std::copy(  std::istream_iterator<ComponentT>(ss),
                    std::istream_iterator<ComponentT>(),
                    back_inserter(l.values));
        if(!ss.eof()) {
            throw boost::program_options::validation_error(
                boost::program_options::validation_error::invalid_option_value,
                "Invalid numerical list specification provided.",
                *it);
        }
    }
    v = l;
}

struct HistogramParameters1D {
public:
    int nBins;
    float min, max;
    HistogramParameters1D( int nBins_, float min_, float max_ ) :
        nBins(nBins_), min(min_), max(max_) {}
    friend std::ostream & operator<<( std::ostream & os, const HistogramParameters1D & );
};

void validate(boost::any& v,
              const std::vector<std::string>& values,
              HistogramParameters1D * target_type, int);


struct HistogramParameters2D {
public:
    int nBins[2];
    float min[2], max[2];
    HistogramParameters2D( int nBins1, float min1, float max1,
                           int nBins2, float min2, float max2 ) :
        nBins{ nBins1, nBins2 }, min{min1, min2}, max{max1, max2} {}
    friend std::ostream & operator<<( std::ostream & os, const HistogramParameters2D & );
};

void validate(boost::any& v,
              const std::vector<std::string>& values,
              HistogramParameters2D * target_type, int);

}  // namespace aux
}  // namespace sV

# endif  /* H_STROMA_V_APPLICATION_BOOST_PO_CUSTOM_VALIDATORS_H */

