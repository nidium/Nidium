/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef graphics_contextcache_h__
#define graphics_contextcache_h__

#include <memory>
#include <map>
#include <utility>
#include <vector>



namespace Nidium {
namespace Binding {
    class Canvas2DContext;
}

namespace Graphics {

class ContextCache
{
public:
    ContextCache(){};
    ~ContextCache(){};

    void addToCache(int width, int height, Binding::Canvas2DContext *ctx);

private:
    std::map< std::pair<int /* width */, int /* height */>, std::vector< std::weak_ptr<Binding::Canvas2DContext> > > m_Store;
};

}}

#endif
