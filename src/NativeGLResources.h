#ifndef nativeglresources_h__
#define nativeglresources_h__

#include <stdlib.h>
#include <stdint.h>
#include <NativeHash.h>

class NativeGLResources
{
public:

    enum ResourceType {
        RTEXTURE,
        RPROGRAM,
        RSHADER
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

    Resource *get(const char *name) const {
        return m_List.get(name);
    }

private:
    NativeHash<Resource *> m_List;
};

#endif