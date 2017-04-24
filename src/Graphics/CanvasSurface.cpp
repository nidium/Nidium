/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/

#include "Graphics/CanvasSurface.h"
#include "Graphics/SurfaceCache.h"
#include "Frontend/Context.h"

#define CANVAS_FRAME_THRESHOLD 3

namespace Nidium {
namespace Graphics {

std::shared_ptr<CanvasSurface> CanvasSurface::Create(int width,
    int height, GrContext *gr)
{
    Frontend::Context *nctx   = Frontend::Context::GetObject<Frontend::Context>();

    /* Check for reusable surface */
    auto cached = nctx->m_ContextCache.getCachedSurface(width, height);
    if (cached) {
        cached.get()->reset();
        printf("[Cached] Returned a cached surface %dx%d\n", width, height);
        return cached;
    }

    sk_sp<SkSurface> surface = SkSurface::MakeRenderTarget(gr, SkBudgeted::kNo,
                                    SkImageInfo::MakeN32Premul(width, height));

    if (!surface) {
        return nullptr;
    }

    auto cs = std::make_shared<CanvasSurface>(width, height, surface);

    nctx->m_ContextCache.addToCache(width, height, cs);

    return cs;
}

bool CanvasSurface::resize(int width, int height)
{
    sk_sp<SkSurface> newSurface = m_SkiaSurface->makeSurface(SkImageInfo::MakeN32Premul(width, height));

    if (!newSurface) {
        return false;
    }

    printf("Resize surface from %dx%d to %dx%d\n", m_Width, m_Height, width, height);

    replaceSurface(newSurface, width, height);

    return true;
}

void CanvasSurface::replaceSurface(sk_sp<SkSurface> newSurface, int width, int height) {
    m_Width  = width;
    m_Height = height;

    newSurface->getCanvas()->clear(0x00000000);

    // Blit the old surface into the new one
    m_SkiaSurface->draw(newSurface->getCanvas(), 0, 0, nullptr);

    // Keep the old matrix in place
    newSurface->getCanvas()->setMatrix(m_SkiaSurface->getCanvas()->getTotalMatrix());

    m_SkiaSurface = newSurface;
}

bool CanvasSurface::canBeClaimed()
{
    Frontend::Context *nctx   = Frontend::Context::GetObject<Frontend::Context>();
    return m_LastMarkedFrame > 0 && (m_LastMarkedFrame + CANVAS_FRAME_THRESHOLD) < nctx->getCurrentFrame();
}

void CanvasSurface::clear()
{
    if (m_SkiaSurface) {
        m_SkiaSurface.get()->getCanvas()->clear(0x00000000);
    }
}

void CanvasSurface::reset()
{
    if (!m_SkiaSurface) {
        return;
    }

    SkCanvas *canvas = m_SkiaSurface.get()->getCanvas();

    canvas->resetMatrix();
    canvas->restoreToCount(canvas->getSaveCount());
    canvas->clear(0x00000000);
}

}}
