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

# ifndef H_STROMA_V_CONSTRUCTORS_DICTIONARY_H
# define H_STROMA_V_CONSTRUCTORS_DICTIONARY_H

# include "sV_config.h"

# ifdef RPC_PROTOCOLS

# include "app/app.h"

# include <goo_dict/dict.hpp>
# include <goo_dict/injection.hpp>
# include <goo_exception.hpp>

# include <typeinfo>
# include <typeindex>

namespace sV {
namespace sys {

/**@class IndexOfConstructables
 * @brief Dictionary indexing available data analysis routines.
 *
 * This dictionary is designed for automated run-time indexing of all data
 * sources and data processing handlers and expresses a factory method pattern.
 *
 * Speaking about the motivation for this singleton to be introduced, we have
 * had to provide a location available at the runtime where various dynamically
 * loaded classes may register their polymorphic constructors. Besides of
 * immediate convenience in C++ this class appeared to become extremely useful
 * for wrappers code as it does involve data introspection for construction
 * routines.
 *
 * In terms of architectural pattern (factory):
 *  - Instead of using dedicated product creator classes, we're offering the
 * function callbacks referred by name (string). The functions implementing
 * those callbacks in most cases will be short static routines and won't
 * pollute code much with any extra definition.
 *  - The `IndexOfConstructables` indexes multiple abstract product classes
 * available in running applications. The set of abstract products may be
 * extended at run time.
 * */
class IndexOfConstructables {
public:
    struct AbstractEnumerableEntry {
        goo::dict::Dictionary arguments;
        AbstractEnumerableEntry( const goo::dict::Dictionary & d ) : arguments(d){}
    };

    typedef std::unordered_map<std::string, AbstractEnumerableEntry *> ConstructablesSection;

    template<typename ConstructableT>
    struct EnumerableEntry : public AbstractEnumerableEntry {
        typedef ConstructableT * (*ConstructorT)( const goo::dict::Dictionary & );
        ConstructorT constructor;
        EnumerableEntry( ConstructorT ctr, const goo::dict::Dictionary & args ) :
            AbstractEnumerableEntry(args), constructor(ctr) {}
    };
private:
    static IndexOfConstructables * _self;
protected:
    /// by-TypeID index of constructable objects.
    /// The key refers to common parent type of constructable, thus, defining
    /// particular set (section) of constructable objects, e.g. data source
    /// formats, event treatment handlers, geometry shapes, etc.
    std::unordered_map<std::type_index, ConstructablesSection> _procsCtrDict;

    template<typename ConstructableT> ConstructablesSection &
    _get_section( bool insertNonExisting=false );
public:
    /// Singleton getter.
    static IndexOfConstructables & self();

    /// Returns whether or not any constructable was registered.
    static bool empty() { return !_self; }

    /// Inserts reader callback.
    template<typename ConstructableT> void add_constructor(
                const std::string &, EnumerableEntry<ConstructableT> * );

    /// Returns known constructors for specified (as a template argument) base
    /// type.
    template<typename ConstructableT> const ConstructablesSection &
    known_constructors() const {
        if( _self ) {
            return self()._get_section<ConstructableT>();
        } else {
            emraise( singletonRepCtr, "No constructable instances were "
                "defined upon section retrieval." );
        }
    }

    /// Looks for entry. Raises notFound on failure.
    template<typename ConstructableT> const EnumerableEntry<ConstructableT> &
    find( const std::string & );

    /// Returns constructor's dictionary for referenced type. Useful for
    /// runtime inspection of an available types. May return NULL if no
    /// constructors where added for such base type.
    const ConstructablesSection * constructors_for( const std::type_index & ) const;

    /// In-line construct.
    template<typename ConstructableT> ConstructableT *
    construct( const std::string &, const goo::dict::Dictionary & );
};  // class IndexOfConstructables

//
// Implementation

template<typename ConstructableT> IndexOfConstructables::ConstructablesSection &
IndexOfConstructables::_get_section( bool insertNonExisting ) {
    std::type_index tIndex = std::type_index(typeid(ConstructableT));
    auto sectionIt = _procsCtrDict.find( tIndex );
    if( _procsCtrDict.end() == sectionIt ) {
        if( insertNonExisting ) {
            sectionIt = _procsCtrDict.emplace(
                    tIndex, ConstructablesSection() ).first;
        } else {
            emraise( notFound, "Has no enumerated descendants of type %s.",
                typeid(ConstructableT).name() );
        }
    }
    return sectionIt->second;
}

template<typename ConstructableT>
void IndexOfConstructables::add_constructor(
                const std::string & name,
                EnumerableEntry<ConstructableT> * entry ) {
    // Get appropriate section (or insert it if it does not yet exist).
    auto & sectRef = _get_section<ConstructableT>( true );
    // Emplace new entry into by-name map for this ancestor:
    auto ir = sectRef.emplace( name, entry );
    if( ! ir.second ) {
        sV_logw( "Omitting repeatative enumeration of entry \"%s\":%s.\n",
            name.c_str(), typeid(ConstructableT).name() );
    } else {
        sV_log2( "Constructor for type \"%s\":%s has been registered.\n",
            name.c_str(), typeid(ConstructableT).name() );
    }
}

template<typename ConstructableT> const IndexOfConstructables::EnumerableEntry<ConstructableT> &
IndexOfConstructables::find( const std::string & name ) {
    auto & sectRef = _get_section<ConstructableT>();
    auto it = sectRef.find( name );
    if( sectRef.end() == it ) {
        emraise( notFound, "Unable to find constructor for \"%s\" with base type %s.\n",
            name.c_str(), typeid(ConstructableT).name() );
    }
    return *static_cast<IndexOfConstructables::EnumerableEntry<ConstructableT>*>(it->second);
}

template<typename ConstructableT> ConstructableT *
IndexOfConstructables::construct(
                    const std::string & name,
                    const goo::dict::Dictionary & parameters ) {
    auto & sect = _get_section<ConstructableT>();
    auto it = sect.find( name );
    if( sect.end() == it ) {
        emraise( notFound, "Has no virtual constructor for entity \"%s\":%s.",
            name.c_str(), typeid(ConstructableT).name() );
    }
    return static_cast<EnumerableEntry<ConstructableT>*>(it->second)
            ->constructor( parameters );
}

}  // namespace sys

template<typename ConstructableT>
void print_constructables_reference( std::ostream & os ) {
    for( auto ctrEntry : sys::IndexOfConstructables::self().known_constructors<ConstructableT>() ) {
        os  << "   * \"" << ctrEntry.first << "\" " << std::endl;
        std::list<std::string> lines;
        ctrEntry.second->arguments.print_ASCII_tree( lines );
        for( auto line : lines ) {
            os << "        " << line << std::endl;
        }
    }
}

}  // namespace sV


//
// Helper macros

/**@def
 * @brief Produces default constructor name useful that may be used in macros
 *        for short.
 * 
 * Utilizing this macro is optional.
 **/
# define StromaV_DEFAULT_CONSTRUCTOR_NAME( cxxClassName )                   \
                        __static_construct_ ## cxxClassName

/**@def StromaV_IMPLEMENT_DEFAULT_CONSTRUCTOR_FOR
 * @brief Default ctr implementation.
 *
 * Implements default constructor that just forwards initialized dictionary
 * to concrete product constructor (actual class constructor). Implemented
 * as a static routine.
 *
 * Utilizing this macro is optional.
 **/
# define StromaV_IMPLEMENT_DEFAULT_CONSTRUCTOR_FOR( cxxBaseClassName,       \
                                                    cxxClassName )          \
static cxxBaseClassName * StromaV_DEFAULT_CONSTRUCTOR_NAME(cxxClassName) (  \
                                const ::goo::dict::Dictionary & dict ) {    \
    return new cxxClassName( dict );}

/**@def StromaV_DEFINE_CONSTRUCTABLE
 * @brief Helper macro adding the registering routine to the current
 *        implementation file for certain constructible class.
 *
 * This macro contains open function declaration implying that one will
 * implement function body immediately after this macro within the curly
 * brackets. This body must return the goo::dict::Dictionary instance.
 *
 * It sums up few mandatory routines that registers the certain constructible
 * class descendant within its siblings allowing user code to invoke it at the
 * runtime.
 *
 * Note, that this macro has to be used only once for each class and should
 * always be located in the implementation source file.
 *
 * Utilizing this macro is optional.
 **/
# define StromaV_DEFINE_CONSTRUCTABLE(  cxxBaseClass,   \
                                        cxxClassName,   \
                                        name,           \
                                        constructorName )   \
static void __static_register_ ## cxxClassName ## _ctr() __attribute__ ((constructor(156)));  \
static goo::dict::Dictionary __static_assemble_config_ ## cxxClassName ();      \
static void __static_register_ ## cxxClassName ## _ctr() {                      \
    sV::sys::IndexOfConstructables::self().add_constructor<cxxBaseClass>( name, \
        new sV::sys::IndexOfConstructables::EnumerableEntry<cxxBaseClass>(      \
        constructorName, __static_assemble_config_ ## cxxClassName() ) ); }     \
goo::dict::Dictionary __static_assemble_config_ ## cxxClassName ()


/**@def StromaV_DEFINE_STD_CONSTRUCTABLE
 * @brief Helper macro summing up \ref StromaV_DEFINE_CONSTRUCTABLE and
 *        \ref StromaV_IMPLEMENT_DEFAULT_CONSTRUCTOR_FOR .
 *
 * This macro is designed to even more abbreviate virtual ctr definition.
 * */
# define StromaV_DEFINE_STD_CONSTRUCTABLE(  cxxClassName,                   \
                                            name,                           \
                                            cxxBaseClass )                  \
StromaV_IMPLEMENT_DEFAULT_CONSTRUCTOR_FOR( cxxBaseClass,                    \
                                           cxxClassName )                   \
StromaV_DEFINE_CONSTRUCTABLE( cxxBaseClass,                                 \
                        cxxClassName,                                       \
                        name,                                               \
                        StromaV_DEFAULT_CONSTRUCTOR_NAME( cxxClassName ) )

# endif  // RPC_PROTOCOLS
# endif  // H_STROMA_V_CONSTRUCTORS_DICTIONARY_H

