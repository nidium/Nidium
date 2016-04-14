#ifndef nativeglresources_h__
#define nativeglresources_h__

#include <stdlib.h>
#include <stdint.h>

#include <Core/Hash.h>

#include "NativeMacros.h"

class NativeGLResources
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
            NativeGLResources::ResourceType type,
            char *name);

        ~Resource();
    private:
        uint32_t m_Glid;
        ResourceType m_Type;
    };

    NativeGLResources()
    {
        /* Call delete automagically on every
        item of the hashtable */

        m_List.setAutoDelete(true);
    };
    ~NativeGLResources() {};

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
    Nidium::Core::Hash64<Resource *> m_List;

    uint64_t genId(uint32_t glid, ResourceType type) const
    {
        return (((uint64_t)glid << 8) | (uint8_t)type);
    }
};

#endif

