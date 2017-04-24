/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "Graphics/ContextCache.h"
#include "Binding/JSCanvas2DContext.h"

namespace Nidium {
namespace Graphics {

void ContextCache::addToCache(int width, int height, Binding::Canvas2DContext *ctx)
{
    auto &store = m_Store[std::make_pair(width, height)];

    auto sctx = (std::shared_ptr<Binding::Canvas2DContext> *)ctx->getWrappedPtr();

    store.push_back(*sctx);

    printf("[%dx%d] Element added to cache %ld\n", width, height, store.size());
}

}}
