#ifndef nativecanvascontext_h__
#define nativecanvascontext_h__

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

class NativeCanvas2DContext;
struct NativeRect;

class NativeCanvasContext
{
public:

    class JSObject *jsobj;
    struct JSContext *jscx;

    enum mode {
        CONTEXT_2D,
        CONTEXT_WEBGL
    } m_Mode;

    virtual void translate(double x, double y)=0;
    virtual void setSize(int width, int height)=0;
    virtual void setScale(double x, double y, double px=1, double py=1)=0;
    virtual void clear(uint32_t color)=0;
    virtual void flush()=0;

    virtual void composeWith(NativeCanvas2DContext *layer,
        double left, double top, double opacity,
        double zoom, const NativeRect *rclip)=0;

    NativeCanvasContext() :
        jsobj(NULL), jscx(NULL) {

        printf("New canvas context\n");
    }
    virtual ~NativeCanvasContext(){};
};

#endif