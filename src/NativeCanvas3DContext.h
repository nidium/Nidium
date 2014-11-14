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
    /*
        width and height is in logical pixel
    */
    NativeCanvas3DContext(NativeCanvasHandler *handler,
        struct JSContext *cx, int width, int height, NativeUIInterface *ui);
    virtual ~NativeCanvas3DContext();

    virtual void translate(double x, double y) override;
    virtual void setSize(int width, int height, bool redraw = true) override;
    virtual void setScale(double x, double y, double px=1, double py=1) override;
    virtual void clear(uint32_t color = 0x00000000) override;
    virtual void flush() override;
    virtual uint32_t getTextureID() const override;

    /* Returns the size in device pixel */
    virtual void getSize(int *width, int *height) const override;

    uint32_t getFrameBufferID() const {
        return m_GLObjects.fbo;
    }
private:
    
    /*
        width and height are in device pixel
    */
    bool createFBO(int width, int height);
    void cleanUp();

    struct {
        uint32_t fbo;
        uint32_t texture;
        uint32_t vao;
    } m_GLObjects;

    struct {
        /*
            int device pixel
        */
        int width;
        int height;
    } m_Device;
};

#endif