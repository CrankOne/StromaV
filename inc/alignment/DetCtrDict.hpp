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

# ifndef H_STROMA_V_EVENT_DISPLAY_DETECTOR_CONSTRUCTORS_DICTIONARY_H
# define H_STROMA_V_EVENT_DISPLAY_DETECTOR_CONSTRUCTORS_DICTIONARY_H

# include "../config.h"

# ifdef ALIGNMENT_ROUTINES

# include <unordered_map>

namespace sV {

namespace mixins {
class AlignmentApplication;
}  // namespace mixins

namespace alignment {

class iDetector;

class DetectorConstructorsDict {
public:
    typedef iDetector * (*DetectorConstructor)( const std::string & detName );
private:
    static std::unordered_map<std::string, DetectorConstructor> * _byName;
    DetectorConstructorsDict();
    ~DetectorConstructorsDict();
public:
    static void register_detector_constructor( const std::string &,
                                               DetectorConstructor );

    static void list_constructors( std::ostream & );

    DetectorConstructor get_constructor( const std::string & );

    friend class ::sV::mixins::AlignmentApplication;
};

}  // namespace alignment
}  // namespace sV

# define REGISTER_EVD_CONSTRUCTOR( className, ctrName )                         \
static ::sV::alignment::iDetector * __detector_ ## className ## _constructor  \
    ( const std::string & detName ) {                                           \
    return new className( ctrName, detName );                                   \
}                                                                               \
static void __ctr_register_alignment_detector ## className () __attribute__(( __constructor__(156) )); \
static void __ctr_register_alignment_detector ## className () {                 \
    sV::alignment::DetectorConstructorsDict::register_detector_constructor(   \
                    ctrName,                                                    \
                    __detector_ ## className ## _constructor );                 \
}

# endif  // ALIGNMENT_ROUTINES

# endif  // H_STROMA_V_EVENT_DISPLAY_DETECTOR_CONSTRUCTORS_DICTIONARY_H

