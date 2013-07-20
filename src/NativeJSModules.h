#ifndef nativejsmodules_h__
#define nativejsmodules_h__

#include "external/SplayTree.h"

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
