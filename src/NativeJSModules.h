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
        char *fileName;
        char *name;

        JSObject *exports;

        NativeJSModule *parent;

        bool init();
        bool initJS();
        JS::Value require(char *name);

        ~NativeJSModule();
    private:
        JSContext *cx;
        NativeJSModules *main;

        void setScriptDir();
        char *findModulePath(NativeJSModule *module);
};

class NativeJSModules
{
    public:
        NativeJSModules(JSContext *cx) : cx(cx)
        {
            tree.setCompare(NativeJSModules::compare); 
        };

        static bool compare(NativeJSModule *first, NativeJSModule *second)
        {
            bool ret = strcmp(first->filePath, second->filePath) != 0;
            return ret;
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

        char *normalizeName(const char *name);
        JSObject *init(JSObject *scope, const char *name);
        bool init(NativeJSModule *module);
    private:
        SplayTree<NativeJSModule *> tree;
        JSContext *cx;
        bool initJS(NativeJSModule *cmodule);

};

#endif
