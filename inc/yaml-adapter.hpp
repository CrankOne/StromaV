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

# ifndef H_STROMA_V_YAML_CPP_CONFIG_PARSER_ADAPTER_H
# define H_STROMA_V_YAML_CPP_CONFIG_PARSER_ADAPTER_H

# include "sV_config.h"

# include "goo_dict/dict.hpp"

# include <yaml.h>

namespace sV {
namespace aux { 

void read_yaml_config_file_to_goo_dict( goo::dict::Dictionary &
                                      , const std::string filename
                                      , bool addUndeclared=false );

void read_yaml_node_to_goo_dict( goo::dict::Dictionary &
                               , const YAML::Node &
                               , const std::string nameprefix=""
                               , bool addUndeclared=false );

void read_yaml_node_to_goo_list( goo::dict::iSingularParameter &
                               , const YAML::Node &
                               , const std::string nameprefix="" );

}  // namespace ::sV::aux
}  // namespace sV

namespace goo {
namespace dict {

template<>
class Parameter<YAML::Node> : public
                        mixins::iDuplicable< iAbstractParameter,
                        Parameter<YAML::Node>,
                        iParameter<YAML::Node> > {
public:
    typedef typename DuplicableParent::Parent::Value Value;
public:
    /// Only long option ctr.
    Parameter( const char * name_,
               const char * description_ );

    Parameter( const Parameter<YAML::Node> & o ) : DuplicableParent( o ) {}
    friend class ::goo::dict::InsertionProxy;
protected:
    /// Sets parameter value from given string.
    virtual Value _V_parse( const char * ) const override;
    /// Returns set value.
    virtual std::string _V_stringify_value( const Value & ) const override;
public:
    /// Sets the internal YAML node value to copy of given one.
    void assign_copy_of( const YAML::Node & );
};

}  // namespace ::goo::dict
}  // namespace goo


# endif  // H_STROMA_V_YAML_CPP_CONFIG_PARSER_ADAPTER_H

