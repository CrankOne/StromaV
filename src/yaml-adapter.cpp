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

# include "app/app.h"

# include "yaml-adapter.hpp"

# include "goo_dict/dict.hpp"
# include "goo_exception.hpp"

namespace sV {
namespace aux {

void
read_yaml_config_file_to_goo_dict( goo::dict::Dictionary & D
                                 , const std::string filename
                                 , bool addUndeclared ) {
    try {
        std::vector<YAML::Node> yNodes = YAML::LoadAllFromFile( filename );
        if( yNodes.empty() ) {
            sV_logw( "The config file \"%s\" is empty.\n",
                    filename.c_str() );
        }
        for( auto yNode : yNodes ) {
            if( YAML::NodeType::Map != yNode.Type() ) {
                emraise( badParameter, "YAML document root node(s) is "
                    "expected to be a map(s)." );
            }
            read_yaml_node_to_goo_dict( D, yNode, "", addUndeclared );
        }
    } catch ( YAML::Exception & e ) {
        emraise( parserFailure, "While parsing \"%s\" error: %s.",
            filename.c_str(), e.what() );
    }
}

void
read_yaml_node_to_goo_dict( goo::dict::Dictionary & D
                          , const YAML::Node & node
                          , const std::string nameprefix
                          , bool addUndeclared ) {
    for( auto subNode : node ) {
        const std::string nName = subNode.first.as<std::string>(),
                          nodePath = nameprefix + nName
                          ;
        switch( subNode.second.Type() ) {
            case YAML::NodeType::Map : {
                goo::dict::Dictionary * dPtr = 
                                        D.probe_subsection( nName.c_str() );
                if( dPtr ) {
                    read_yaml_node_to_goo_dict( *dPtr
                                              , subNode.second
                                              , nodePath + "."
                                              , addUndeclared );
                } else if( D.probe_parameter( nName.c_str() ) ) {
                    // Means dynamic node: iSingularParameter<YAML::Node>
                    static_cast<::goo::dict::Parameter<YAML::Node>&>(D.parameter(nName))
                            .assign_copy_of( node );
                } else {
                    if( !addUndeclared ) {
                        sV_log3( "Unused config subsection: \"%s\".\n"
                               , nodePath.c_str());
                    } else {
                        auto subSect = new goo::dict::Dictionary( nName.c_str()
                                                                , "<YAML injection>" );
                        read_yaml_node_to_goo_dict( *subSect
                                                  , subNode.second
                                                  , nodePath + "."
                                                  , addUndeclared );
                        D.insert_section( subSect );
                    }
                }
            } break;
            case YAML::NodeType::Sequence : {
                auto pPtr = D.probe_parameter( nName.c_str() );
                if( pPtr ) {
                    read_yaml_node_to_goo_list( *pPtr,
                                                subNode.second,
                                                nodePath + "." );
                } else {
                    if( !addUndeclared ) {
                        sV_log3( "Unused list parameter: \"%s\".\n"
                               , nodePath.c_str());
                    } else {
                        // Try to infer node type based on node topology (check
                        // whether they're maps or scalars itself).
                        bool hasSequence = false
                           , hasMap = false
                           ;
                        for( auto entry : subNode.second ) {
                            hasSequence |= entry.IsSequence();
                            hasMap |= entry.IsMap();
                            if( hasSequence || hasMap ) continue;
                            if( !entry.IsScalar() && ! entry.IsNull() ) {
                                emraise( badValue, "Unable to recognize type "
                                        "of node \"%s.%s.%s\"."
                                       , nodePath.c_str()
                                       , nName.c_str()
                                       , subNode.first.as<std::string>().c_str() );
                            }
                        }
                        if( hasSequence ) {
                            if( hasMap ) {
                                // Generally, YAML does support such case, but
                                // practical use case is not clear.
                                emraise( badState, "Unable to defer type for "
                                        "YAML sequence node %s.%s: child nodes "
                                        "contains sequence AND map."
                                       , nodePath.c_str()
                                       , nName.c_str() );
                            }
                            // Note: generally speaking, there are ways to
                            // introduce such a support, but the particular use
                            // cases fot this remains unclear, again.
                            emraise( unimplemented, "sV YAML to goo::dict "
                                     "adapter does not support "
                                     "multidimensional arrays (node %s.%s)."
                                    , nodePath.c_str()
                                    , nName.c_str() );
                        }
                        _TODO_ // TODO
                        // ...
                    }
                }
            } break;
            case YAML::NodeType::Scalar: {
                auto pPtr = D.probe_parameter( nName.c_str() );
                if( pPtr ) {
                    pPtr->parse_argument( subNode.second.as<std::string>().c_str() );
                } else {
                    sV_log3( "Unused config parameter: \"%s\".\n",
                                nodePath.c_str() );
                }
            } break;
            case YAML::NodeType::Undefined : {
                _TODO_  // TODO
            } break;
            case YAML::NodeType::Null : {
                _TODO_  // TODO
            } break;
        }
    }
}

void
read_yaml_node_to_goo_list( goo::dict::iSingularParameter & genList
                          , const YAML::Node & node
                          , const std::string /*nameprefix*/ ) {
    for( auto subNode : node ) {
        if( !subNode.IsScalar() ) {
            emraise( unimplemented, "sV YAML config parser doesn't support "
                "list entries other than scalar values." );
        }
        genList.parse_argument( subNode.as<std::string>().c_str() );
    }
}

}  // namespace aux
}  // namespace sV

namespace goo {
namespace dict {


Parameter<YAML::Node>::Parameter( const char * name_,
               const char * description_ ) : DuplicableParent( name_,
                              description_,
                              0x0 | iAbstractParameter::atomic
                                  | iAbstractParameter::singular,
                              '\0' ) {}

Parameter<YAML::Node>::Value
Parameter<YAML::Node>::_V_parse( const char * ) const {
    _FORBIDDEN_CALL_  // TODO
}

std::string
Parameter<YAML::Node>::_V_stringify_value( const Value & ) const {
    // todo: stringify YMAL node
    return "<YAML-node>";
}

void
Parameter<YAML::Node>::assign_copy_of( const YAML::Node & orig ) {
    _set_value( YAML::Clone( orig ) );
}

}  // namespace ::goo::dict
}  // namespace goo

