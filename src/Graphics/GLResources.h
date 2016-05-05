/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef graphics_glresources_h__
#define graphics_glresources_h__

#include <stdlib.h>
#include <stdint.h>

#include <Core/Hash.h>

#include "Macros.h"

using Nidium::Core::Hash64;

namespace Nidium {
namespace Graphics {

class GLResources
{
public:

    enum ResourceType {
        RTEXTURE,
        RPROGRAM,
        RSHADER,
        RBUFFER,
        RVERTEX_ARRAY
    };

    class Resource {
    public:
        Resource(uint32_t glid,
            GLResources::ResourceType type,
            char *name);

        ~Resource();
    private:
        uint32_t m_Glid;
        ResourceType m_Type;
    };

    GLResources()
    {
        /* Call delete automagically on every
        item of the hashtable */

        m_List.setAutoDelete(true);
    };
    ~GLResources() {};

    void add(uint32_t glid, ResourceType type, const char *name = NULL);

    Resource *get(uint32_t glid, ResourceType type) const
    {
        return m_List.get(genId(glid, type));
    }

    void remove(uint32_t glid, ResourceType type)
    {
        return m_List.erase(genId(glid, type));
    }

private:
    Hash64<Resource *> m_List;

    uint64_t genId(uint32_t glid, ResourceType type) const
    {
        return (((uint64_t)glid << 8) | (uint8_t)type);
    }
};

} // namespace Graphics
} // namespace Nidium

#endif

