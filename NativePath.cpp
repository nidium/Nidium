/*
    NativeJS Core Library
    Copyright (C) 2014 Anthony Catel <paraboul@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
#include "NativePath.h"

#include <string>
#include <sys/stat.h>
#include <vector>

#include <js/OldDebugAPI.h>

#include "NativeUtils.h"
#include "NativeJS.h"

char *g_m_Root = NULL;
char *g_m_Pwd = NULL;

int NativePath::g_m_SchemesCount = 0;
struct NativePath::schemeInfo *NativePath::g_m_DefaultScheme = NULL;
struct NativePath::schemeInfo NativePath::g_m_Schemes[NATIVE_MAX_REGISTERED_SCHEMES] = {};

NativePath::NativePath(const char *origin, bool allowAll, bool noFilter) :
    m_Path(NULL), m_Dir(NULL), m_Scheme(NULL)
{
    if (origin == NULL) {
        return;
    }
    const char *pOrigin;
    int originlen = strlen(origin);

    if (originlen > MAXPATHLEN-1 || !originlen/* || origin[originlen-1] == '/'*/) {
        return;
    }
    m_Path = (char *)malloc(sizeof(char) * (MAXPATHLEN*4 + 1));
    m_Path[0] = '\0';

    if (!NativePath::getPwd() && URLSCHEME_MATCH(origin, "file")) {
        realpath(origin, m_Path);
        m_Scheme = NativePath::getScheme(origin, &pOrigin);
    } else if (!NativePath::getPwd() || noFilter) {
        m_Scheme = NativePath::getScheme(origin, &pOrigin);
        if (m_Scheme->getBaseDir() != NULL) {
            strcat(m_Path, m_Scheme->getBaseDir());

        }
        strcat(m_Path, pOrigin);
    } else {
        bool outsideRoot      = false;
        schemeInfo *pwdScheme = NativePath::getPwdScheme();
        schemeInfo *scheme    = NativePath::getScheme(origin, &pOrigin);
        char *sanitized       = NativePath::sanitize(pOrigin, &outsideRoot);

        /* Relative Path */

        if (this->isRelative(origin)) {
            if (outsideRoot) {
                this->invalidatePath();
                return;
            }

            strcat(m_Path, NativePath::getPwd());
            strcat(m_Path, &sanitized[2]);
            free(sanitized);
            m_Scheme = pwdScheme;

            this->setDir();
            return;
        }

        /* Absolute path */

        if (!allowAll && SCHEME_MATCH(scheme, "file") &&
            !pwdScheme->allowLocalFileStream()) {

            this->invalidatePath();
            return;
        }
        const char *baseDir;

        if ((baseDir = scheme->getBaseDir()) != NULL) {
            if (outsideRoot) {
                this->invalidatePath();
                return;
            }

            strcat(m_Path, baseDir);
            strcat(m_Path, &sanitized[2]);
            m_Scheme = scheme;

            this->setDir();

            return;
        }

        if (SCHEME_MATCH(scheme, "file")) {
            strcat(m_Path, NativePath::getRoot());
            strcat(m_Path, &sanitized[2]);

        } else {
            strcat(m_Path, origin);
        }

        free(sanitized);

        m_Scheme = scheme;

    }

    this->setDir();
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

void NativePath::setDir()
{
    int len = strlen(m_Path);
    if (len == 0) {
        m_Dir = NULL;
        return;
    }
    m_Dir = (char *)malloc(sizeof(char) * (len + 1));
    memcpy(m_Dir, m_Path, len + 1);

    char *pos = strrchr(m_Dir, '/');
    if (pos != NULL) {
        pos[1] = '\0';
    }
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

char *NativePath::sanitize(const char *path, bool *external, bool relative)
{
    enum {
        PATH_STATE_START,
        PATH_STATE_DOT,
        PATH_STATE_DOUBLE_DOT,
        PATH_STATE_NAME,
        PATH_STATE_SLASH
    } state = PATH_STATE_SLASH;

    int pathlen = strlen(path);

    if (external) {
        *external = false;
    }

    if (pathlen == 0) {
        return NULL;
    }

    int counter = 0, minCounter = 0, counterPos = 0;
    bool outsideRoot = false;

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
                        counter--;
                        counterPos = native_max(0, counterPos - 1);
                        if (counter < 0) {
                            outsideRoot = true;
                        }
                        elements[counterPos].clear();
                        minCounter = native_min(counter, minCounter);
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

    std::string finalPath;
    if (outsideRoot) {
        for (int i = minCounter; i != 0; i++) {
            finalPath += "../";
        }
    } else {
        finalPath += (relative ? "./" : "/");
    }

    for (int i = 0; elements[i].length() != 0; i++) {
        finalPath += elements[i] + "/";
    }

    if (finalPath.length() > 0 && path[pathlen-1] != '/' &&
        finalPath[finalPath.length()-1] == '/') {

        finalPath[finalPath.length()-1] = '\0';
    }

    if (external) {
        *external = outsideRoot;
    }
    if (strcmp(finalPath.c_str(), ".") == 0 && strlen(finalPath.c_str()) == 1) {
        return strdup("./");
    }
    return strdup(finalPath.c_str());
}

