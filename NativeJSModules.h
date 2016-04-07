/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef nativejsmodules_h__
#define nativejsmodules_h__

#include <string.h>

#include <jsapi.h>
#include <jspubtd.h>

#include "NativeHash.h"

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
        bool m_Cached;

        JS::Heap<JSObject *> exports;

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
        NativeJSModules(JSContext *cx) : m_TopDir("/"), cx(cx)
        {
            m_Paths[0] = (const char *)"modules";
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
            module->m_Cached = true;
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

