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

# ifndef H_STROMA_V_ABSTRACT_APPLICATION_H
# define H_STROMA_V_ABSTRACT_APPLICATION_H

# include <boost/asio/io_service.hpp>

# include "app.h"
# include "app/ascii_display.hpp"
# include "../logging.hpp"
# include "../ctrs_dict.hpp"

# include <goo_exception.hpp>
# include <goo_app.hpp>
# include <goo_dict/configuration.hpp>
# include <goo_dict/injection.hpp>
# include <goo_path.hpp>

# include <iostream>

# ifdef GOO_GDB_EXEC
/// Attaches gdb if SIGSEGV received.
# define ATTACH_GDB_ON_SEGFAULT_ENVVAR          "ATTACH_GDB_ON_SEGFAULT"
# endif

# ifdef GOO_GCORE_EXEC
/// Makes coredump if SIGSEGV received.
# define PRODUCE_COREDUMP_ON_SEGFAULT_ENVVAR    "COREDUMP_ON_SEGFAULT"
# endif

namespace sV {

/**@class AbstractApplication
 * @brief Abstract application class constituing logging streams
 * and configuration/run entry points.
 * @ingroup Application
 */
class AbstractApplication : public goo::App<goo::dict::Configuration, std::ostream>,
                            public sV::aux::ASCII_Display {
public:
    typedef goo::dict::Configuration Config;
    typedef std::ostream Stream;
    typedef goo::App<Config, Stream> Parent;

    /// Aux class substituting config definitions and environment variables in
    /// the insertable config-parameters. To refer the config option, use
    /// $(CFG:config.path) syntax (were "CFG:" is mandatory prefix). To refer
    /// environment variable, use the $(EVVARNAME) syntax.
    class ConfigPathInterpolator : public goo::filesystem::Path::Interpolator {
    protected:
        const Config & _cfgRef;
    public:
        ConfigPathInterpolator( const Config & cfgRef ) : _cfgRef( cfgRef ) {}
        virtual std::string interpolate( const std::string & p ) const override;
    };  // class ConfigPathInterpolator

    /// Class collecting configuration parameters mappings (from common config
    /// to particular constructible classes). The utilizing usecase is actually
    /// restricted to only C++ applications, so we've made it a nested class.
    class ConstructableConfMapping {
    public:
        typedef goo::dict::DictionaryInjectionMap Injection;
        struct Mappings : public std::unordered_map<std::string, Injection> {
            std::string baseName, baseDescription;
        };
    private:
        std::map<std::type_index, Mappings> _sections;

        static ConstructableConfMapping * _self;
    protected:
        template<typename T> Mappings & _get_section( bool insertNonExisting=false );
    public:
        /// Returns a singleton instance of this class.
        static ConstructableConfMapping & self();

        /// Sets description for particular base type.
        template<typename T> void set_basetype_description( const std::string &, const std::string & );

        /// Inserts new config mapping for particular virtual ctr.
        template<typename T> void add_mappings( const std::string &, const goo::dict::DictionaryInjectionMap & );

        /// Returns config mapping dictionary.
        const std::map<std::type_index, Mappings> sections() const { return _sections; }

        /// Produces own config dictionary according to mappings from common
        /// one.
        template<typename T> void own_conf_for(
                        const std::string &,
                        const goo::dict::Dictionary &,
                        goo::dict::Dictionary & );
    };
protected:
    std::map<std::string, std::string> _envVarsDocs;
    Stream * _eStr;
    aux::StreamBuffer _lBuffer,
                      _eBuffer
                      ;
    Config * _appCfg,
             _configuration;

    /// Stores linked paths between common config and constructible configs.
    static std::unordered_map<std::type_index,
            std::unordered_map<std::string, goo::dict::DictionaryInjectionMap> >
            * _cfgInjectionsPtr;

    mutable bool _immediateExit;
    mutable uint8_t _verbosity;

    mutable ConfigPathInterpolator _pathInterpolator;

    /// Creates instance of type ConfigObjectT according to command-line arguments
    virtual Config * _V_construct_config_object( int argc, char * const argv[] ) const override;
    /// Configures application according to recently constructed config object.
    virtual void _V_configure_application( const Config * ) override;
    /// Should create the logging stream of type LogStreamT (app already configured).
    virtual Stream * _V_acquire_stream() override;
    virtual Stream * _V_acquire_errstream();
    /// User application should be configured here.
    virtual void _V_configure_concrete_app() {}
private:
    boost::asio::io_service _ioService;
    /// This should only be set by descendant classes, from ctr.
    /// Will be set to 0x0 until RootApplication descendant part is
    /// ctrd. Setting is available via the _enable_ROOT_feature()
    /// method and not via ctr arg due to C++ virtual inheritance
    /// mechanics implementing mixins concept.
    uint8_t _ROOTAppFeatures;
protected:
    void _process_options( const Config * );
    virtual const char * _get_prefix_for_loglevel( int8_t );
    /// For available features codes see mixins::RootApplication.
    void _enable_ROOT_feature( uint8_t ftCode );

    /// Appends common config with various options from dynamically loaded
    /// modules. See implementation for details.
    virtual void _append_common_config( Config & cfg );
    /// Performs configs parsing if path in dir, or just forwards execution to
    /// _parse_config_file() if it is a file.
    void _parse_configs( const goo::filesystem::Path & path );
    /// Parses config file by given path into common config.
    void _parse_config_file( const std::string & path );
    /// Sets the common config option.
    void _set_common_option( const std::string & path,
                             const std::string & strVal );
public:
    /// Default ctr --- the void config instance should be provided here.
    AbstractApplication( Config * );
    virtual ~AbstractApplication() {}

    /// Verbosity level setter.
    virtual void verbosity( uint8_t v ) { _verbosity = v; }
    /// Verbosity level getter.
    uint8_t verbosity() const { return _verbosity; }

    /// Prints current build info.
    virtual void dump_build_info( std::ostream & ) const;

    /// Returns error stream (as goo basically doesn't support it).
    std::ostream & es() { return ( _eStr ? *_eStr : std::cerr ); }

    /// Returns common interpolator ptr;
    ConfigPathInterpolator * path_interpolator() const { return &_pathInterpolator; }

    /// Routes message to appropriate streams.
    virtual void message( int8_t level, const std::string message, bool noprefix=false );

    /// Common config option getter.
    template<typename T>
    T cfg_option(const std::string & name) const {
        return _configuration[name.c_str()].as<T>();
    }

    /// Common config option getter.
    template<typename T>
    const std::list<T> & cfg_options_list(const std::string & name) const {
        return _configuration[name.c_str()].as_list_of<T>();
    }

    template<typename T>
    T app_option(const std::string & name) const {
        return _appCfg->parameter(name.c_str()).as<T>();
    }

    /// App config multiple option getter.
    template<typename T>
    const std::list<T> & app_options_list(const std::string & name) const {
        return _appCfg->parameter(name.c_str()).as_list_of<T>();
    }

    /// Returns reference to common config object.
    const Config & common_co() const { return _configuration; }

    /// Returns value of "immediate exit" flag that is used during
    /// configuration stage.
    bool do_immediate_exit() const { return _immediateExit; }

    /// Returns pointer to common boost::io_service instance
    /// (interface method for future usage).
    boost::asio::io_service * boost_io_service_ptr() { return &_ioService; }

    uint8_t ROOT_features() const { return _ROOTAppFeatures; }
};  // AbstractApplication

/// Constructs a new instance with its virtual ctr using common config
/// and defined mappings.
template<typename T> T *
generic_new( const std::string & name ) {
    const auto & vctrEntry = sV::sys::IndexOfConstructables::self().find<T>( name );
    goo::dict::Dictionary ownCfg( vctrEntry.arguments );
    if( ownCfg.name() ) {
        // Unnamed conf dicts has to be understood as ones without any config
        // options.
        AbstractApplication::ConstructableConfMapping::self()
            .own_conf_for<T>( name, goo::app<AbstractApplication>().common_co(),
                                    ownCfg );
    }
    if( goo::app<AbstractApplication>().verbosity() > 2 ) {
        sV_log3( "generic_new(): own config for \"%s\":\n", name.c_str() );
        std::list<std::string> lst;
        if( ownCfg.name() ) {
            ownCfg.print_ASCII_tree( lst );
            for( auto line : lst ) {
                sV_log3( "    %s\n", line.c_str() );
            }
        } else {
            sV_log3( "    <no parameters required>\n" );
        }
    }
    return vctrEntry.constructor( ownCfg );
}


template<typename T>
AbstractApplication::ConstructableConfMapping::Mappings &
AbstractApplication::ConstructableConfMapping::_get_section( bool insertNonExisting ) {
    std::type_index tIndex = std::type_index(typeid(T));
    auto sectionIt = _sections.find( tIndex );
    if( _sections.end() == sectionIt ) {
        if( insertNonExisting ) {
            sectionIt = _sections.emplace(
                    tIndex, Mappings() ).first;
        } else {
            emraise( notFound, "Has no enumerated injection mappings section "
                "for type %s.", typeid(T).name() );
        }
    }
    return sectionIt->second;
}

template<typename T> void
AbstractApplication::ConstructableConfMapping::add_mappings(
                            const std::string & name,
                            const goo::dict::DictionaryInjectionMap & mps ) {
    auto & mappings = _get_section<T>( true );
    auto ir = mappings.emplace( name, mps );
    if( !ir.second ) {
        sV_logw( "Omitting repeatative definition of config mappings "
            "for \"%s\":%s.\n", name.c_str(), typeid(T).name() );
    }
}

template<typename T> void
AbstractApplication::ConstructableConfMapping::set_basetype_description(
                const std::string & baseName,
                const std::string & baseDescription ) {
    auto & mappings = _get_section<T>( true );
    if( !mappings.baseName.empty() ) {
        if( mappings.baseName != baseName ) {
            sV_logw( "Changing conf mapping base type %s name from \"%s\" to \"%s\".\n",
                typeid(T).name(),
                mappings.baseName.c_str(),
                baseName.c_str() );
            mappings.baseName = baseName;
        }
    } else {
        mappings.baseName = baseName;
    }
    mappings.baseDescription = baseDescription;
}

template<typename T> void
AbstractApplication::ConstructableConfMapping::own_conf_for(
                        const std::string & nm,
                        const goo::dict::Dictionary & commonCfg,
                        goo::dict::Dictionary & dest ) {
    auto injsIt = _sections.find( typeid(T) );
    if( _sections.end() == injsIt ) {
        emraise( notFound, "Has no config options mappings for base type "
            "\"%s\".", typeid(T).name() );
    }
    const Mappings & injs = injsIt->second;
    auto injIt = injs.find( nm );
    if( injs.end() == injIt ) {
        emraise( notFound, "Has no config options mappings for \"%s\" "
            "virtual constructor type of base \"%s\".",
            nm.c_str(), typeid(T).name() );
    }
    const Injection & inj = injIt->second;

    inj.inject_parameters( commonCfg, dest );
}

//template<typename T> T * generic_new( const std::string & );  // XXX?

}  // namespace sV


/**@def StromaV_DEFINE_STD_CONSTRUCTABLE_MCONF
 * @brief Helper macro utilizing \ref StromaV_DEFINE_STD_CONSTRUCTABLE with
 *      minor addendum for referencing common config parameters.
 *
 * The expected return type of function definition that has to follow right
 * after this macro is an std::pair consisting of (own) parameters dictionary
 * and mapping object. The sV::AbstractApplication::ConstructableConfMapping
 * will be appended with this mapping object while dictionary will be forwarded
 * to \ref IndexOfConstructables singleton index as usual for
 * \ref StromaV_DEFINE_STD_CONSTRUCTABLE.
 * */
# define StromaV_DEFINE_STD_CONSTRUCTABLE_MCONF(    cxxClassName,           \
                                                    name,                   \
                                                    cxxBaseClassName )      \
static std::pair<goo::dict::Dictionary, goo::dict::DictionaryInjectionMap>  \
    __static_shim_ ## cxxClassName ## _ctr_w_mapping();                     \
StromaV_DEFINE_STD_CONSTRUCTABLE( cxxClassName, name, cxxBaseClassName ) {  \
    auto p = __static_shim_ ## cxxClassName ## _ctr_w_mapping();            \
    sV::AbstractApplication::ConstructableConfMapping::self()               \
                .add_mappings<cxxBaseClassName>( name, p.second );          \
    return p.first; }                                                       \
static std::pair<goo::dict::Dictionary, goo::dict::DictionaryInjectionMap>  \
    __static_shim_ ## cxxClassName ## _ctr_w_mapping()


# endif  // H_STROMA_V_ABSTRACT_APPLICATION_H

