/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/

/*
    /!\ this class is not thread safe
    TODO: windows ('shlwapi')
   http://msdn.microsoft.com/en-us/library/windows/desktop/bb773559%28v=vs.85%29.aspx
*/

#ifndef core_path_h__
#define core_path_h__

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#ifndef _MSC_VER
#include <sys/param.h>
#include <unistd.h>
#endif

#include <ape_array.h>

struct JSContext;

namespace Nidium {
namespace IO {
class Stream;
}
namespace Core {

#define MAX_REGISTERED_SCHEMES 1024


extern char *g_m_Root;
extern char *g_m_Cwd;

#define SCHEME_DEFINE(prefix, streamclass, keepprefix)                   \
    (struct Nidium::Core::Path::schemeInfo)                              \
    {                                                                    \
        .str                  = prefix,                                  \
        .base                 = streamclass::CreateStream,               \
        .GetBaseDir           = streamclass::GetBaseDir,                 \
        .keepPrefix           = keepprefix,                              \
        .AllowLocalFileStream = streamclass::AllowLocalFileStream,       \
        .AllowSyncStream      = streamclass::AllowSyncStream             \
    }

#define URLSCHEME_MATCH(url, scheme) \
    (strcmp(Nidium::Core::Path::GetScheme(url)->str, scheme "://") == 0)
#define SCHEME_MATCH(obj, scheme) (strcmp(obj->str, scheme "://") == 0)

class Path
{
public:
    struct schemeInfo
    {
        const char *str;
        Nidium::IO::Stream *(*base)(const char *);
        const char *(*GetBaseDir)();
        bool keepPrefix;
        bool (*AllowLocalFileStream)();
        bool (*AllowSyncStream)();
    };

    /*
        allowAll defines if origin must match "cwd" stream class (if false)
    */
    explicit Path(const char *origin,
                  bool allowAll = false,
                  bool noFilter = false);

#if 0
    operator const char *() {
        return m_Path;
    }
#endif
    const char *path() const
    {
        return m_Path;
    }

    const char *dir() const
    {
        return m_Dir;
    }

    const char *host() const
    {
        return m_Host;
    }

    Nidium::IO::Stream *CreateStream(bool onlySync = false) const
    {
        if (!m_Scheme || !m_Path) {
            return NULL;
        }

        if (onlySync && !m_Scheme->AllowSyncStream()) {
            return NULL;
        }

        return m_Scheme->base(m_Path);
    }

    schemeInfo *GetScheme() const
    {
        return m_Scheme;
    }

    bool static HasScheme(const char *str);

    bool static IsRelative(const char *path);

    ~Path()
    {
        if (m_Path) {
            free(m_Path);
        }
        if (m_Dir) {
            free(m_Dir);
        }
    };

    static void RegisterScheme(const schemeInfo &scheme,
                               bool isDefault = false);
    static void UnRegisterSchemes();
    static schemeInfo *GetScheme(const char *url, const char **pURL = NULL);

    static char *Sanitize(const char *path, bool *outsideRoot = nullptr);

    static void Chroot(const char *root)
    {
        if (g_m_Root != NULL && root != g_m_Root) {
            free(g_m_Root);
        }
        g_m_Root = (root != NULL ? strdup(root) : NULL);
    }

    static void CD(const char *dir)
    {
        if (g_m_Cwd != NULL && dir != g_m_Cwd) {
            free(g_m_Cwd);
        }
        g_m_Cwd = (dir != NULL ? strdup(dir) : NULL);
    }

    static char *GetDir(const char *fullpath);

    static const char *GetRoot()
    {
        return g_m_Root;
    }

    static const char *GetCwd()
    {
        return g_m_Cwd;
    }

    static schemeInfo *GetCwdScheme()
    {
        if (!g_m_Cwd) {
            return NULL;
        }
        return GetScheme(g_m_Cwd);
    }

    static int g_m_SchemesCount;
    static struct schemeInfo g_m_Schemes[MAX_REGISTERED_SCHEMES];
    static struct schemeInfo *g_m_DefaultScheme;
    static void Makedirs(const char *dirWithSlashes);
    static bool InDir(const char *dir, const char *root);

private:
    void parse(const char *path);
    void invalidatePath();
    char *m_Path;
    char *m_Dir;
    char *m_Host;
    schemeInfo *m_Scheme;
};

} // namespace Core
} // namespace Nidium

#endif
