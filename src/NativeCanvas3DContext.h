#ifndef nativecanvas3dcontext_h__
#define nativecanvas3dcontext_h__

#include <stdint.h>
#include "NativeJSExposer.h"
#include "NativeCanvasContext.h"

class NativeCanvasHandler;
class NativeUIInterface;

class NativeCanvas3DContext : public NativeCanvasContext
{
public:
    NativeCanvas3DContext(NativeCanvasHandler *handler,
        struct JSContext *cx, int width, int height, NativeUIInterface *ui);
    virtual ~NativeCanvas3DContext();

    virtual void translate(double x, double y) override;
    virtual void setSize(int width, int height, bool redraw = true) override;
    virtual void setScale(double x, double y, double px=1, double py=1) override;
    virtual void clear(uint32_t color = 0x00000000) override;
    virtual void flush() override;

    /* Returns the size in device pixel */
    virtual void getSize(int *width, int *height) const override;

    virtual void composeWith(NativeCanvas2DContext *layer,
        double left, double top, double opacity,
        double zoom, const NativeRect *rclip) override;
private:
    
    bool createFBO();

    struct {
        uint32_t fbo;
        uint32_t texture;
    } m_GLObjects;
};

#endif