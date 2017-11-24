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

# ifndef H_STROMA_V_UTILS_ASCII_DISPLAY_H
# define H_STROMA_V_UTILS_ASCII_DISPLAY_H

# include <cstdint>
# include <cassert>
# include <boost/thread/mutex.hpp>
# include <boost/thread/thread.hpp>
# include <boost/thread/condition_variable.hpp>

namespace sV {
namespace aux {

/** Creates dynamically-updated ASCII-display that does not cause
 * sufficient performance penalty (which usually appears in text terminals
 * due to ASCII-output latency).
 *
 * Life cycle implies two stages: "printing" and "ready". While printing it does
 * not generates any output contrary to "ready" stage. Switching (toggling)
 * between stages are performed by calling special method --- notify_ascii_display().
 * */
class ASCII_Display {
public:
    static constexpr size_t LineLength = 512;

    typedef uint8_t NLines;
    class ASCII_Entry {
    private:
        ASCII_Display * _displayPtr;
        /// Should be called by destructor of display instance.
        void _unleash() { _displayPtr = nullptr; }
    protected:
        ASCII_Entry( ASCII_Display *, NLines );
        virtual ~ASCII_Entry();
    public:
        /// Returns result of is_ready() invokation for owning display.
        bool can_acquire_display_buffer() const {
                return _displayPtr ? _displayPtr->is_ready() : false; }
        /// Could be used in snprintf() calls.
        char ** my_ascii_display_buffer() {
                assert(_displayPtr);
                return _displayPtr->get_buffer_of(this); }
        friend class ASCII_Display;
    };
private:
    /// Manifesting bool variable;
    bool _isReady;
    /// Tells listening thread to exit.
    bool _quit;
    /// Conjugates with _isReady.
    bool _enabled;
    bool _updated;

    boost::mutex _printingMtx;
    boost::condition_variable _notifier;
    boost::thread _printingThread;

    /// Returns false when exit flag was set.
    bool _wait_for_info();
    /// Launches printing worker
    void _print_lines();
protected:

    std::map<const ASCII_Entry *, char ** > _entries;

    /// Implicitly called by ASCII_Entry constructor.
    void _register_me( const ASCII_Entry *, NLines );
    /// Implicitly called by ASCII_Entry destructor.
    void _erase_me( const ASCII_Entry * );
    /// Called by ASCII_Entry my_ascii_display_buffer() method.
    char ** get_buffer_of( const ASCII_Entry * );
public:
    bool is_ready() const { return _isReady && _enabled; }

    /**@brief Toggling method that initiates transition between «printing» and
     * «ready» stage.
     *
     * Once called in «ready» state, immediately forces the instance to
     * manifest itself as on «printing» state (is_ready() returns false).
     * When actual printing is done the next notify() invokation causes
     * the instance manifest itself as «ready» (is_ready() returns true);
     * */
    void notify_ascii_display();

    ASCII_Display( bool enabled=false );
    ~ASCII_Display();

    void enable_ASCII_display();
    void disable_ASCII_display();
};  // class ASCII_Display

}  // namespace aux
}  // namespace sV

# endif  // H_STROMA_V_UTILS_ASCII_DISPLAY_H

