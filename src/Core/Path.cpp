/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "Core/Path.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef _MSC_VER
#include "Port/MSWindows.h"
#else
#include <strings.h>
#endif

#include <string>
#include <vector>

#include <prio.h>

#include "Core/Utils.h"

namespace Nidium {
namespace Core {

// {{{ Preamble
char *g_m_Root = NULL;
char *g_m_Cwd  = NULL;

int Path::g_m_SchemesCount                                        = 0;
struct Path::schemeInfo *Path::g_m_DefaultScheme                  = NULL;
struct Path::schemeInfo Path::g_m_Schemes[MAX_REGISTERED_SCHEMES] = {};
// }}}

// {{{ Path
Path::Path(const char *origin, bool allowAll, bool noFilter)
    : m_Path(nullptr), m_Dir(nullptr), m_Host(nullptr), m_Scheme(nullptr)
{
    if (origin == nullptr) {
        return;
    }

    if (!Path::GetCwd() || noFilter) {
        const char *pOrigin;
        schemeInfo *scheme = Path::GetScheme(origin, &pOrigin);

        if (URLSCHEME_MATCH(origin, "file")) {
            if (!(m_Path = realpath(pOrigin, nullptr))) {
                this->invalidatePath();
                return;
            }
        } else {
            std::string tmp;
            if (scheme->GetBaseDir() != nullptr) {
                tmp += scheme->GetBaseDir();
            }
            tmp += pOrigin;

            m_Path = strdup(tmp.c_str());
        }

        m_Scheme = scheme;
        m_Dir    = Path::GetDir(m_Path);

        return;
    }

    schemeInfo *rootScheme = Path::GetCwdScheme();
    bool localRoot         = rootScheme->AllowLocalFileStream();

    // Remote Chroot trying to access local file.
    if (!localRoot && strncmp(origin, "file://", 7) == 0) {
        this->invalidatePath();
        return;
    }

    this->parse(origin);

    if (!m_Path) {
        this->invalidatePath();
        return;
    }

    // Local root check if we are still in nidium Chroot.
    if (localRoot && !allowAll
        && !Path::InDir(m_Path, m_Scheme->GetBaseDir())) {
        this->invalidatePath();
        return;
    }
}

void Path::parse(const char *origin)
{
    const char *pOrigin;
    const char *baseDir;
    schemeInfo *scheme     = Path::GetScheme(origin, &pOrigin);
    schemeInfo *rootScheme = Path::GetCwdScheme();
    const char *root       = Path::GetRoot();
    char *path = strdup(pOrigin);
    PtrAutoDelete<char *> _path(path, free);

    bool isRelative  = Path::IsRelative(origin);
    bool isLocalRoot = rootScheme->AllowLocalFileStream();

    m_Scheme = scheme;

    // Path with prefix. Extract it.
    if (scheme->keepPrefix) {
        const char *prefix = scheme->str;
        int next           = strlen(prefix);

        // Remove any extra slashes
        for (int i = next; path[i] && path[i] == '/'; i++) {
            next++;
        }

        path = &path[next];
    }

    // Path with host. Extract it.
    if (!scheme->AllowLocalFileStream()) {
        char *res = strchr(path, '/');
        if (res == nullptr) {
            // Path/Host without trailing slash
            m_Host = strdup(path);
            path   = strdup("/");

            free(_path.ptr());
            _path.set(path);
        } else {
            m_Host = strndup(path, res - path);
            path   = &path[res - path];
        }
    }

    // Relative path (no prefix) on a remote root.
    // Set the appropriate host & scheme.
    if ((isRelative || origin[0] == '/') && !isLocalRoot) {
        int hostStart       = strlen(rootScheme->str);
        const char *hostEnd = strchr(&root[hostStart], '/');
        int hostLen         = (hostEnd - &root[hostStart]);

        m_Scheme = rootScheme;
        m_Host = static_cast<char *>(malloc((hostLen + 1) * sizeof(char)));
        strncpy(m_Host, &root[hostStart], hostLen);
        m_Host[hostLen] = '\0';
    }

    // Prepare the full absolute path for Sanitize
    if (!isRelative && SCHEME_MATCH(scheme, "file")) {
        // Absolute file on disk
        baseDir = nullptr;
    } else if (isRelative && !isLocalRoot) {
        // Remote root, get the path of the root
        baseDir = &root[strlen(scheme->str) + strlen(m_Host)];
    } else if (SCHEME_MATCH(scheme, "file")) {
        // Relative file on disk
        baseDir = Path::GetCwd();
    } else {
        // Prefixed scheme with a base directory
        baseDir = scheme->GetBaseDir();
    }

    if (baseDir) {
        char *tmp = static_cast<char *>(
            malloc((strlen(path) + strlen(baseDir) + 1) * sizeof(char)));
        strcpy(tmp, baseDir);
        strcat(tmp, path);

        path = tmp;

        free(_path.ptr());
        _path.set(tmp);
    }

    m_Path = Path::Sanitize(path);
    if (!m_Path) {
        // No path
        return;
    }

    if (m_Host) {
        // Path have a host, prepend it along with the scheme to the final path
        char *tmp = static_cast<char *>(
            malloc((strlen(m_Path) + strlen(m_Host) + strlen(m_Scheme->str) + 1)
                   * sizeof(char)));
        strcpy(tmp, m_Scheme->str);
        strcat(tmp, m_Host);
        strcat(tmp, m_Path);

        free(m_Path);

        m_Path = tmp;
    } else if(m_Scheme->keepPrefix) {
        // Path is on a scheme with a prefix, prepend it
        char *tmp = static_cast<char *>(
            malloc((strlen(m_Path) + strlen(m_Scheme->str) + 1)
                   * sizeof(char)));
        strcpy(tmp, m_Scheme->str);
        strcat(tmp, m_Path);

        free(m_Path);

        m_Path = tmp;
    }

    m_Dir = Path::GetDir(m_Path);
}

bool Path::IsRelative(const char *path)
{
    const char *pPath;

    if (!path) {
        return false;
    }

    schemeInfo *scheme = Path::GetScheme(path, &pPath);

    /* We assume that if a "prefix based" path is given, it's an absolute URL */
    if (!SCHEME_MATCH(scheme, "file")) {
        return false;
    }

    /* We are matching a local file scheme (either from file:// or simple path)
     */
    return pPath[0] != '/';
}

char *Path::GetDir(const char *fullpath)
{
    int len = strlen(fullpath);
    char *ret;
    if (len == 0) {
        return NULL;
    }
    ret = static_cast<char *>(malloc(sizeof(char) * (len + 1)));
    memcpy(ret, fullpath, len + 1);

    char *pos = strrchr(ret, '/');
    if (pos != NULL) {
        pos[1] = '\0';
    }

    return ret;
}

bool Path::HasScheme(const char *str)
{
    for (int i = 0; i < Path::g_m_SchemesCount; i++) {
        if (strcasecmp(Path::g_m_Schemes[i].str, str) == 0) {
            return true;
        }
    }
    return false;
}

void Path::RegisterScheme(const Path::schemeInfo &scheme, bool isDefault)
{
    if (HasScheme(scheme.str)
        || Path::g_m_SchemesCount + 1 >= MAX_REGISTERED_SCHEMES) {

        return;
    }

    schemeInfo *newScheme = &Path::g_m_Schemes[Path::g_m_SchemesCount];

    newScheme->str                  = strdup(scheme.str);
    newScheme->base                 = scheme.base;
    newScheme->keepPrefix           = scheme.keepPrefix;
    newScheme->GetBaseDir           = scheme.GetBaseDir;
    newScheme->AllowLocalFileStream = scheme.AllowLocalFileStream;
    newScheme->AllowSyncStream      = scheme.AllowSyncStream;

    Path::g_m_SchemesCount++;

    if (isDefault || Path::g_m_DefaultScheme == NULL) {
        Path::g_m_DefaultScheme = newScheme;
    }
}

void Path::UnRegisterSchemes()
{
    schemeInfo *scheme;

    for (int i = 0; i < Path::g_m_SchemesCount; i++) {
        scheme = &Path::g_m_Schemes[i];
        // TODO: new style cast
        free((char *)(scheme->str));
    }
    Path::g_m_SchemesCount = 0;
}

Path::schemeInfo *Path::GetScheme(const char *url, const char **pURL)
{
    for (int i = 0; i < Path::g_m_SchemesCount; i++) {
        int len = strlen(Path::g_m_Schemes[i].str);
        if (strncasecmp(Path::g_m_Schemes[i].str, url, len) == 0) {
            bool prefix = Path::g_m_Schemes[i].keepPrefix;
            if (pURL) {
                *pURL = (prefix ? url : &url[len]);
            }
            return &Path::g_m_Schemes[i];
        }
    }
    if (pURL) {
        *pURL = url;
    }
    return g_m_DefaultScheme;
}

void Path::Makedirs(const char *dirWithSlashes)
{
    char tmp[MAXPATHLEN];
    char *p = NULL;
    size_t len;

    len = snprintf(tmp, sizeof(tmp), "%s", dirWithSlashes);
    if (tmp[len - 1] == '/') {
        tmp[len - 1] = 0;
    }
    for (p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = 0;
            PR_MkDir(tmp, 00700);
            *p = '/';
        }
    }
    PR_MkDir(tmp, 00700);
}

#define HANDLE_DOUBLE_DOT()                     \
    counter--;                                  \
    counterPos = nidium_max(0, counterPos - 1); \
    if (counter < 0) {                          \
        outsideRoot = true;                     \
        if (!isRelative) {                      \
            if (external) {                     \
                *external = true;               \
            }                                   \
            return nullptr;                     \
        }                                       \
    }                                           \
    elements[counterPos].clear();               \
    minCounter = nidium_min(counter, minCounter);

// XXX : Only works with path (not URIs)
char *Path::Sanitize(const char *path, bool *external)
{
    enum PathState
    {
        kPathState_Start,
        kPathState_Dot,
        kPathState_DoubleDot,
        kPathState_Name,
        kPathState_Slash
    } state
        = kPathState_Slash;

    if (external) {
        *external = false;
    }

    if (path == nullptr) {
        return nullptr;
    }

    int pathlen = strlen(path);
    if (pathlen == 0) {
        return strdup("");
    }

    int counter = 0, minCounter = 0, counterPos = 0;
    bool outsideRoot = false;
    bool isRelative  = Path::IsRelative(path);

    std::vector<std::string> elements(pathlen);

    for (int i = 0; i < pathlen; i++) {
        switch (path[i]) {
            case '.':
                switch (state) {
                    case kPathState_Dot:
                        state = kPathState_DoubleDot;
                        break;
                    case kPathState_DoubleDot:
                        state = kPathState_Name;
                        break;
                    case kPathState_Name:
                        elements[counterPos] += path[i];
                        break;
                    case kPathState_Slash:
                        state = kPathState_Dot;
                        break;
                    default:
                        break;
                }
                break;
            case '/':
                switch (state) {
                    case kPathState_Dot:
                        break;
                    case kPathState_Name:
                        counter++;
                        counterPos++;
                        break;
                    case kPathState_DoubleDot:
                        HANDLE_DOUBLE_DOT()
                        break;
                    case kPathState_Slash:
                        break;
                    default:
                        break;
                }
                state = kPathState_Slash;
                break;
            default:
                if (state == kPathState_Dot) {
                    // File starting with a dot (ie : config/.nidium)
                    elements[counterPos] += '.';
                }
                elements[counterPos] += path[i];
                state = kPathState_Name;
                break;
        }
    }

    if (state == kPathState_DoubleDot) {
        HANDLE_DOUBLE_DOT()
    }

    std::string finalPath;
    if (outsideRoot) {
        for (int i = minCounter; i != 0; i++) {
            finalPath += "../";
        }
    } else if (!isRelative) {
        finalPath += "/";
    }


    for (int i = 0; elements[i].length() != 0; i++) {
        finalPath += elements[i] + "/";
    }

    if (finalPath.length() > 0 && state == kPathState_Name) {
        finalPath[finalPath.length() - 1] = '\0';
    }

    if (external) {
        *external = outsideRoot;
    }

    if (strcmp(finalPath.c_str(), ".") == 0 && strlen(finalPath.c_str()) == 1) {
        return strdup("");
    }

    return strdup(finalPath.c_str());
}
#undef HANDLE_DOUBLE_DOT

void Path::invalidatePath()
{
    free(m_Path);
    free(m_Host);
    free(m_Dir);

    m_Path = nullptr;
    m_Host = nullptr;
    m_Dir  = nullptr;
}

bool Path::InDir(const char *path, const char *root)
{
    if (!root) {
        return true;
    }

    int i    = 0;
    int diff = 0;

    while (root[i] && path[i] && root[i] == path[i]) {
        i++;
    }

    diff = root[i] || path[i] ? i : -1;

    return diff >= strlen(root);
}
// }}}

} // namespace Core
} // namespace Nidium
