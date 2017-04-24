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
namespace Graphics {

class CanvasSurface;

class SurfaceCache
{
public:
    SurfaceCache(){};
    ~SurfaceCache(){};

    void addToCache(int width, int height, CanvasSurface *ctx);

private:
    std::map< std::pair<int /* width */, int /* height */>, std::vector< std::weak_ptr<CanvasSurface> > > m_Store;
};

}}

#endif
