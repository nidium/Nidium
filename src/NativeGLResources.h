#ifndef nativeglresources_h__
#define nativeglresources_h__

#include <stdlib.h>
#include <stdint.h>
#include <NativeHash.h>
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
        char *m_Name;
    };

    NativeGLResources()
    {
        /* Call delete automagically on every item of the hashtable */
        m_List.setAutoDelete(true);
    };
    ~NativeGLResources(){};

    void add(uint32_t glid, ResourceType type, const char *name = NULL);

    Resource *get(uint32_t glid, ResourceType type) const {
        return m_List.get(((uint64_t)glid << 8) | (uint8_t)type);
    }

private:
    NativeHash64<Resource *> m_List;
};

#endif