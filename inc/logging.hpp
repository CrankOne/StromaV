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


/**@class LoggingFamily
 * @brief Thmeatic section for logging usually referring to certain set of
 *        classes.
 *
 * Families are usual sV virtually-constructible classes indexed in its common
 * IndexOfConstructables class. Besides of that all created instances are
 * indexed at the common static map.
 * */
class LoggingFamily {
private:
    const std::string _name;
    /// Logging level set for family.
    LogLevel _lvl;
    /// Stream references set for family.
    std::ostream * _streamPtrs[7];

    /// Static dictionary of all constructed family objects.
    static std::unordered_map<std::string, LoggingFamily *> * _families;
public:
    /// Ctr. Sets all stream ptrs to null.
    LoggingFamily( const std::string &, LogLevel );
    /// Dtr.
    virtual ~LoggingFamily() {}
    /// Log level getter.
    LogLevel log_level() const { return _lvl; }
    /// Log level setter.
    virtual void log_level( LogLevel );
    /// Returns stream appropriate to the level provided (even if set log level
    /// is lesser than provided). May be
    /// overriden by descendant classes to provide dedicated stream for errors
    /// and messages.
    virtual std::ostream & stream_for( LogLevel );
    /// Returns prefix string appropriate for referred level.
    virtual std::string get_prefix_for_loglevel( LogLevel ) const;
    /// Returns const ref to current family name.
    virtual const std::string & family_name() const { return _name; }
    /// Returns logging family instance. If it does not exist, it will be
    /// created using virtual ctr facility.
    static LoggingFamily & get_instance( const std::string & );
};  // class LoggingFamily

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
 */
class Logger {
private:
    /// Buffer for snprintf;
    mutable char _bf[GOO_EMERGENCY_BUFLEN];

    /// Common logging prefix related to this particular instance.
    std::string _prefix;

    /// Family to which this instance belongs.
    LoggingFamily & _family;
public:
    /// Creates new instance that may access logging functions via own methods
    /// automatically dispatching message to the certain logging section
    /// (LoggingFamily). Second argument describes format string for current
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
    virtual void log_msg( LogLevel lvl, const char * fmt, ... ) const;
    /// Returns logging prefix related to this particular instance.
    const std::string & logging_prefix() const { return _prefix; }
    /// Returns reference to logging family (mutable).
    LoggingFamily & log_family() { return _family; }
    /// Returns reference to logging family (const).
    const LoggingFamily & log_family() const { return _family; }
};

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
