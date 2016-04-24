#include "Graphics/SkGradient.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <SkGradientShader.h>

#include "Graphics/Skia.h"

namespace Nidium {
namespace Graphics {

NativeSkGradient::NativeSkGradient(double x1, double y1, double x2, double y2)
{
    m_StartPoint.x = x1;
    m_StartPoint.y = y1;

    m_EndPoint.x = x2;
    m_EndPoint.y = y2;

    m_ColorsStop.count = 0;
    m_ColorsStop.allocated = 8;

    m_ColorsStop.items = (struct _colorStop *)malloc(sizeof(struct _colorStop) *
        m_ColorsStop.allocated);

    m_NeedUpdate = 1;
    m_IsRadial = 0;
    m_CurrentShader = NULL;
}

NativeSkGradient::NativeSkGradient(double x0, double y0, double r0,
    double x1, double y1, double r1)
{
    m_StartPoint.x = x0;
    m_StartPoint.y = y0;
    m_StartPoint.radius = r0;

    m_EndPoint.x = x1;
    m_EndPoint.y = y1;
    m_EndPoint.radius = r1;

    m_ColorsStop.count = 0;
    m_ColorsStop.allocated = 8;

    m_ColorsStop.items = (struct _colorStop *)malloc(sizeof(struct _colorStop) *
        m_ColorsStop.allocated);

    m_NeedUpdate = 1;
    m_IsRadial = 1;
    m_CurrentShader = NULL;
}

void NativeSkGradient::addColorStop(double position, char *color)
{

    if (m_ColorsStop.count == m_ColorsStop.allocated) {
        m_ColorsStop.allocated = m_ColorsStop.allocated << 1;

        m_ColorsStop.items = (struct _colorStop *)realloc(m_ColorsStop.items,
            sizeof(struct _colorStop) * m_ColorsStop.allocated);
    }

    m_ColorsStop.items[m_ColorsStop.count].color    = NativeSkia::parseColor(color);
    m_ColorsStop.items[m_ColorsStop.count].position = SkDoubleToScalar(position);

    m_ColorsStop.count++;
    m_NeedUpdate = 1;

}

SkShader *NativeSkGradient::build()
{
    if (!m_NeedUpdate) {
        return m_CurrentShader;
    }

    if (m_ColorsStop.count < 2) {
        m_CurrentShader = NULL;
        m_NeedUpdate = 0;
        printf("Building gradient with invalid number of (addColorStop) : %d\n", m_ColorsStop.count);
        return NULL;
    }

    SkPoint pts[2];

    pts[0].set(SkDoubleToScalar(m_StartPoint.x), SkDoubleToScalar(m_StartPoint.y));
    pts[1].set(SkDoubleToScalar(m_EndPoint.x), SkDoubleToScalar(m_EndPoint.y));

    SkColor colors[m_ColorsStop.count];
    SkScalar pos[m_ColorsStop.count];

    for (unsigned int i = 0; i < m_ColorsStop.count; i++) {
        colors[i] = m_ColorsStop.items[i].color;
        pos[i] = m_ColorsStop.items[i].position;
    }

    m_NeedUpdate = 0;
    SkSafeUnref(m_CurrentShader);

    if (m_IsRadial) {
        m_CurrentShader = SkGradientShader::CreateTwoPointConical(pts[0],
            SkDoubleToScalar(m_StartPoint.radius), pts[1],
            SkDoubleToScalar(m_EndPoint.radius), colors, pos, m_ColorsStop.count,
            SkShader::kClamp_TileMode);
    } else {
        m_CurrentShader = SkGradientShader::CreateLinear(pts, colors,
            pos, m_ColorsStop.count, SkShader::kClamp_TileMode);
    }

    return m_CurrentShader;
}

NativeSkGradient::~NativeSkGradient()
{
    //if (currentShader)
        //NLOG("Free gradient %d for %p", (currentShader ? currentShader->getRefCnt() : 0), currentShader);
    free(m_ColorsStop.items);
    SkSafeUnref(m_CurrentShader);
}


} // namespace Graphics
} // namespace Nidium

