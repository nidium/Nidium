/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef graphics_gradient_h__
#define graphics_gradient_h__

#include <stdint.h>

#include <GrGLTexture.h>
#include <SkCanvas.h>
#include <SkRefCnt.h>

class SkShader;

namespace Nidium {
namespace Graphics {

struct _colorStop
{
    SkColor m_Color;
    SkScalar m_Position;
};

class Gradient
{
private:
    struct
    {
        double x;
        double y;
        double radius;
    } m_StartPoint;

    struct
    {
        double x;
        double y;
        double radius;
    } m_EndPoint;

    struct
    {
        uint32_t count;
        uint32_t allocated;

        struct _colorStop *items;
    } m_ColorsStop;

    int m_IsRadial;
    int m_NeedUpdate;

    sk_sp<SkShader> m_CurrentShader;

public:
    ~Gradient();
    Gradient(double x1, double y1, double x2, double y2);
    Gradient(double x0, double y0, double r0, double x1, double y1, double r1);
    void addColorStop(double position, char *color);
    sk_sp<SkShader> build();
};

} // namespace Graphics
} // namespace Nidium

#endif
