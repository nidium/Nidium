#include "NativeSkGradient.h"
#include "NativeSkia.h"
#include "SkGradientShader.h"
#include <stdio.h>
#include "NativeMacros.h"

NativeSkGradient::NativeSkGradient(double x1, double y1, double x2, double y2)
{
    startPoint.x = x1;
    startPoint.y = y1;

    endPoint.x = x2;
    endPoint.y = y2;

    colorsStop.count = 0;
    colorsStop.allocated = 8;

    colorsStop.items = (struct _colorStop *)malloc(sizeof(struct _colorStop) *
        colorsStop.allocated);

    needUpdate = 1;
    isRadial = 0;
    currentShader = NULL;
}

NativeSkGradient::NativeSkGradient(double x0, double y0, double r0,
    double x1, double y1, double r1)
{
    startPoint.x = x0;
    startPoint.y = y0;
    startPoint.radius = r0;

    endPoint.x = x1;
    endPoint.y = y1;
    endPoint.radius = r1;

    colorsStop.count = 0;
    colorsStop.allocated = 8;

    colorsStop.items = (struct _colorStop *)malloc(sizeof(struct _colorStop) *
        colorsStop.allocated);

    needUpdate = 1;
    isRadial = 1;
    currentShader = NULL;
}

NativeSkGradient::~NativeSkGradient()
{
    //if (currentShader)
        //NLOG("Free gradient %d for %p", (currentShader ? currentShader->getRefCnt() : 0), currentShader);
    free(colorsStop.items);
    SkSafeUnref(currentShader);
}

void NativeSkGradient::addColorStop(double position, char *color)
{

    if (colorsStop.count == colorsStop.allocated) {
        colorsStop.allocated = colorsStop.allocated << 1;

        colorsStop.items = (struct _colorStop *)realloc(colorsStop.items,
            sizeof(struct _colorStop) * colorsStop.allocated);
    }

    colorsStop.items[colorsStop.count].color    = NativeSkia::parseColor(color);
    colorsStop.items[colorsStop.count].position = SkDoubleToScalar(position);

    colorsStop.count++;
    needUpdate = 1;

}

SkShader *NativeSkGradient::build()
{
    if (!needUpdate) {
        return currentShader;
    }

    if (colorsStop.count < 2) {
        currentShader = NULL;
        needUpdate = 0;
        printf("Building gradient with invalid number of (addColorStop) : %d\n", colorsStop.count);
        return NULL;
    }

    SkPoint pts[2];

    pts[0].set(SkDoubleToScalar(startPoint.x), SkDoubleToScalar(startPoint.y));
    pts[1].set(SkDoubleToScalar(endPoint.x), SkDoubleToScalar(endPoint.y));

    SkColor colors[colorsStop.count];
    SkScalar pos[colorsStop.count];

    for (unsigned int i = 0; i < colorsStop.count; i++) {
        colors[i] = colorsStop.items[i].color;
        pos[i] = colorsStop.items[i].position;
    }

    needUpdate = 0;
    SkSafeUnref(currentShader);

    if (isRadial) {
        currentShader = SkGradientShader::CreateTwoPointConical(pts[0],
            SkDoubleToScalar(startPoint.radius), pts[1],
            SkDoubleToScalar(endPoint.radius), colors, pos, colorsStop.count,
            SkShader::kClamp_TileMode);
    } else {
        currentShader = SkGradientShader::CreateLinear(pts, colors,
            pos, colorsStop.count, SkShader::kClamp_TileMode);
    }

    return currentShader;
}

