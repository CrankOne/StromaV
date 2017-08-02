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

# ifdef GEANT4_MC_MODEL
# include <G4ThreeVector.hh>
# endif

# include "utils.hpp"

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

# if 0
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
# endif

// The histogram parametization grammar:
// expr ::= '{' axes '}'
//
// axes ::= axis
//        | axis 'x' axes
//        ;
//
// axis ::= NBINS '[' MIN ':' MAX ']'
//      ;


struct HistogramParameters1D {
public:
    int nBins;
    float min, max;
    HistogramParameters1D( int nBins_, float min_, float max_ ) :
        nBins(nBins_), min(min_), max(max_) {}

    HistogramParameters1D() : HistogramParameters1D( 10, 0, 1 ) {}

    friend std::ostream & operator<<( std::ostream & os, const HistogramParameters1D & );
};

struct HistogramParameters2D {
public:
    int nBins[2];
    float min[2], max[2];
    HistogramParameters2D( int nBins1, float min1, float max1,
                           int nBins2, float min2, float max2 ) :
        nBins{ nBins1, nBins2 }, min{min1, min2}, max{max1, max2} {}

    HistogramParameters2D( ) : HistogramParameters2D( 10, 0, 1, 10, 0, 1 ) {}

    friend std::ostream & operator<<( std::ostream & os, const HistogramParameters2D & );
};

//void validate(boost::any& v,
//              const std::vector<std::string>& values,
//              HistogramParameters2D * target_type, int);

}  // namespace aux
}  // namespace sV

namespace goo {
namespace dict {

template<>
class Parameter<sV::aux::HistogramParameters2D> : public
                        mixins::iDuplicable< iAbstractParameter,
                        Parameter<sV::aux::HistogramParameters2D>,
                        iParameter<sV::aux::HistogramParameters2D>
                        # ifdef SWIG
                        , false, false
                        # endif
                        > {
public:
    typedef typename DuplicableParent::Parent::Value Value;
public:
    /// Only long option ctr.
    Parameter( const char * name_,
               const char * description_,
               const sV::aux::HistogramParameters2D & );

    Parameter( const Parameter<sV::aux::HistogramParameters2D> & o ) : DuplicableParent( o ) {}
    friend class ::goo::dict::InsertionProxy;
protected:
    /// Sets parameter value from given string.
    virtual Value _V_parse( const char * ) const override;
    /// Returns set value.
    virtual std::string _V_stringify_value( const Value & ) const override;
};

template<>
class Parameter<sV::aux::HistogramParameters1D> : public
                        mixins::iDuplicable< iAbstractParameter,
                        Parameter<sV::aux::HistogramParameters1D>,
                        iParameter<sV::aux::HistogramParameters1D>
                        # ifdef SWIG
                        , false, false
                        # endif
                        > {
public:
    typedef typename DuplicableParent::Parent::Value Value;
public:
    /// Only long option ctr.
    Parameter( const char * name_,
               const char * description_,
               const sV::aux::HistogramParameters1D & );

    Parameter( const Parameter<sV::aux::HistogramParameters1D> & o ) : DuplicableParent( o ) {}
    friend class ::goo::dict::InsertionProxy;
protected:
    /// Sets parameter value from given string.
    virtual Value _V_parse( const char * ) const override;
    /// Returns set value.
    virtual std::string _V_stringify_value( const Value & ) const override;
};


# ifdef GEANT4_MC_MODEL
template<>
class Parameter<G4ThreeVector> : public
                        mixins::iDuplicable< iAbstractParameter,
                            Parameter<G4ThreeVector>,
                            iParameter<G4ThreeVector>
                            # ifdef SWIG
                            , false, false
                            # endif
                            > {
public:
    typedef typename DuplicableParent::Parent::Value Value;
public:
    /// Only long option ctr.
    Parameter( const char * name_,
               const char * description_,
               const G4ThreeVector & ) :
                    DuplicableParent( name_,
                              description_,
                              0x0 | iAbstractParameter::set
                                  | iAbstractParameter::atomic
                                  | iAbstractParameter::singular,
                              '\0' ) {}

    Parameter( const Parameter<G4ThreeVector> & o ) : DuplicableParent( o ) {}
    friend class ::goo::dict::InsertionProxy;
protected:
    /// Sets parameter value from given string.
    virtual Value _V_parse( const char * str ) const override {
        return ::sV::aux::parse_g4_three_vector( str );
    }
    /// Returns set value.
    virtual std::string _V_stringify_value( const Value & v ) const override {
        char bf[128];
        snprintf( bf, sizeof(bf), "{%.3e,%.3e,%.3e}", v[0], v[1], v[2] );
        return bf;
    }
};
# endif

}  // namespace dict
}  // namespace goo

# endif  /* H_STROMA_V_APPLICATION_BOOST_PO_CUSTOM_VALIDATORS_H */

