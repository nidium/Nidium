/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/

#include "Graphics/CanvasSurface.h"
#include "Graphics/SurfaceCache.h"
#include "Frontend/Context.h"

namespace Nidium {
namespace Graphics {

std::shared_ptr<CanvasSurface> CanvasSurface::Create(int width,
    int height, GrContext *gr)
{
    Frontend::Context *nctx   = Frontend::Context::GetObject<Frontend::Context>();

    sk_sp<SkSurface> surface = SkSurface::MakeRenderTarget(gr, SkBudgeted::kNo,
                                    SkImageInfo::MakeN32Premul(width, height));

    if (!surface) {
        return nullptr;
    }

    auto cs = std::make_shared<CanvasSurface>(width, height, surface);

    nctx->m_ContextCache.addToCache(width, height, cs);

    return cs;
}

}}
