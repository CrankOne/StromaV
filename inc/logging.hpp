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

# ifndef H_STROMA_V_LOGGING_AUX_H
# define H_STROMA_V_LOGGING_AUX_H

# include "ctrs_dict.hpp"

# include <goo_config.h>

# include <list>
# include <sstream>

# if defined(TEMPLATED_LOGGING) && TEMPLATED_LOGGING
#   include "ctemplate/template.h"
# endif

// fwd
class TGCommandPlugin;

namespace sV {
namespace logging {

enum LogLevel : int8_t {
    error = -2,
    warning = -1,
    quiet = 0,
    laconic = 1,
    verbose = 2,
    loquacious = 3,
    debug = 4,
};

/**@class StreamBuffer
 * @brief A stream buffer class multiplicating its output to substreams.
 *
 * A custom stream buffer class for std::ostream that forwards
 * logging depending on provided settings. Not well optimized,
 * so shouldn't be used for massive output.
 * The idea is taken from:
 * from http://stackoverflow.com/questions/13703823/a-custom-ostream
 *
 * This class also adopts the colored output for substreams where
 * coloring information (as an ASCII escape sequences) is not need.
 * When color suppression is specified the substitution regex will
 * be applied to lines.
 **/
class StreamBuffer : public std::stringbuf {
protected:
    /// Streaming targets to provide logging output.
    std::list<std::pair<std::ostream *, bool> > _targets;
    /// An STL-method invoked on each stream buffer update.
    virtual int sync() override;
public:
    /// Default ctr.
    StreamBuffer();
    virtual ~StreamBuffer();

    /// Pushes back a pointer to STL ostream instance where the output
    /// is to be routed to.
    void add_target( std::ostream *, bool enableColoring=true );
};  // class StreamBuffer


/**@class iLoggingFamily
 * @brief Thmeatic section for logging usually referring to certain set of
 *        classes.
 *
 * Families are usual sV virtually-constructible classes indexed in its common
 * IndexOfConstructables class. Besides of that all created instances are
 * indexed at the common static map.
 * */
class iLoggingFamily {
private:
    std::string _name;
    const std::string _className;
    /// Logging level set for family.
    LogLevel _lvl;
    /// Static dictionary of all constructed family objects.
    static std::unordered_map<std::string, iLoggingFamily *> * _families;
    /// True, when instance was initialized with dictionary and false.
    bool _customized;
protected:
    /// (IM) Will be invoked by application instance on the parsing config stage.
    virtual void _V_configure( const goo::dict::Dictionary & ) = 0;
    /// (IM) Has to return stream appropriate to the level provided (even if set log
    /// level is lesser than provided). May be used by descendant classes to
    /// provide dedicated stream for errors and messages.
    virtual std::ostream & _V_stream_for( LogLevel l ) = 0;
    /// (IM) Has to prepends a common (plain, not templated) prefix and print
    /// message into appropriate stream.
    virtual void _V_message( LogLevel lvl, const std::string & instanceID, const std::string & ) const = 0;
    # if defined(TEMPLATED_LOGGING) && TEMPLATED_LOGGING
    /// (IM) This version of logging method is only available when templated logging
    /// is enabled (Ctemplate library was found on system). May accept arbitrary
    /// information defined in template dictionary (context).
    virtual void _V_message( ctemplate::TemplateDictionary & ldct, LogLevel lvl) const = 0;
    # endif

    /// May be called by ctr of descendant classes when constructor handles the
    /// dictionary. Sets _customized = true.
    void set_customized() { _customized = true; }
    /// Invalidates customization flag.
    void unset_customized() { _customized = false; }

    /// Protected family name setter.
    void set_name( const std::string & fn ) { _name = fn; }
public:
    /// Ctr. Sets all stream ptrs to null.
    /// Since the logging families are almost always created before the
    /// configs read, the config dictionary in this ctr may be empty. At this
    /// case, please, provision using the configure() function.
    iLoggingFamily( const std::string & className,
                    LogLevel );
    /// Dtr.
    virtual ~iLoggingFamily() {}
    /// Returns logging family instance. If it does not exist, it will be
    /// created using virtual ctr facility.
    static iLoggingFamily & get_instance( const std::string & );

    /// Log level getter.
    LogLevel log_level() const { return _lvl; }
    /// Log level setter.
    virtual void log_level( LogLevel );
    /// Forwards call to _V_stream_for() interfacing method.
    virtual std::ostream & stream_for( LogLevel l ) {
        return _V_stream_for(l); }
    /// Returns const ref to current family name.
    virtual const std::string & family_name() const { return _name; }
    virtual const std::string & logging_class_name() const { return _className; }
    /// Has to be invoked by application instance after the parsing config stage.
    virtual void configure( const goo::dict::Dictionary & d ) {
        _V_configure(d); set_customized(); }
    /// Forwards invokation to interfacing method _V_message().
    virtual void message( LogLevel lvl, const std::string & instanceID, const std::string & msg ) const {
        _V_message( lvl, instanceID, msg ); }
    # if defined(TEMPLATED_LOGGING) && TEMPLATED_LOGGING
    /// Forwards invokation to interfacing method _V_message() with Ctemplate
    /// dict.
    virtual void message( ctemplate::TemplateDictionary & ldct, LogLevel lvl) const {
        _V_message( ldct, lvl ); }
    # endif

    /// Returns true, when this instance was correctly configured (i.e.,
    /// configured after config loaded).
    bool customized() const { return _customized; }

    /// Has to be called after config is parsed to re-initialize already
    /// created family instances.
    static size_t initialize_families();
};  // class iLoggingFamily

/// Shortcut for define virtual ctr for logging family classes without common
/// config mapping.
# define StromaV_LOGGING_CLASS_DEFINE( cxxClassName, name )                 \
StromaV_DEFINE_STD_CONSTRUCTABLE( cxxClassName, name, sV::logging::iLoggingFamily )

/// Shortcut for define virtual ctr for logging family classes with common
/// config mapping.
# define StromaV_LOGGING_CLASS_DEFINE_MCONF( cxxClassName, name )           \
StromaV_DEFINE_STD_CONSTRUCTABLE_MCONF( cxxClassName, name, sV::logging::iLoggingFamily )

class CommonLoggingFamily : public iLoggingFamily {
private:
    /// Stream references set for family.
    std::ostream * _streamPtrs[8];
protected:
    static const char templateNames[8][64];
    static const char logPrefixes[8][32];

    /// Returns stream appropriate to the level provided (even if set log level
    /// is lesser than provided). May be overriden by descendant classes to
    /// provide dedicated stream for errors and messages.
    virtual std::ostream & _V_stream_for( LogLevel ) override;
    /// Configures logging family instance (mostly sets the prefixes).
    void _V_configure( const goo::dict::Dictionary & ) override;
    /// Just prepends a common (plain, not templated) prefix and prints to
    /// appropriate stream. Uses statically-defined prefixes declared within
    /// this class.
    virtual void _V_message( LogLevel lvl, const std::string & instanceID, const std::string & ) const override;
    # if defined(TEMPLATED_LOGGING) && TEMPLATED_LOGGING
    /// This version of logging method is only available when templated logging
    /// is enabled (Ctemplate library was found on system). May accept arbitrary
    /// information defined in template dictionary (context). Uses global
    /// Ctemplates cached templates.
    virtual void _V_message( ctemplate::TemplateDictionary & ldct, LogLevel lvl) const override;
    # endif
public:
    /// Since the "Common" logging family is almost always created before the
    /// configs read, the config dictionary in this ctr is almost never used.
    CommonLoggingFamily( const goo::dict::Dictionary &,
                         LogLevel l=loquacious );
    /// Returns plain (non-template) prefix string appropriate for referred
    /// level.
    virtual std::string get_prefix_for_loglevel( LogLevel ) const;
};

/**@class Logger
 * @brief A utility class for dedicated logging.
 *
 * This class may be included into inheritance chain when it is neccesary to
 * provide a dedicated logging facility, separated or not from sV's
 * application-wide logging.
 * 
 * Follows common convention of three-leveled logging, where 0 does not
 * produces logging messages at all and 3 generates loquacious output. Negative
 * codes are reserved for warnings (-1) and errors (-2). Logging an error has
 * no effect besides printing an error message to the proper stream (i.e., no
 * abort, termination or exceptions thrown).
 *
 * Depending on whether the Ctemplates library was found on the host system,
 * the behaviour of this class may vary in two ways:
 *  - When Ctemplates was not found, this class will merely forward the
 *    log_msg() invokation to the corresponding family instance adding just a
 *    static prefix.
 *  - When Ctemplates was found, in addition to extra log_msg() method, the
 *    template rendering facility will be involved supporting some additional
 *    information.
 */
class Logger {
private:
    /// Buffer for snprintf;
    mutable char _bf[GOO_EMERGENCY_BUFLEN];

    /// Common logging prefix related to this particular instance.
    std::string _prefix;

    /// Family to which this instance belongs.
    iLoggingFamily & _family;
public:
    /// Creates new instance that may access logging functions via own methods
    /// automatically dispatching message to the certain logging section
    /// (iLoggingFamily). Second argument describes format string for current
    /// instance that, if non-empty, will be printed before any message issued
    /// by this instance, after common section prefix.
    Logger( const std::string & familyName,
            const std::string & prfx="" );
    virtual ~Logger();
    /// Returns current logging level.
    LogLevel log_level() const { return _family.log_level(); }
    /// Method with functionality similar to native C printf(), putting the
    /// message into logging stream if given lvl is equal to or less than
    /// current for this Logger instance.
    virtual void log_msg( const char *, size_t, LogLevel lvl, const char * fmt, ... ) const;

    # if defined(TEMPLATED_LOGGING) && TEMPLATED_LOGGING
    /// This version of logging method is only available when templated logging
    /// is enabled (Ctemplate library was found on system). May accept arbitrary
    /// information defined in template dictionary (context).
    virtual void log_msg( ctemplate::TemplateDictionary & ldct,
                          LogLevel lvl, const char * fmt, ... ) const;
    # endif

    /// Returns logging prefix related to this particular instance.
    const std::string & logging_prefix() const { return _prefix; }
    /// Returns reference to logging family (mutable).
    iLoggingFamily & log_family() { return _family; }
    /// Returns reference to logging family (const).
    const iLoggingFamily & log_family() const { return _family; }
};

# if defined(TEMPLATED_LOGGING) && TEMPLATED_LOGGING
#   define _sV_mylog( file, line, lvl, ... ) do {   \
if( this->log_level() < lvl ) break;                        \
    ctemplate::TemplateDictionary dict("sV_mylog");         \
    dict.SetValue( "file", file );                          \
    dict.SetIntValue( "line", line );                       \
    std::string fileName( file );                           \
    const char * lstDlmPtr;                                 \
    if( !fileName.empty() && ( '\0' != *(lstDlmPtr = strrchr(file, '/')) ) ) {  \
        dict.SetValue( "shortFile", ++lstDlmPtr );          \
    }                                                       \
    this->log_msg( dict, (::sV::logging::LogLevel) lvl, __VA_ARGS__ );  \
} while(false);
# else
#   define _sV_mylog( file, line, lvl, __VA_ARGS__ )        \
        this->log_msg( file, line, (::sV::logging::LogLevel) lvl, __VA_ARGS__ );
# endif

# define sV_mylog1(...) _sV_mylog( __FILE__, __LINE__,  1, __VA_ARGS__ )
# define sV_mylog2(...) _sV_mylog( __FILE__, __LINE__,  2, __VA_ARGS__ )
# define sV_mylog3(...) _sV_mylog( __FILE__, __LINE__,  3, __VA_ARGS__ )
# define sV_mylogd(...) _sV_mylog( __FILE__, __LINE__,  4, __VA_ARGS__ )
# define sV_mylogw(...) _sV_mylog( __FILE__, __LINE__, -1, __VA_ARGS__ )
# define sV_myloge(...) _sV_mylog( __FILE__, __LINE__, -2, __VA_ARGS__ )

}  // namespace logging

namespace aux {
/** Uses Public Morozoff antipattern internally. Accepts standard X11
 * font ID string.
 * @ingroup cernroot
 */
void set_font_of_TGCommandPlugin( TGCommandPlugin *, const std::string & );
}  // namespace ::sV::aux

}  // namespace sV

# endif  // H_STROMA_V_LOGGING_AUX_H
