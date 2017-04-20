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

# include "app/abstract.hpp"
# include "app/cvalidators.hpp"

# include <boost/program_options.hpp>

namespace po = boost::program_options;

namespace sV {
namespace aux {

const std::string sciFltRxStr = "([-+]?[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?)",
                  sciRxStr1 = "(\\d+)\\[" + sciFltRxStr + "\\:" + sciFltRxStr + "\\]",
                  sciRxStr2 = sciRxStr1 + "x" + sciRxStr1
                  ;

void
validate(boost::any& v,
              const std::vector<std::string>& values,
              HistogramParameters1D * /*target_type*/, int) {
    static std::regex r(sciRxStr1);
    po::validators::check_first_occurrence(v);
    // Extract the first string from 'values'. If there is more than
    // one string, it's an error, and exception will be thrown.
    const std::string& s = po::validators::get_single_string(values);
    // Do regex match and convert the interesting parts.
    std::smatch match;
    if( std::regex_match(s, match, r) ) {
        v = boost::any( HistogramParameters1D(
                boost::lexical_cast<int>(match[1]),
                boost::lexical_cast<float>(match[2]),
                boost::lexical_cast<float>(match[4])
            ) );
        # if 0
        std::cout << s << '\n';
        for (size_t i = 0; i < match.size(); ++i) {
            std::ssub_match subMatch = match[i];
            std::string piece = subMatch.str();
            std::cout << "  submatch " << i << ": " << piece << '\n';
        }
        # endif
    } else {
        throw po::validation_error(po::validation_error::invalid_option_value);
    }
}

void
validate(boost::any& v,
         const std::vector<std::string>& values,
         HistogramParameters2D * /*target_type*/, int) {
    static std::regex r(sciRxStr2);
    po::validators::check_first_occurrence(v);
    // Extract the first string from 'values'. If there is more than
    // one string, it's an error, and exception will be thrown.
    const std::string& s = po::validators::get_single_string(values);
    // Do regex match and convert the interesting parts.
    std::smatch match;
    if( std::regex_match(s, match, r) ) {
        # if 1
        v = boost::any( HistogramParameters2D(
                boost::lexical_cast<int>(match[1]),
                boost::lexical_cast<float>(match[2]),
                boost::lexical_cast<float>(match[4]),
                boost::lexical_cast<int>(match[6]),
                boost::lexical_cast<float>(match[7]),
                boost::lexical_cast<float>(match[9])
            ) );
        # else
        std::cout << s << '\n';
        for (size_t i = 0; i < match.size(); ++i) {
            std::ssub_match subMatch = match[i];
            std::string piece = subMatch.str();
            std::cout << "  submatch " << i << ": " << piece << '\n';
        }
        # endif
    } else {
        throw po::validation_error(po::validation_error::invalid_option_value);
    }
}

std::ostream &
operator<<( std::ostream & os, const HistogramParameters1D & p ) {
    os << "{" << p.nBins << "[" << p.min << ":" << p.max << "]}";
    return os;
}

std::ostream &
operator<<( std::ostream & os, const HistogramParameters2D & p ) {
    os << "{"
       << p.nBins[0] << "[" << p.min[0] << ":" << p.max[0] << "]"
       << p.nBins[1] << "[" << p.min[1] << ":" << p.max[1] << "]"
       << "}";
    return os;
}

}  // namespace aux
}  // namespace sV

