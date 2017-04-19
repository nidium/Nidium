/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef graphics_canvas3dcontext_h__
#define graphics_canvas3dcontext_h__

#include <stdint.h>

#include "Graphics/CanvasContext.h"

namespace Nidium {
namespace Interface {
class UIInterface;
}
namespace Graphics {

class CanvasHandler;

class Canvas3DContext : public CanvasContext
{
public:
    enum FLags
    {
        kUNPACK_FLIP_Y_WEBGL_Flag            = 1 << 0,
        kUNPACK_PREMULTIPLY_ALPHA_WEBGL_Flag = 1 << 1
    };

    /*
        width and height is in logical pixel
    */
    Canvas3DContext(CanvasHandler *handler,
                    struct JSContext *cx,
                    int width,
                    int height,
                    Interface::UIInterface *ui);
    virtual ~Canvas3DContext();

    virtual void translate(double x, double y) override;
    virtual void setSize(float width, float height, bool redraw = true) override;
    virtual void
    setScale(double x, double y, double px = 1, double py = 1) override;
    virtual void clear(uint32_t color = 0x00000000) override;
    virtual void flush() override;
    virtual uint32_t getTextureID() const override;

    /* Returns the size in device pixel */
    virtual void getSize(int *width, int *height) const override;

    /*
        Read pixels from the underlying buffer
        Memory can be modified but not free'd
    */
    virtual uint8_t *getPixels() override;

    uint32_t getFrameBufferID() const
    {
        return m_GLObjects.fbo;
    }

    void setFlag(uint32_t flag)
    {
        m_Flags = flag;
    }

    uint32_t getFlag() const
    {
        return m_Flags;
    }

    void removeFlag(uint32_t flag)
    {
        m_Flags &= ~flag;
    }

    void addFlag(uint32_t flag)
    {
        m_Flags |= flag;
    }

    bool hasFlag(uint32_t flag)
    {
        return m_Flags & flag;
    }
private:
    /*
        width and height are in device pixel
    */
    bool createFBO(int width, int height);
    void cleanUp();

    struct
    {
        uint32_t fbo;
        uint32_t texture;
        uint32_t vao;
        uint32_t renderbuffer;
        uint32_t colorbuffer;

        uint32_t fbo_sampled;
    } m_GLObjects;

    struct
    {
        /*
            int device pixel
        */
        int width;
        int height;
    } m_Device;

    uint32_t m_Flags;

    struct
    {
        int width;
        int height;

        uint8_t *pixels;
    } m_CachedPixels;
};

} // namespace Graphics
} // namespace Nidium

#endif
