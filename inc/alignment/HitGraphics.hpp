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

# ifndef H_STROMA_V_ALIGNMENT_HIT_GRAPHICS_H
# define H_STROMA_V_ALIGNMENT_HIT_GRAPHICS_H

# include <list>
# include <unordered_map>

namespace sV {
namespace aux {

struct HitGraphicsTraits {
    enum IsotropicPointMarkerType {
        dots,
        circles,
        // ...
    };
    struct IsotropicPointMarkerParameters {
        uint8_t rgba[4];
        float position[3];
        float size;
    };
    struct AnisotropicPointMarkerParameters : public IsotropicPointMarkerParameters {
        uint8_t dx[2], dy[2], dz[2];
    };
    struct LineMarkerParameters {
        uint8_t width;
        uint8_t rgba[4];
        float positions[2][3];
    };

    typedef std::list<IsotropicPointMarkerParameters *> IsotropicMarkers;
    typedef std::list<AnisotropicPointMarkerParameters *> AnisotropicMarkers;
    typedef std::list<LineMarkerParameters *> LineMarkers;

    struct HitMarkers {
        IsotropicMarkers    isotropicMarkers;
        AnisotropicMarkers  anisotropicMarkers;
        LineMarkers         lineMarkers;
    };

    typedef void (*IsotropicPointPainter)( const IsotropicPointMarkerParameters & );
};  // struct HitGraphicsTraits

struct iHitsPainter : public HitGraphicsTraits {
    using HitGraphicsTraits::IsotropicMarkers;
    using HitGraphicsTraits::AnisotropicMarkers;
    using HitGraphicsTraits::LineMarkers;

    virtual void   draw_isotropic_points_markers( const HitGraphicsTraits::IsotropicMarkers &,
                                                        IsotropicPointMarkerType ) = 0;
    virtual void draw_anisotropic_points_markers( const HitGraphicsTraits::AnisotropicMarkers & ) = 0;
    virtual void draw_line_markers( const HitGraphicsTraits::LineMarkers & ) = 0;
};  // struct iHitsPainter

}  // namespace aux
}  // namespace sV

# endif  // H_STROMA_V_ALIGNMENT_HIT_GRAPHICS_H

