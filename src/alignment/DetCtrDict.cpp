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

# include "alignment/DetCtrDict.hpp"

# ifdef ALIGNMENT_ROUTINES

# include "app/abstract.hpp"

namespace sV {
namespace alignment {

std::unordered_map<std::string, DetectorConstructorsDict::DetectorConstructor> *
DetectorConstructorsDict::_byName = nullptr;

DetectorConstructorsDict::DetectorConstructorsDict() {
    if( !_byName ) {
        sV_logw( "No detector constructors were automatically added.\n" );
    }
}

DetectorConstructorsDict::~DetectorConstructorsDict() {
    if( _byName ) {
        delete _byName;
        _byName = nullptr;
    }
}

void
DetectorConstructorsDict::register_detector_constructor( const std::string & name,
                                                         DetectorConstructor ctr ) {
    typedef std::remove_pointer<DECLTYPE(_byName)>::type DictType;
    if( !_byName ) {
        _byName = new DictType;
    }
    DictType & byName = *_byName;
    auto insertionResult = byName.insert( DictType::value_type( name, ctr ) );
    if( !insertionResult.second ) {
        emraise( nonUniq, "Unable to insert constructor \"%s\":%p.", name.c_str(), ctr );
    }
}

DetectorConstructorsDict::DetectorConstructor
DetectorConstructorsDict::get_constructor( const std::string & lookupName ) {
    assert( _byName );
    auto it = _byName->find( lookupName );
    if( _byName->end() != it ) {
        return it->second;
    }
    return nullptr;
}

void
DetectorConstructorsDict::list_constructors( std::ostream & os ) {
    os << "Available detector constructors:" << std::endl;
    if( !_byName || _byName->empty() ) {
        os << "  <no ctrs available>" << std::endl;
        return;
    }
    for( auto it  = _byName->begin(); it != _byName->end(); ++it ) {
        os << "  " << it->first << std::endl;
    }
}

}  // namespace alignment
}  // namespace sV

# endif

