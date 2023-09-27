#pragma once
#ifndef CLIP_LINE_H__
#define CLIP_LINE_H__

#include <core/fwd.h>

// From https://www.wikiwand.com/en/Cohen%E2%80%93Sutherland_algorithm
// Thank you wikipedia!

// Cohen–Sutherland clipping algorithm clips a line from
// P0 = (x0, y0) to P1 = (x1, y1) against a rectangle with
// diagonal from (xmin, ymin) to (xmax, ymax).

using OutCode = int;
constexpr int OC_INSIDE = 0;
constexpr int OC_LEFT = 1;
constexpr int OC_RIGHT = 2;
constexpr int OC_BOTTOM = 4;
constexpr int OC_TOP = 8;

OutCode compute_out_code(const Vector2 &v, float xmin, float xmax, float ymin,
                         float ymax)
{
    OutCode code;
    code = OC_INSIDE; // initialised as being inside of [[clip window]]
    if (v[0] < xmin - Epsilon)
    { // to the left of clip window
        code |= OC_LEFT;
    }
    else if (v[0] > xmax + Epsilon)
    { // to the right of clip window
        code |= OC_RIGHT;
    }
    if (v[1] < ymin - Epsilon)
    { // below the clip window
        code |= OC_BOTTOM;
    }
    else if (v[1] > ymax + Epsilon)
    { // above the clip window
        code |= OC_TOP;
    }
    return code;
};

#endif