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

# ifndef H_STROMA_V_ANALYSIS_DICTIONARY_H
# define H_STROMA_V_ANALYSIS_DICTIONARY_H

# include "sV_config.h"

# ifdef RPC_PROTOCOLS

# include "analysis/pipeline.hpp"
# include "app/abstract.hpp"

namespace sV {
namespace sys {

/**@class IndexOfConstructibles
 * @brief Dictionary indexing available data analysis routines.
 *
 * This dictionary is designed for automated run-time indexing of all data
 * sources and data processing handlers.
 *
 * For C++ applications one has to push this class into inheritance chain to
 * make sV::AbstractApplication append common configuration dictionary with
 * its supplementary options. For other use cases, one may consider to use
 * supp_options() manually.
 * */
class IndexOfConstructibles {
public:
    /// Callback type for functions that assemble configuration dictionary and
    /// its mapping to common config for constructable types enumerated in this
    /// dictionary.
    typedef std::pair<goo::dict::Dictionary, goo::dict::DictionaryInjectionMap>
                                (*OptionsSupplement)( goo::dict::Dictionary & );

    template<typename ConstructableT>
    struct EnumerableEntry {
        typedef ConstructableT * (*ConstructorT)( const goo::dict::Dictionary & );
        ConstructorT constructor;
        std::string  description;
        struct Arguments {
            goo::dict::Dictionary dict;
            goo::dict::DictionaryInjectionMap parametersInjector;
        } * arguments;
    };

    struct iConstructibleSectionBase { };

    template<class ConstructibleT> struct ConstructiblesSection : public iConstructibleSectionBase {
        std::unordered_map<std::string, EnumerableEntry<ConstructibleT> > byName;
    };
protected:
    /// by-TypeID index of constructible objects.
    /// The key refers to common parent type of constructible, thus, defining
    /// particular set (section) of constructible objects, e.g. data source
    /// formats, event treatment handlers, geometry shapes, etc.
    std::unordered_map<
                std::type_index,
                iConstructibleSectionBase *> _procsCtrDict;

    static AnalysisDictionary * _self;

    template<typename ConstructableT> ConstructiblesSection<ConstructableT> *
    _get_section( bool insertNonExisting=false );
public:
    /// Singleton getter.
    static AnalysisDictionary & self();

    /// Returns true if no analysis routines were enabled.
    static bool empty() { return !_self || _self->_procsCtrDict.empty(); }

    /// Inserts reader callback.
    template<typename ConstructableT> void add_constructor(
                const std::string &, const EnumerableEntry<ConstructableT> & );

    /// Returns known constructors for specified (as a template argument) base
    /// type.
    template<typename ConstructableT> void const ConstructiblesSection<ConstructableT> &
    known_constructors() const { return self()->_get_section<ConstructableT>(); }

    /// Looks for reader callback pointed out by string.
    template<typename ConstructableT> const EnumerableEntry<ConstructableT> & find( const std::string & );
};  // class IndexOfConstructibles


//
// Implementation

template<typename ConstructableT> ConstructiblesSection<ConstructableT> *
IndexOfConstructibles::_get_section( bool insertNonExisting ) {
    std::type_index tIndex = std::type_index(typeid(ConstructableT));
    auto sectionIt = _procsCtrDict.find( tIndex );
    if( _procsCtrDict.end() == sectionIt ) {
        if( insertNonExisting ) {
            sectionIt = _procsCtrDict.emplace(
                    tIndex, new ConstructiblesSection<ConstructableT>() ).first;
        } else {
            emraise( notFound, "Has no enumerated descendants of type %s.",
                typeid(ConstructableT).name() );
        }
    }
    return static_cast<ConstructiblesSection<ConstructableT> >( sectionIt.second );
}

template<typename ConstructableT>
void IndexOfConstructibles::add_constructor(
                const std::string & name,
                const EnumerableEntry<ConstructableT> & entry ) {
    // Get appropriate section (or insert it if it does not yet exist).
    auto sectPtr = _get_section<ConstructableT>( true );
    // Emplace new entry into by-name map for this ancestor:
    auto ir = sectPtr->emplace( name, entry );
    if( ! ir.second ) {
        sV_logw( "Omitting repeatative enumeration of entry \"%s\":%s.\n",
            name.c_str(), typeid(ConstructableT).name() );
    } else {
        sV_log2( "Constructor for type \"%s\":%s has been registered.\n",
            name.c_str(), typeid(ConstructableT).name() );
    }
}

template<typename ConstructableT> const EnumerableEntry<ConstructableT> &
IndexOfConstructibles::find( const std::string & name ) {
    auto sectPtr = _get_section<ConstructableT>();
    auto it = sectPtr->find( name );
    if( sectPtr->end() == it ) {
        emraise( notFound, "Unable to find constructor for \"%s\" with base type %s.\n",
            name.c_str(), typeid(ConstructableT).name() );
    }
    return it->second;
}


// Helper macros
///////////////

/**@def StromaV_DEFINE_DATA_PROCESSOR
 * @brief A data processing constructor implementation macro. */
# define StromaV_DEFINE_CONSTRUCTIBLE( cxxClassName, name, description )        \
static void __static_register_ ## ProcessorClass ## _prc() __attribute__ ((constructor(156)));  \
static void __static_register_ ## ProcessorClass ## _prc() {                    \
    sV::sys::add_constructor( txtName, __static_construct_ ## ProcessorClass, description ); }


/**@def StromaV_DEFINE_CONFIG_ARGUMENTS
 * @brief Supplementary configuration insertion macro for
 * \ref AnalysisDictionary . Performs appending of common config options.
 * Returns pair consisting of own config option dict and its injective mapping.
 * This mapping has to describe common config to own config parameters
 * correspondance. */
# define StromaV_DEFINE_CONFIG_ARGUMENTS( DNAME, ClassName )                \
static std::pair<goo::dict::Dictionary, goo::dict::DictionaryInjectionMap>  \
            _get_supp_options( goo::dict::Dictionary & );                   \
static void __static_register_args() __attribute__ ((constructor(156)));    \
static void __static_register_args() {                                      \
    sV::AnalysisDictionary::supp_options( _get_supp_options );}             \
static std::pair<goo::dict::Dictionary, goo::dict::DictionaryInjectionMap>  \
                _get_supp_options( goo::dict::Dictionary & DNAME )

}  // namespace sys
}  // namespace sV

# endif  // RPC_PROTOCOLS
# endif  // H_STROMA_V_ANALYSIS_DICTIONARY_H

