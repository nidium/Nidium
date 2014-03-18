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
#include "NativeJS.h"
#include <jsapi.h>
#include <jsdbgapi.h>

char *g_m_Root = NULL;
char *g_m_Pwd = NULL;

int NativePath::g_m_SchemesCount = 0;
struct NativePath::schemeInfo *NativePath::g_m_DefaultScheme = NULL;

void NativePath::registerScheme(const NativePath::schemeInfo &scheme,
    bool isDefault)
{
    if (NativePath::g_m_SchemesCount + 1 >= NATIVE_MAX_REGISTERED_SCHEMES) {
        return;
    }

    memcpy(&g_m_Schemes[NativePath::g_m_SchemesCount],
        &scheme, sizeof(schemeInfo));

    NativePath::g_m_SchemesCount++;

    if (isDefault || g_m_DefaultScheme == NULL) {
        g_m_DefaultScheme = &g_m_Schemes[NativePath::g_m_SchemesCount];
    }
}

NativePath::schemeInfo *NativePath::getScheme(const char *url)
{
    for (int i = 0; i < NativePath::g_m_SchemesCount; i++) {
        int len = strlen(NativePath::g_m_Schemes[i].str);
        if (strncasecmp(NativePath::g_m_Schemes[i].str, url,
                        len) == 0) {
            bool prefix = NativePath::g_m_Schemes[i].keepPrefix;
            return &NativePath::g_m_Schemes[i];
        }
    }

    return g_m_DefaultScheme;
}

const char * NativePath::currentJSCaller(JSContext *cx)
{
    if (cx == NULL) {
        /* lookup in the TLS */
        NativeJS *js = NativeJS::getNativeClass();
        if (!js || (cx = js->getJSContext()) == NULL) {
            return NULL;
        }
    }

    JSScript *parent;
    unsigned lineno;
    JS_DescribeScriptedCaller(cx, &parent, &lineno);
    return JS_GetScriptFilename(cx, parent);
}

int NativePath::getNumPath(const char *path)
{
    enum {
        PATH_STATE_START,
        PATH_STATE_DOT,
        PATH_STATE_DOUBLE_DOT,
        PATH_STATE_NAME,
        PATH_STATE_SLASH
    } state = PATH_STATE_SLASH;

    int counter = 0;
    int pathlen = strlen(path);

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
                        counter++;
                        break;
                    case PATH_STATE_DOUBLE_DOT:
                        counter--;
                        break;
                    case PATH_STATE_NAME:
                        counter++;
                        break;
                    case PATH_STATE_SLASH:
                        break;
                    default:
                        break;
                }
                state = PATH_STATE_SLASH;           
                break;
            default:
                state = PATH_STATE_NAME;
                break;
        }
    }

    return counter;
}
