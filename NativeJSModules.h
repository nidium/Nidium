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

#include "external/SplayTree.h"
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

        bool init();
        bool initJS();
        bool initNative();
        JS::Value require(char *name);

        ~NativeJSModule();
    private:
        JSContext *cx;
        NativeJSModules *modules;

        void setScriptDir();
};

class NativeJSModules
{
    public:
        NativeJSModules(JSContext *cx) : cx(cx), main(NULL)
        {
            tree.setCompare(NativeJSModules::compare); 
        }

        NativeJSModule *main;

        static int compare(NativeJSModule *first, NativeJSModule *second)
        {
            int ret;
            ret = strcmp(first->filePath, second->filePath);
            return ret < 0;
        }

        void add(NativeJSModule *module)
        {
            this->tree.insert(module);
        }

        void remove(NativeJSModule *module)
        {
            this->tree.erase(module);
        }

        NativeJSModule *find(NativeJSModule *module)
        {
            SplayTree<NativeJSModule *>::node *node = this->tree.find(module);

            if (!node) {
                return NULL;
            }

            return node->key;
        }

        JSObject *init(JSObject *scope, const char *name);
        bool init(NativeJSModule *module);
        static char *findModulePath(NativeJSModule *parent, NativeJSModule *module);
        static char *findModuleInPath(NativeJSModule *module, const char *path);
    private:
        SplayTree<NativeJSModule *> tree;
        JSContext *cx;
        bool initJS(NativeJSModule *cmodule);

};

#endif
