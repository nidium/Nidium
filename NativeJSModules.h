/*
    NativeJS Core Library
    Copyright (C) 2013 Anthony Catel <paraboul@gmail.com>
    Copyright (C) 2013 Nicolas Trani <n.trani@weelya.com>

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

#ifndef nativejsmodules_h__
#define nativejsmodules_h__

#include <jspubtd.h>
#include <NativeHash.h>
#include <string>

#define NATIVE_MODULES_PATHS_COUNT 2

class NativeJSModules;

class NativeJSModule
{
    public:
        NativeJSModule(JSContext *cx, NativeJSModules *modules, NativeJSModule *parent, const char *name);

        enum ModuleType {
            NONE, JS, NATIVE, JSON
        };

        char *absoluteDir;
        char *filePath;
        char *name;
        int m_ModuleType;

        JSObject *exports;

        NativeJSModule *parent;
        NativeJSModules *modules;

        bool init();
        bool initJS();
        bool initMain();
        bool initNative();

        JS::Value require(char *name);

        ~NativeJSModule();
    private:
        JSContext *cx;

        JS::Value load(JS::Value &scope);
};

class NativeJSModules
{
    public:
        NativeJSModules(JSContext *cx) : m_TopDir(NULL), cx(cx)
        {
            m_Paths[0] = (const char *)"nidium_modules";
            m_Paths[1] = (const char *)"node_modules";
        }

        ~NativeJSModules()
        {
            for (int i = 0; m_EnvPaths[i] != NULL; i++) {
                free(m_EnvPaths[i]);
            }
            delete this->main;
        }

        NativeJSModule *main;
        const char *m_TopDir;

        void add(NativeJSModule *module)
        {
            m_Cache.set(module->filePath, module);
        }

        void remove(NativeJSModule *module)
        {
            m_Cache.erase(module->filePath);
        }

        NativeJSModule *find(NativeJSModule *module)
        {
            return m_Cache.get(module->filePath);
        }

        void setPath(const char *topDir)
        {
            m_TopDir = topDir;
        }

        bool init();
        bool init(NativeJSModule *module);

        static char *findModulePath(NativeJSModule *parent, NativeJSModule *module);
        static bool getFileContent(const char *file, char **content, size_t *size);
    private:
        NativeHash<NativeJSModule *> m_Cache;
        const char *m_Paths[2];
        char *m_EnvPaths[64];
        JSContext *cx;

        bool initJS(NativeJSModule *cmodule);

        static std::string findModuleInPath(NativeJSModule *module, const char *path);
        static bool loadDirectoryModule(std::string &dir);

        static void dirname(std::string &source);
};

#endif
