/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef nativeskgradient_h__
#define nativeskgradient_h__

#include <stdint.h>

#include <GrGLTexture.h>
#include <SkCanvas.h>

class SkShader;

struct _colorStop
{
    SkColor color;
    SkScalar position;
};

class NativeSkGradient
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

    struct {
        uint32_t count;
        uint32_t allocated;

        struct _colorStop *items;
    } m_ColorsStop;

    int m_IsRadial;
    int m_NeedUpdate;
    SkShader *m_CurrentShader;

  public:
    ~NativeSkGradient();
    NativeSkGradient(double x1, double y1, double x2, double y2);
    NativeSkGradient(double x0, double y0, double r0, double x1,
      double y1, double r1);
    void addColorStop(double position, char *color);
    SkShader *build();

};

#endif

