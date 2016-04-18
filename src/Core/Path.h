/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/

/*
    /!\ this class is not thread safe
    TODO: windows ('shlwapi') http://msdn.microsoft.com/en-us/library/windows/desktop/bb773559%28v=vs.85%29.aspx
*/

#ifndef core_path_h__
#define core_path_h__

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/param.h>

#include <ape_array.h>

namespace Nidium {
	namespace IO {
		class Stream;
	}
}

struct JSContext;

namespace Nidium {
namespace Core {

#define MAX_REGISTERED_SCHEMES 1024


extern char *g_m_Root;
extern char *g_m_Pwd;

#define SCHEME_DEFINE(prefix, streamclass, keepprefix) ( \
struct Nidium::Core::Path::schemeInfo) { \
    .str        = prefix, \
    .base       = streamclass::createStream, \
    .getBaseDir = streamclass::getBaseDir, \
    .keepPrefix = keepprefix, \
    .allowLocalFileStream = streamclass::allowLocalFileStream, \
    .allowSyncStream = streamclass::allowSyncStream \
}

#define URLSCHEME_MATCH(url, scheme) (strcmp(Nidium::Core::Path::getScheme(url)->str, scheme "://") == 0)
#define SCHEME_MATCH(obj, scheme) (strcmp(obj->str, scheme "://") == 0)

class Path
{
public:
    struct schemeInfo {
        const char *str;
        Nidium::IO::Stream *(*base)(const char *);
        const char *(*getBaseDir)();
        bool keepPrefix;
        bool (*allowLocalFileStream)();
        bool (*allowSyncStream)();
    };

    /*
        allowAll defines if origin must match "pwd" stream class (if false)
    */
    explicit Path(const char *origin, bool allowAll = false,
        bool noFilter = false);

#if 0
    operator const char *() {
        return m_Path;
    }
#endif
    const char *path() const {
        return m_Path;
    }

    const char *dir() const {
        return m_Dir;
    }

    const char *host() const {
        return m_Host;
    }

    Nidium::IO::Stream *createStream(bool onlySync = false) const {
        if (!m_Scheme || !m_Path) {
            return NULL;
        }

        if (onlySync && !m_Scheme->allowSyncStream()) {
            return NULL;
        }

        return m_Scheme->base(m_Path);
    }

    schemeInfo *getScheme() const {
        return m_Scheme;
    }

    bool static isRelative(const char *path);

    ~Path() {
        if (m_Path) {
            free(m_Path);
        }
        if (m_Dir) {
            free(m_Dir);
        }
    };

    static void registerScheme(const schemeInfo &scheme,
        bool isDefault = false);
    static void unRegisterSchemes();
    static schemeInfo *getScheme(const char *url, const char **pURL = NULL);

    static char *sanitize(const char *path, bool *outsideRoot = nullptr);

    static void chroot(const char *root) {
        if (g_m_Root != NULL && root != g_m_Root) {
            free(g_m_Root);
        }
        g_m_Root = (root != NULL ? strdup(root) : NULL);
    }

    static void cd(const char *dir) {
        if (g_m_Pwd != NULL && dir != g_m_Pwd) {
            free(g_m_Pwd);
        }
        g_m_Pwd = (dir != NULL ? strdup(dir) : NULL);
    }

    static char *getDir(const char *fullpath);

    static const char *getRoot() {
        return g_m_Root;
    }

    static const char *getPwd() {
        return g_m_Pwd;
    }

    static schemeInfo *getPwdScheme() {
        if (!g_m_Pwd) {
            return NULL;
        }
        return Path::getScheme(g_m_Pwd);
    }

    static char *currentJSCaller(JSContext *cx = NULL);
    static int g_m_SchemesCount;
    static struct schemeInfo g_m_Schemes[MAX_REGISTERED_SCHEMES];
    static struct schemeInfo *g_m_DefaultScheme;
    static void makedirs(const char * dirWithSlashes);
    static char *absolutize(const char *relative, const char *root);
    static bool inDir(const char *dir, const char *root);
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
