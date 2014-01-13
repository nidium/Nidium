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
#include <string.h>

class NativeJSModules;

class NativeJSModule
{
    public:
        NativeJSModule(JSContext *cx, NativeJSModules *modules, NativeJSModule *parent, const char *name);

        char *dir;
        char *absoluteDir;
        char *filePath;
        char *name;
        bool native;

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

        void setScriptDir();
};

class NativeJSModules
{
    public:
        NativeJSModules(JSContext *cx) : cx(cx) {}

        ~NativeJSModules()
        {
            delete this->main;
        }

        NativeJSModule *main;
        const char *m_TopDir;

        bool init()
        {
            this->main = new NativeJSModule(cx, this, NULL, "MAIN");

            if (!main) return false;

            return this->main->initMain();
        }

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

        JSObject *init(JSObject *scope, const char *name);
        bool init(NativeJSModule *module);
        static char *findModulePath(NativeJSModule *parent, NativeJSModule *module);
        static char *findModuleInPath(NativeJSModule *module, const char *path);
    private:
        NativeHash<NativeJSModule *> m_Cache;
        JSContext *cx;
        bool initJS(NativeJSModule *cmodule);

};

#endif
