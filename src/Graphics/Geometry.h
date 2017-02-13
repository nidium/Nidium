/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef graphics_geometry_h__
#define graphics_geometry_h__

#include <stdint.h>

namespace Nidium {
namespace Graphics {

struct Rect
{
    double m_fLeft, m_fTop, m_fBottom, m_fRight;
    bool isEmpty() const
    {
        return m_fLeft >= m_fRight || m_fTop >= m_fBottom;
    }
    bool intersect(double left, double top, double right, double bottom)
    {
        if (left < right && top < bottom && !this->isEmpty() && m_fLeft < right
            && left < m_fRight && m_fTop < bottom && top < m_fBottom) {
            if (m_fLeft < left) m_fLeft = left;
            if (m_fTop < top) m_fTop = top;
            if (m_fRight > right) m_fRight = right;
            if (m_fBottom > bottom) m_fBottom = bottom;
            return true;
        }
        return false;
    }

    bool
    checkIntersect(double left, double top, double right, double bottom) const
    {
        if (left < right && top < bottom && !this->isEmpty() && m_fLeft < right
            && left < m_fRight && m_fTop < bottom && top < m_fBottom) {
            return true;
        }
        return false;
    }

    Rect scaled(float scale) const
    {
        Rect r = { m_fLeft * scale, m_fTop * scale, m_fBottom * scale,
                   m_fRight * scale };

        return r;
    }
    bool contains(double x, double y) const
    {
        return !this->isEmpty() && m_fLeft <= x && x < m_fRight && m_fTop <= y
               && y < m_fBottom;
    }
};

}
}

#endif