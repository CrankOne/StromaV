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

# include "app.h"
# include "app/ascii_display.hpp"
# include "../logging.hpp"

# include <goo_exception.hpp>
# include <goo_app.hpp>

# include <boost/program_options.hpp>
# include <boost/asio/io_service.hpp>

# ifdef GOO_GDB_EXEC
/// Attaches gdb if SIGSEGV received.
# define ATTACH_GDB_ON_SEGFAULT_ENVVAR          "ATTACH_GDB_ON_SEGFAULT"
# endif

# ifdef GOO_GCORE_EXEC
/// Makes coredump if SIGSEGV received.
# define PRODUCE_COREDUMP_ON_SEGFAULT_ENVVAR    "COREDUMP_ON_SEGFAULT"
# endif

namespace sV {

namespace po = boost::program_options;

/**@class AbstractApplication
 * @brief Abstract application class constituing logging streams
 * and configuration/run entry points.
 * @ingroup Application
 */
class AbstractApplication : public goo::App<po::variables_map, std::ostream>,
                            public sV::aux::ASCII_Display {
public:
    typedef goo::App<po::variables_map, std::ostream> Parent;
    typedef po::variables_map Config;
    typedef std::ostream Stream;
protected:
    std::map<std::string, std::string> _envVarsDocs;
    Stream * _eStr;
    aux::StreamBuffer _lBuffer,
                      _eBuffer
                      ;
    Config * _variablesMap;
    mutable bool _immediateExit;
    mutable uint8_t _verbosity;

    /// Creates instance of type ConfigObjectT according to command-line arguments
    virtual Config * _V_construct_config_object( int argc, char * const argv[] ) const override;
    /// Configures application according to recently constructed config object.
    virtual void _V_configure_application( const Config * ) override;
    /// Should create the logging stream of type LogStreamT (app already configured).
    virtual Stream * _V_acquire_stream() override;
    virtual Stream * _V_acquire_errstream();
    /// User application should fullfill this chain to provide its own options.
    virtual std::vector<po::options_description> _V_get_options() const;
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

    /// Routes message to appropriate streams.
    virtual void message( int8_t level, const std::string message, bool noprefix=false );

    /// Safe po::variables_map getter.
    template<typename T>
    T cfg_option(const std::string & name) {
        if( !co_is_set() ) {
            emraise(badState, "Couldn't get option \"%s\" value as config object is not constructed yet (pre-init stage).", name.c_str());
        }
        try {
            return co()[name].as<T>();
        } catch( boost::bad_any_cast & e ) {
            emraise(badCast, "Couldn't parse or locate option \"%s\" value.", name.c_str());
        }
    }

    bool do_immediate_exit() const { return _immediateExit; }

    /// Returns pointer to common boost::io_service instance
    /// (interface method for future usage).
    boost::asio::io_service * boost_io_service_ptr() { return &_ioService; }
};  // AbstractApplication

}  // namespace sV

# endif  // H_STROMA_V_ABSTRACT_APPLICATION_H

