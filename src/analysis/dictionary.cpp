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

# include "analysis/dictionary.hpp"

# ifdef RPC_PROTOCOLS
# include "app/app.h"

namespace sV {

// Readers
/////////

std::unordered_map<std::string, std::pair<AnalysisDictionary::EvSeqCtr, std::string> >  *
    AnalysisDictionary::_readersDict = nullptr;

void
AnalysisDictionary::add_reader( const std::string & key, EvSeqCtr ctr, const std::string & descr ) {
    if( !_readersDict ) {
        _readersDict = new std::remove_pointer<DECLTYPE(_readersDict)>::type;
    }
    auto insertionResult = _readersDict->emplace( key,
            std::pair<AnalysisDictionary::EvSeqCtr, std::string>( ctr, descr ) );
    if( !insertionResult.second ) {
        if( insertionResult.first->second.first != ctr ) {
            sV_loge( "Failed to register an event sequence reader "
                     "\"%s\" because this name isn't unique.\n", key.c_str() );
        } else {
            sV_log3( "Omitting repeatative insertion of event reader "
                     "\"%s\" (%p).\n", key.c_str(), ctr );
        }
    }
}

void
AnalysisDictionary::list_readers( std::ostream & os ) {
    if( !_readersDict || _readersDict->empty() ) {
        os << "No formats available." << std::endl;
    } else {
        os << "Available data formats:" << std::endl;
        for( auto it  = _readersDict->begin();
                  it != _readersDict->end(); ++it) {
            os << "  - " << it->first << std::endl
               << "  " << (void*) it->second.first
               << " : " << it->second.second << std::endl;
        }
    }
}

AnalysisDictionary::EvSeqCtr
AnalysisDictionary::find_reader( const std::string & key ) {
    if( !_readersDict ) {
        emraise( badState, "No readers registered." )
    }
    auto it = _readersDict->find(key);
    if( _readersDict->end() == it ) {
        emraise( notFound, "Format unknown: \"%s\".", key.c_str() );
    }
    return it->second.first;
}

// Processors
////////////

std::unordered_map<std::string, std::pair<AnalysisDictionary::EvProcCtr, std::string> > *
    AnalysisDictionary::_procsDict = nullptr;

void
AnalysisDictionary::add_processor( const std::string & key, EvProcCtr ctr, const std::string & descr ) {
    if( !_procsDict ) {
        _procsDict = new std::remove_pointer<DECLTYPE(_procsDict)>::type;
    }
    auto insertionResult = _procsDict->emplace( key,
            std::pair<AnalysisDictionary::EvProcCtr, std::string>( ctr, descr ) );
    if( !insertionResult.second ) {
        if( insertionResult.first->second.first != ctr ) {
            sV_loge( "Failed to register an event processor \"%s\" because "
                     "it's name isn't unique (adding %p vs known %p).\n",
                     key.c_str(), ctr, insertionResult.first->second.first );
        } else {
            sV_log3( "Omitting repeatative insertion of event processor "
                     "\"%s\" (%p).\n", key.c_str(), ctr );
        }
    }
}

void
AnalysisDictionary::list_processors( std::ostream & os ) {
    if( !_procsDict || _procsDict->empty() ) {
        os << "No processors available." << std::endl;
    } else {
        os << "Available data processors:" << std::endl;
        for( auto it  = _procsDict->begin();
                  it != _procsDict->end(); ++it) {
            os << "  - " << it->first << std::endl
               << "  " << (void*) it->second.first
               << " : " << it->second.second << std::endl;
        }
    }
}

AnalysisDictionary::EvProcCtr
AnalysisDictionary::find_processor( const std::string & key ) {
    if( !_procsDict ) {
        emraise( badState, "No processors registered." )
    }
    auto it = _procsDict->find(key);
    if( _procsDict->end() == it ) {
        emraise( notFound, "Processor unknown: \"%s\".", key.c_str() );
    }
    return it->second.first;
}

// Options
/////////

std::unordered_set<AnalysisDictionary::OptionsSupplement> *
AnalysisDictionary::_suppOpts = nullptr;

void
AnalysisDictionary::supp_options( OptionsSupplement optsSupp ) {
    if( !_suppOpts ) {
        _suppOpts = new std::unordered_set<OptionsSupplement>();
    }
    _suppOpts->insert( optsSupp );
}

}  // namespace sV

# endif  // RPC_PROTOCOLS
