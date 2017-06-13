/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef graphics_canvassurface_h__
#define graphics_canvassurface_h__

#include <SkSurface.h>
#include <SkCanvas.h>
#include <memory>

namespace Nidium {
namespace Graphics {

class SkiaContext;

/*
    All size are expressed in device pixel
*/
class CanvasSurface
{
public:
    friend class SurfaceCache;

    static std::shared_ptr<CanvasSurface> Create(int width, int height, GrContext *gr);

    static std::shared_ptr<CanvasSurface> Wrap(int width, int height, sk_sp<SkSurface> surface) {
        return std::make_shared<CanvasSurface>(width, height, surface);
    }

    CanvasSurface(int width, int height, sk_sp<SkSurface> surface) :
        m_SkiaSurface(surface), m_Width(width), m_Height(height)
    {

    }

    bool resize(int width, int height);
    void replaceSurface(sk_sp<SkSurface> newSurface, int width,
        int height, bool addToCache = false);

    /* Reset the state of the underlying SkCanvas object */
    void reset();

    /* Check whether this surface can be reused */
    bool canBeClaimed(int width, int height);

    sk_sp<SkSurface> getSkiaSurface() {
        return m_SkiaSurface;
    }

    int width() const {
        return m_Width;
    }

    int height() const {
        return m_Height;
    }

    void mark(uint64_t frame) {
        m_LastMarkedFrame = frame;
    }

    uint64_t getMark() {
        return m_LastMarkedFrame;
    }

    void attachToSkiaContext(SkiaContext *sc) {
        m_CurrentSkiaContext = sc;
    }

    void dettachSkiaContext(SkiaContext *onlyif) {
        if (m_CurrentSkiaContext == onlyif) {
            m_CurrentSkiaContext = nullptr;
        }
    }

    CanvasSurface *reclaim();

private:
    void touch();

    uint64_t m_LastMarkedFrame = 0;
    sk_sp<SkSurface> m_SkiaSurface;
    int m_Width = 0, m_Height = 0;

    SkiaContext *m_CurrentSkiaContext = nullptr;
};

}}

#endif
