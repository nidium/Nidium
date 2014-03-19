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

/*
    /!\ this class is not thread safe
    TODO: windows ('shlwapi') http://msdn.microsoft.com/en-us/library/windows/desktop/bb773559%28v=vs.85%29.aspx
*/

#ifndef nativepath_h__
#define nativepath_h__

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/param.h>
#include <ape_array.h>

#define NATIVE_MAX_REGISTERED_SCHEMES 1024

struct JSContext;
class NativeBaseStream;

extern char *g_m_Root;
extern char *g_m_Pwd;

#define SCHEME_DEFINE(prefix, streamclass, keepprefix) (struct NativePath::schemeInfo){prefix, streamclass::createStream, keepprefix}
#define URLSCHEME_MATCH(url, scheme) (strcmp(NativePath::getScheme(url)->str, scheme "://") == 0)
#define SCHEME_MATCH(obj, scheme) (strcmp(obj->str, scheme "://") == 0)

class NativePath
{
public:
    struct schemeInfo {
        const char *str;
        NativeBaseStream *(*base)(const char *);
        bool keepPrefix;
    };

    /*
        allowAll defines if origin must match "pwd" stream class (if false)
    */
    explicit NativePath(const char *origin, bool allowAll = false);

    operator const char *() {
        printf("Returned path : %s\n", m_Path);
        return m_Path;
    }

    const char *path() const {
        return m_Path;
    }

    const char *dir() const {
        return m_Dir;
    }

    bool isRelative(const char *path);

    ~NativePath(){
        if (m_Path) {
            free(m_Path);
        }
    };

    static void registerScheme(const schemeInfo &scheme,
        bool isDefault = false);
    static schemeInfo *getScheme(const char *url, const char **pURL = NULL);

    static char *sanitize(const char *path, bool *external = NULL);

    static void chroot(const char *root) {
        if (g_m_Root != NULL && root != g_m_Root) {
            free(g_m_Root);
        }
        g_m_Root = strdup(root);
    }

    static void cd(const char *dir) {
        if (g_m_Pwd != NULL && dir != g_m_Pwd) {
            free(g_m_Pwd);
        }
        g_m_Pwd = strdup(dir);
    }

    static const char *getRoot() {
        return g_m_Root;
    }

    static const char *getPwd() {
#if 0
        if (g_m_Pwd == NULL) {
            static char cpwd[MAXPATHLEN];
            getcwd(cpwd, MAXPATHLEN);
            return cpwd;
        }
#endif
        return g_m_Pwd;        
    }

    static const char *currentJSCaller(JSContext *cx = NULL);
    static int g_m_SchemesCount;
    static struct schemeInfo g_m_Schemes[NATIVE_MAX_REGISTERED_SCHEMES];
    static struct schemeInfo *g_m_DefaultScheme;
private:
    void setDir();

    char *m_Path;
    char *m_Dir;
};


#endif