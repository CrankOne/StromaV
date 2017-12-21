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

# ifndef H_STROMAV_PIPELINEASSEMBLER_H
# define H_STROMAV_PIPELINEASSEMBLER_H

# include "sV_config.h"

# include "analysis/pipeline.hpp"

# include <goo_dict/dict.hpp>

namespace sV {

//class iPipelineBuilder {
//public:
//    void delete_processors();
//};

/**@brief Pipeline assembilng class.
 * @class PipelineAssembler
 *
 * The class performs building of the analysis pipeline structure according to
 * given specification. The motivation of creating this class comes from
 * potential variety of messages passing through different pipeline types that
 * must be assembled into single generic pipeline object. This class provides
 * some basic helpers for that purpose.
 *
 * */
template< typename MessageT >
class PipelineBuilder : public std::vector<sV::aux::iEventProcessor<MessageT>*>  {
public:
    typedef MessageT Message;
    typedef ::pipet::Pipe<Message> ThePipeline;
    typedef sV::aux::iEventProcessor<Message> TheProcessor;
    typedef std::vector<sV::aux::iEventProcessor<MessageT>*> Parent;
public:
    /// Builds pipeline according to given specification.
    ThePipeline build_pipe( const goo::dict::iSingularParameter & pplDesign ) {
        ThePipeline ppl;
        for( auto pe : pplDesign.as_list_of<goo::dict::Dictionary>() ) {
            TheProcessor *procPtr;
            if( 1 != pe.parameters().size() ) {
                goo::dict::Dictionary *parametersSect =
                        pe.probe_subsection( "parameters" );
                if( !parametersSect ) {
                    // no parameters given --- use generic constructor
                    procPtr = generic_new<TheProcessor>( pe["name"].as<std::string>());
                } else {
                    procPtr = sV::sys::IndexOfConstructables::self()
                            .construct<TheProcessor>( pe["name"].as<std::string>()
                                                      , *parametersSect );
                }
            } else if( !strcmp( "pipe", pe.parameters().front()->name()) ) {
                _TODO_ // ... here be dragons: fork construction
            } else if( !strcmp( "fork", pe.parameters().front()->name()) ) {
                _TODO_ // ... here be dragons: sub-pipe construction
            }
            ppl.push_back( *procPtr );
            Parent::push_back( procPtr );
        }
    }
};

}  // namespace sV

# endif // H_STROMAV_PIPELINEASSEMBLER_H
