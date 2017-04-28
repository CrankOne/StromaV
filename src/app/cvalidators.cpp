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

//# include <boost/program_options.hpp>
# include <boost/lexical_cast.hpp>  // TODO: change to something from Goo's dicts

//namespace po = boost::program_options;

namespace sV {
namespace aux {

const std::string sciFltRxStr = "([-+]?[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?)",
                  sciRxStr1 = "(\\d+)\\[" + sciFltRxStr + "\\:" + sciFltRxStr + "\\]",
                  sciRxStr2 = sciRxStr1 + "x" + sciRxStr1
                  ;

# if 0
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
# endif

std::ostream &
operator<<( std::ostream & os, const HistogramParameters1D & p ) {
    os << "{" << p.nBins << "[" << p.min << ":" << p.max << "]}";
    return os;
}

std::ostream &
operator<<( std::ostream & os, const HistogramParameters2D & p ) {
    os << "{"
       << p.nBins[0] << "[" << p.min[0] << ":" << p.max[0] << "]"
       << "x"
       << p.nBins[1] << "[" << p.min[1] << ":" << p.max[1] << "]"
       << "}";
    return os;
}

}  // namespace aux
}  // namespace sV

namespace goo {
namespace dict {

Parameter<sV::aux::HistogramParameters2D>::Parameter( const char * name_,
               const char * description_,
               const sV::aux::HistogramParameters2D & ) :
                        DuplicableParent( name_,
                              description_,
                              0x0 | iAbstractParameter::set
                                  | iAbstractParameter::atomic
                                  | iAbstractParameter::singular,
                              '\0' ) {}

Parameter<sV::aux::HistogramParameters2D>::Value
Parameter<sV::aux::HistogramParameters2D>::_V_parse( const char * s ) const {
    static std::regex r( sV::aux::sciRxStr2 );
    std::smatch match;
    std::string sCpy(s);
    if( std::regex_match(sCpy, match, r) ) {
        try {
            return sV::aux::HistogramParameters2D(
                    boost::lexical_cast<int>(match[1]),
                    boost::lexical_cast<float>(match[2]),
                    boost::lexical_cast<float>(match[4]),
                    boost::lexical_cast<int>(match[6]),
                    boost::lexical_cast<float>(match[7]),
                    boost::lexical_cast<float>(match[9])
                );
        } catch( boost::bad_lexical_cast & e ) {
            std::cout << "Bad lexical cast while extracting tokens "
                         "1, 2, 4, 6, 7, 8 from \""
                      << s << "\":" << std::endl;
            for (size_t i = 0; i < match.size(); ++i) {
                std::ssub_match subMatch = match[i];
                std::string piece = subMatch.str();
                std::cout << "  submatch " << i << ": " << piece << '\n';
            }
        }
    }
    emraise( parserFailure, "Unable to interpret string \"%s\" as 2D "
        "histogram parameters. Expected form: "
        "<nBinX>[<minX>:<maxX>]x<nBinY>[<minY>:<maxY>].", s );
}

std::string
Parameter<sV::aux::HistogramParameters2D>::_V_stringify_value( const Value & v ) const {
    std::stringstream ss;
    ss << v;
    return ss.str();
}


Parameter<sV::aux::HistogramParameters1D>::Parameter( const char * name_,
               const char * description_,
               const sV::aux::HistogramParameters1D & ) :
                        DuplicableParent( name_,
                              description_,
                              0x0 | iAbstractParameter::set
                                  | iAbstractParameter::atomic
                                  | iAbstractParameter::singular,
                              '\0' ) {}

sV::aux::HistogramParameters1D
Parameter<sV::aux::HistogramParameters1D>::_V_parse( const char * s ) const {
    static std::regex r(sV::aux::sciRxStr1);
    std::smatch match;
    std::string sCpy(s);
    if( std::regex_match(sCpy, match, r) ) {
        try {
            return sV::aux::HistogramParameters1D(
                    boost::lexical_cast<int>(match[1]),
                    boost::lexical_cast<float>(match[2]),
                    boost::lexical_cast<float>(match[4])
                );
        } catch( boost::bad_lexical_cast & e ) {
            std::cout << "Bad lexical cast while extracting tokens 1, 2, 4 from \""
                      << s << "\":" << std::endl;
            for (size_t i = 0; i < match.size(); ++i) {
                std::ssub_match subMatch = match[i];
                std::string piece = subMatch.str();
                std::cout << "  submatch " << i << ": " << piece << '\n';
            }
        }
    }
    emraise( parserFailure, "Unable to interpret string \"%s\" as 1D "
        "histogram parameters. Expected form: <nBin>[<min>:<max>].", s );
}

std::string
Parameter<sV::aux::HistogramParameters1D>::_V_stringify_value( const Value & v ) const {
    std::stringstream ss;
    ss << v;
    return ss.str();
}

}  // namespace dict
}  // namespace goo

