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

/*
    All size are expressed in device pixel
*/
class CanvasSurface
{
public:

    static std::shared_ptr<CanvasSurface> Create(int width, int height, GrContext *gr) {
        sk_sp<SkSurface> surface = SkSurface::MakeRenderTarget(gr, SkBudgeted::kNo,
                                        SkImageInfo::MakeN32Premul(width, height));

        if (!surface) {
            return nullptr;
        }

        return std::make_shared<CanvasSurface>(width, height, surface);
    }

    static std::shared_ptr<CanvasSurface> Wrap(int width, int height, sk_sp<SkSurface> surface) {
        return std::make_shared<CanvasSurface>(width, height, surface);
    }

    CanvasSurface(int width, int height, sk_sp<SkSurface> surface) :
        m_SkiaSurface(surface), m_Width(width), m_Height(height)
    {

    }

    sk_sp<SkSurface> getSkiaSurface() {
        return m_SkiaSurface;
    }

    bool used() const {
        return m_Used;
    }

    int width() const {
        return m_Width;
    }

    int height() const {
        return m_Height;
    }

    bool resize(int width, int height)
    {
        sk_sp<SkSurface> newSurface = m_SkiaSurface->makeSurface(SkImageInfo::MakeN32Premul(width, height));

        if (!newSurface) {
            return false;
        }

        replaceSurface(newSurface, width, height);

        return true;
    }

    void replaceSurface(sk_sp<SkSurface> newSurface, int width, int height) {
        m_Width  = width;
        m_Height = height;

        newSurface->getCanvas()->clear(0x00000000);

        // Blit the old surface into the new one
        m_SkiaSurface->draw(newSurface->getCanvas(), 0, 0, nullptr);

        // Keep the old matrix in place
        newSurface->getCanvas()->setMatrix(m_SkiaSurface->getCanvas()->getTotalMatrix());

        m_SkiaSurface = newSurface;
    }

private:
    sk_sp<SkSurface> m_SkiaSurface;
    int m_Width = 0, m_Height = 0;
    bool m_Used = true;
};

}}

#endif
