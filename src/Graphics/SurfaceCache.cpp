/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "Graphics/SurfaceCache.h"
#include "Graphics/CanvasSurface.h"
#include "Binding/JSCanvas2DContext.h"

namespace Nidium {
namespace Graphics {

void SurfaceCache::addToCache(int width, int height, std::shared_ptr<CanvasSurface> cs)
{
    auto &store = m_Store[std::make_pair(width, height)];

    store.push_back(cs);

    m_Counter++;

    printf("[%dx%d] Element added to cache %ld (total %d)\n", width, height, store.size(), m_Counter);
}

std::shared_ptr<CanvasSurface> SurfaceCache::getCachedSurface(int width, int height)
{
    auto &store = m_Store[std::make_pair(width, height)];

    for (auto &surface : store) {
        CanvasSurface *cs = surface.get();
        if (cs->canBeClaimed(width, height)) {
            cs->reclaim();
            return surface;
        }
    }

    return nullptr;
}

}}
