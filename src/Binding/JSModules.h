/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef binding_jsmodules_h__
#define binding_jsmodules_h__

#include <string.h>

#include <jsapi.h>
#include <jspubtd.h>

#include "Core/Hash.h"


namespace Nidium {
namespace Binding {

#define NIDIUM_MODULES_PATHS_COUNT 2

class JSModules;

// {{{ JSModule

class JSModule
{
    public:
        JSModule(JSContext *cx, JSModules *modules, JSModule *parent, const char *name);

        enum ModuleType {
            NONE, JS, NIDIUM, JSON
        };

        char *absoluteDir;
        char *filePath;
        char *name;
        int m_ModuleType;
        bool m_Cached;

        JS::Heap<JSObject *> exports;

        JSModule *parent;
        JSModules *modules;

        bool init();
        bool initJS();
        bool initMain();
        bool initNidium();

        JS::Value require(char *name);

        ~JSModule();
    private:
        JSContext *cx;

        JS::Value load(JS::Value &scope);
};

// {{{ JSModules

class JSModules
{
    public:
        JSModules(JSContext *cx) : m_TopDir("/"), cx(cx)
        {
            m_Paths[0] = (const char *)"modules";
            m_Paths[1] = (const char *)"node_modules";
        }

        ~JSModules()
        {
            for (int i = 0; m_EnvPaths[i] != NULL; i++) {
                free(m_EnvPaths[i]);
            }
            delete this->main;
        }

        JSModule *main;
        const char *m_TopDir;

        void add(JSModule *module)
        {
            m_Cache.set(module->filePath, module);
            module->m_Cached = true;
        }

        void remove(JSModule *module)
        {
            m_Cache.erase(module->filePath);
        }

        JSModule *find(JSModule *module)
        {
            return m_Cache.get(module->filePath);
        }

        void setPath(const char *topDir)
        {
            m_TopDir = topDir;
        }

        bool init();
        bool init(JSModule *module);

        static char *findModulePath(JSModule *parent, JSModule *module);
        static bool getFileContent(const char *file, char **content, size_t *size);
    private:
        Nidium::Core::Hash<JSModule *> m_Cache;
        const char *m_Paths[2];
        char *m_EnvPaths[64];
        JSContext *cx;

        bool initJS(JSModule *cmodule);

        static std::string findModuleInPath(JSModule *module, const char *path);
        static bool loadDirectoryModule(std::string &dir);

        static void dirname(std::string &source);
};

} // namespace Binding
} // namespace Nidium

#endif

