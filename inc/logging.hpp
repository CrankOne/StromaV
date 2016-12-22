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

# include <list>
# include <sstream>

// fwd
class TGCommandPlugin;

namespace sV {
namespace aux {

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


/// Uses Public Morozoff antipattern internally. Accepts standard X11
/// font ID string.
void set_font_of_TGCommandPlugin( TGCommandPlugin *, const std::string & );

}  // namespace aux
}  // namespace sV

# endif  // H_STROMA_V_LOGGING_AUX_H
