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

# ifndef H_STROMA_V_EVENT_DISPLAY_DRAWABLE_DETECTOR_MIXIN_H
# define H_STROMA_V_EVENT_DISPLAY_DRAWABLE_DETECTOR_MIXIN_H

# include "sV_config.h"

# ifdef ALIGNMENT_ROUTINES

# include "iDetector.hpp"

# include "../detector_ids.h"
# include <cassert>

# include <TEveScene.h>  // XXX
//# include <TEveProjectionBases.h>

class TEveText;
class TAttBBox;
class TEveElementList;

namespace sV {
namespace alignment {

/**@brief A mixing to detector abstraction allowing it to be drawable.
 *
 * This class provides basic interface for ROOT's TEve drawing utility.
 * By being ebedded to other facility (in particular DetectorSet) this
 * mixin claims following lifecycle:
 *  1. The main decoration and immutable geometrical primitives should be
 *     drawn and initialized in local aggregating volume on invokation of
 *     DrawableDetector::draw_detector() method.
 *  2. If particular detector instance had a hit(s) during the event, the
 *     draw_hit() will be called. Note, that it implies also that user
 *     detector class also inherits the ReceptiveDetector mixin.
 * Usually detector primitives has a label, so we defined a helper method
 * to handle it.
 * */
class DrawableDetector : public virtual iDetector/*,
                         public TEveProjectable*/ {
public:
    typedef TEveScene Container;
private:
    Container * _top;
    TEveText * _label;
protected:
    virtual void _V_draw_detector( Container * ) = 0;
    virtual bool _V_draw_hit() = 0;
    virtual const TAttBBox * _V_bbox() const { return nullptr; }
    virtual void _set_label( const char *, float x, float y, float z );
public:
    DrawableDetector( const std::string & fn, const std::string & dn ) :
        iDetector(fn, dn, true, false, false),
        _top(nullptr) {}
    void draw_detector( Container * top_ ) { _V_draw_detector( _top = top_ ); }
    bool draw_hit() { return _V_draw_hit(); }
    bool was_drawn() const { return _top; }
    const Container * top_ptr() const;
    Container * top_ptr();
    TEveText & label() { assert(_label); return *_label; }
    const TEveText & label() const { assert(_label); return *_label; }
    const TAttBBox * bbox() const;
};  // class ReceptiveDetector

}  // namespace alignment
}  // namespace sV

# endif  // ALIGNMENT_ROUTINES

# endif  // H_STROMA_V_EVENT_DISPLAY_DRAWABLE_DETECTOR_MIXIN_H

