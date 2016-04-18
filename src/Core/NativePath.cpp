/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "NativePath.h"

#include <string>
#include <vector>
#include <sys/stat.h>

#include <js/OldDebugAPI.h>

#include "NativeUtils.h"

#include "JS/NativeJS.h"

char *g_m_Root = NULL;
char *g_m_Pwd = NULL;

int NativePath::g_m_SchemesCount = 0;
struct NativePath::schemeInfo *NativePath::g_m_DefaultScheme = NULL;
struct NativePath::schemeInfo NativePath::g_m_Schemes[NATIVE_MAX_REGISTERED_SCHEMES] = {};

NativePath::NativePath(const char *origin, bool allowAll, bool noFilter) :
    m_Path(nullptr), m_Dir(nullptr), m_Host(nullptr), m_Scheme(nullptr)
{
    if (origin == nullptr) {
        return;
    }

    if (!NativePath::getPwd() || noFilter) {
        const char *pOrigin;
        schemeInfo *scheme = NativePath::getScheme(origin, &pOrigin);

        if (URLSCHEME_MATCH(origin, "file")) {
            if (!(m_Path = realpath(pOrigin, nullptr))) {
                this->invalidatePath();
                return;
            }
        } else {
            std::string tmp;
            if (scheme->getBaseDir() != nullptr) {
                tmp += scheme->getBaseDir();
            }
            tmp += pOrigin;

            m_Path = strdup(tmp.c_str());
        }

        m_Scheme = scheme;
        m_Dir = NativePath::getDir(m_Path);

        return;
    }

    schemeInfo *rootScheme = NativePath::getPwdScheme();
    bool localRoot = rootScheme->allowLocalFileStream();

    // Remote chroot trying to access local file. 
    if (!localRoot && strncmp(origin, "file://", 7) == 0) {
        this->invalidatePath();
        return;
    }

    this->parse(origin);

    if (!m_Path) {
        this->invalidatePath();
        return;
    }

    // Local root check if we are still in Native chroot.
    if (localRoot && !allowAll && !NativePath::inDir(m_Path, m_Scheme->getBaseDir())) {
        this->invalidatePath();
        return;
    }
}

void NativePath::parse(const char *origin)
{
    const char *pOrigin;
    const char *baseDir;
    schemeInfo *scheme = NativePath::getScheme(origin, &pOrigin);
    schemeInfo *rootScheme = NativePath::getPwdScheme();
    const char *root = NativePath::getRoot();
    char *path = strdup(pOrigin);
    NativePtrAutoDelete<char *> _path(path, free);

    bool isRelative = NativePath::isRelative(origin);
    bool isLocalRoot = rootScheme->allowLocalFileStream();

    m_Scheme = scheme;

    // Path with prefix. Extract it.
    if (scheme->keepPrefix) {
        const char *prefix = scheme->str;
        int next = strlen(prefix);

        // Remove any extra slashes
        for (int i = next; path[i] && path[i] == '/'; i++) {
            next++;
        }

        path = &path[next];
    }

    // Path with host. Extract it.
    if (!scheme->allowLocalFileStream()) {
        char *res = strchr(path, '/');
        if (res == nullptr) {
            // Path/Host without trailing slash
            m_Host = strdup(path);
            path = strdup("/");

            free(_path.ptr());
            _path.set(path);
        } else {
            m_Host = strndup(path, res - path);
            path = &path[res - path];
        }
    }

    // Relative path (no prefix) on a remote root.
    // Set the appropriate host & scheme.
    if ((isRelative || origin[0] == '/') && !isLocalRoot) { 
        int hostStart = strlen(rootScheme->str);
        const char *hostEnd = strchr(&root[hostStart], '/');
        int hostLen = (hostEnd - &root[hostStart]);

        m_Scheme = rootScheme;
        m_Host = (char *)malloc((hostLen + 1) * sizeof(char));
        strncpy(m_Host, &root[hostStart], hostLen);
        m_Host[hostLen] = '\0';
    } 

    // Prepare the full absolute path for sanitize
    if (!isRelative && SCHEME_MATCH(scheme, "file")) {
        // Absolute file on disk
        baseDir = nullptr;
    } else if (isRelative && !isLocalRoot) {
        // Remote root, get the path of the root
        baseDir = &root[strlen(scheme->str) + strlen(m_Host)];
    } else if (SCHEME_MATCH(scheme, "file")) {
        // Relative file on disk
        baseDir = NativePath::getPwd();
    } else {
        // Prefixed scheme with a base directory
        baseDir = scheme->getBaseDir();
    }

    if (baseDir) {
        char *tmp = (char *)malloc((strlen(path) + strlen(baseDir) + 1) * sizeof(char));
        strcpy(tmp, baseDir);
        strcat(tmp, path);

        path = tmp;

        free(_path.ptr());
        _path.set(tmp);
    }

    m_Path = NativePath::sanitize(path);
    if (!m_Path) {
        // No path 
        return;
    }

    // Path have a host, prepend it to the final path
    if (m_Host) {
        char *tmp = (char *)malloc((strlen(m_Path) + strlen(m_Host) + strlen(m_Scheme->str) + 1) * sizeof(char));
        strcpy(tmp, m_Scheme->str);
        strcat(tmp, m_Host);
        strcat(tmp, m_Path);

        free(m_Path);

        m_Path = tmp;
    }

    m_Dir = NativePath::getDir(m_Path);

    return;
}

bool NativePath::isRelative(const char *path)
{
    const char *pPath;

    if (!path) {
        return false;
    }

    schemeInfo *scheme = NativePath::getScheme(path, &pPath);

    /* We assume that if a "prefix based" path is given, it's an absolute URL */
    if (!SCHEME_MATCH(scheme, "file")) {
        return false;
    }

    /* We are matching a local file scheme (either from file:// or simple path) */
    return pPath[0] != '/';
}

char *NativePath::getDir(const char *fullpath)
{
    int len = strlen(fullpath);
    char *ret;
    if (len == 0) {
        return NULL;
    }
    ret = (char *)malloc(sizeof(char) * (len + 1));
    memcpy(ret, fullpath, len + 1);

    char *pos = strrchr(ret, '/');
    if (pos != NULL) {
        pos[1] = '\0';
    }

    return ret;
}

void NativePath::registerScheme(const NativePath::schemeInfo &scheme,
    bool isDefault)
{
    if (NativePath::g_m_SchemesCount + 1 >= NATIVE_MAX_REGISTERED_SCHEMES) {
        return;
    }

    schemeInfo *newScheme = &NativePath::g_m_Schemes[NativePath::g_m_SchemesCount];

    newScheme->str = strdup(scheme.str);
    newScheme->base = scheme.base;
    newScheme->keepPrefix = scheme.keepPrefix;
    newScheme->getBaseDir = scheme.getBaseDir;
    newScheme->allowLocalFileStream = scheme.allowLocalFileStream;
    newScheme->allowSyncStream = scheme.allowSyncStream;

    NativePath::g_m_SchemesCount++;

    if (isDefault || NativePath::g_m_DefaultScheme == NULL) {
       NativePath::g_m_DefaultScheme = newScheme;
    }
}

void NativePath::unRegisterSchemes()
{
    schemeInfo *scheme;

    for (int i = 0; i < NativePath::g_m_SchemesCount; i++) {
        scheme = &NativePath::g_m_Schemes[i];
        free((char*)scheme->str);
    }
    NativePath::g_m_SchemesCount = 0;
}

NativePath::schemeInfo *NativePath::getScheme(const char *url, const char **pURL)
{
    for (int i = 0; i < NativePath::g_m_SchemesCount; i++) {
        int len = strlen(NativePath::g_m_Schemes[i].str);
        if (strncasecmp(NativePath::g_m_Schemes[i].str, url,
                        len) == 0) {
            bool prefix = NativePath::g_m_Schemes[i].keepPrefix;
            if (pURL) {
                *pURL = (prefix ? url : &url[len]);
            }
            return &NativePath::g_m_Schemes[i];
        }
    }
    if (pURL) {
        *pURL = url;
    }
    return g_m_DefaultScheme;
}

void NativePath::makedirs(const char* dirWithSlashes)
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
            mkdir(tmp, S_IRWXU);
            *p = '/';
        }
    }
    mkdir(tmp, S_IRWXU);

}

char * NativePath::currentJSCaller(JSContext *cx)
{
    if (cx == NULL) {
        /* lookup in the TLS */
        NativeJS *js = NativeJS::getNativeClass();
        if (!js || (cx = js->getJSContext()) == NULL) {
            return NULL;
        }
    }

    unsigned lineno;

    JS::AutoFilename af;
    JS::DescribeScriptedCaller(cx, &af, &lineno);

    return strdup(af.get());
}

#define HANDLE_DOUBLE_DOT()\
    counter--;\
    counterPos = native_max(0, counterPos - 1);\
    if (counter < 0) {\
        outsideRoot = true;\
        if (!isRelative) {\
            if (external) {\
                *external = true;\
            }\
            return nullptr;\
        }\
    }\
    elements[counterPos].clear();\
    minCounter = native_min(counter, minCounter);

// XXX : Only works with path (not URIs)
char *NativePath::sanitize(const char *path, bool *external)
{
    enum {
        PATH_STATE_START,
        PATH_STATE_DOT,
        PATH_STATE_DOUBLE_DOT,
        PATH_STATE_NAME,
        PATH_STATE_SLASH
    } state = PATH_STATE_SLASH;

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
    bool isRelative = NativePath::isRelative(path);

    std::vector<std::string> elements(pathlen);

    for (int i = 0; i < pathlen; i++) {
        switch (path[i]) {
            case '.':
                switch (state) {
                    case PATH_STATE_DOT:
                        state = PATH_STATE_DOUBLE_DOT;
                        break;
                    case PATH_STATE_DOUBLE_DOT:
                        state = PATH_STATE_NAME;
                        break;
                    case PATH_STATE_NAME:
                        elements[counterPos] += path[i];
                        break;
                    case PATH_STATE_SLASH:
                        state = PATH_STATE_DOT;
                        break;
                    default:
                        break;
                }
                break;
            case '/':
                switch (state) {
                    case PATH_STATE_DOT:
                        break;
                    case PATH_STATE_NAME:
                        counter++;
                        counterPos++;
                        break;
                    case PATH_STATE_DOUBLE_DOT:
                        HANDLE_DOUBLE_DOT()
                        break;
                    case PATH_STATE_SLASH:
                        break;
                    default:
                        break;
                }
                state = PATH_STATE_SLASH;
                break;
            default:
                elements[counterPos] += path[i];
                state = PATH_STATE_NAME;
                break;
        }
    }

    if (state == PATH_STATE_DOUBLE_DOT) {
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

    if (finalPath.length() > 0 && state == PATH_STATE_NAME) {
        finalPath[finalPath.length()-1] = '\0';
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

void NativePath::invalidatePath()
{
    free(m_Path);
    free(m_Host);
    free(m_Dir);

    m_Path = nullptr;
    m_Host = nullptr;
    m_Dir = nullptr;
}

bool NativePath::inDir(const char *path, const char *root) 
{
    if (!root) {
        return true;
    }

    int i = 0;
    int diff = 0;

    while (root[i] && path[i] && root[i] == path[i]) {
        i++;
    }

    diff = root[i] || path[i] ? i : -1;

    return diff >= strlen(root);
}

