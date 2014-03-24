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

#include <jsfriendapi.h>
#include <jsapi.h>
#include <jsdbgapi.h>
#include <libgen.h>
#include <unistd.h>
#include <errno.h> 
#include <dlfcn.h> 
#include <sys/stat.h>

#include <NativeJSExposer.h>
#include <NativeJSModules.h>
#include <NativeJS.h>
#include <NativePath.h>
#include <NativeStreamInterface.h>

#include <jsoncpp.h>
#include <json.h>

#include <algorithm>

#if 0
#define DPRINT(...) printf(__VA_ARGS__)
#else
#define DPRINT(...) (void)0
#endif

static void Exports_Finalize(JSFreeOp *fop, JSObject *obj);

static JSClass native_modules_exports_class = {
    "Exports", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, Exports_Finalize,
    JSCLASS_NO_OPTIONAL_MEMBERS
};

static JSClass native_modules_class = {
    "Module", 0,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, NULL,
    JSCLASS_NO_OPTIONAL_MEMBERS
};

static JSPropertySpec native_modules_exports_props[] = {
    {"exports", 0, 0, JSOP_NULLWRAPPER, JSOP_NULLWRAPPER},
    {"module", 0, 0, JSOP_NULLWRAPPER, JSOP_NULLWRAPPER},
    {0, 0, 0, JSOP_NULLWRAPPER, JSOP_NULLWRAPPER}
};

static JSBool native_modules_require(JSContext *cx, unsigned argc, jsval *vp);

NativeJSModule::NativeJSModule(JSContext *cx, NativeJSModules *modules, NativeJSModule *parent, const char *name) 
    : absoluteDir(NULL), filePath(NULL), name(strdup(name)), m_ModuleType(NONE), 
      exports(NULL), parent(parent), modules(modules), cx(cx)
{
}

bool NativeJSModule::initMain() 
{
    this->name = strdup("__MAIN__");

    JSObject *funObj;
    JSFunction *fun = js::DefineFunctionWithReserved(this->cx, JS_GetGlobalObject(cx), 
            "require", native_modules_require, 1, 0);
    if (!fun) {
        return false;
    }

    funObj = JS_GetFunctionObject(fun);

    js::SetFunctionNativeReserved(funObj, 0, PRIVATE_TO_JSVAL((void *)this));

    this->exports = NULL; // Main module is not a real module, thus no exports

    return true;
}
bool NativeJSModule::init()
{
    if (!this->name || strlen(this->name) == 0) return false;
    DPRINT("= NativeJSModule INIT\n");
    DPRINT("name = %s\n", this->name);

    if (this->parent) {
        this->filePath = NativeJSModules::findModulePath(this->parent, this);
    } else {
        this->filePath = realpath(this->name, NULL);
    }

    if (!this->filePath) {
        // Module not found
        return false;
    }

    NativePath p(this->filePath);

    if (!p.dir()) {
        return false;
    }

    this->absoluteDir = strdup(p.dir());
    
    DPRINT("filepath = %s\n", this->filePath);
    DPRINT("name = %s\n", this->name);

    DPRINT("absolute dir for %s\n", this->absoluteDir);

    return true;
}

bool NativeJSModules::init()
{
    char *paths = getenv("NIDIUM_REQUIRE_PATH");

    if (paths) {
        char *token;
        char *originalPaths = strdup(paths);
        char *tmp = originalPaths;
        int i = 0;

        while ((token = strsep(&tmp, ":")) != NULL && i < 63) {
            if (i > 63) {
            } else {
                m_EnvPaths[i] = strdup(token);
                i++;
            }
        }

        if (token != NULL && i == 63) {
            fprintf(stderr, "Warning : require path ignored %s. A maximum of 63 search path is allowed. All subsequent path will be ignored.\n", token);
        }

        m_EnvPaths[i] = NULL;

        free(originalPaths);
    } else {
        m_EnvPaths[0] = NULL;
    }

    this->main = new NativeJSModule(cx, this, NULL, "MAIN");

    if (!main) return false;

    return this->main->initMain();
}

bool NativeJSModules::init(NativeJSModule *module)
{
    switch (module->m_ModuleType) {
        case NativeJSModule::NATIVE:
            if (!module->initNative()) {
                return false;
            }
            break;
        case NativeJSModule::JS:
            if (!module->initJS()) {
                return false;
            }
            break;
    }

    this->add(module);

    return true;
}

bool NativeJSModule::initNative()
{
    JSObject *exports = JS_NewObject(this->cx, NULL, NULL, NULL);
    if (!exports) {
        return false;
    }

    void *module = dlopen(this->filePath, RTLD_LAZY);
    if (!module) {
        printf("Failed to open module : %s\n", dlerror());
        return false;
    }

    register_module_t registerModule = (register_module_t)dlsym(module, "__NativeRegisterModule");
    if (registerModule && !registerModule(this->cx, exports)) {
        printf("Failed to register module\n");
        return false;
    }

    this->exports = exports;
    JS_AddObjectRoot(cx, &this->exports);

    return true;
}

bool NativeJSModule::initJS()
{
#define TRY_OR_DIE(call) if (call == JS_FALSE) { return false; }
    JSObject *gbl = JS_NewObject(this->cx, &native_modules_exports_class, NULL, NULL);
    if (!gbl) {
        return false;
    }

    NativeJS *njs = NativeJS::getNativeClass(this->cx);

    JS_SetPrivate(gbl, this);

    JSObject *funObj;
    JSFunction *fun = js::DefineFunctionWithReserved(this->cx, gbl, "require", native_modules_require, 1, 0);
    if (!fun) {
        return false;
    }

    funObj = JS_GetFunctionObject(fun);

    js::SetFunctionNativeReserved(funObj, 0, PRIVATE_TO_JSVAL((void *)this));

    JSObject *exports = JS_NewObject(this->cx, NULL, NULL, NULL);
    JSObject *module = JS_NewObject(this->cx, &native_modules_class, NULL, NULL);

    if (!exports || !module) {
        return false;
    }

    JS::Value id;
    JSString *idstr = JS_NewStringCopyN(cx, this->name, strlen(this->name));
    if (!idstr) {
        return false;
    }
    id.setString(idstr);

    jsval exportsVal = OBJECT_TO_JSVAL(exports);
    jsval moduleVal  = OBJECT_TO_JSVAL(module);

    TRY_OR_DIE(JS_DefineProperties(cx, gbl, native_modules_exports_props));
    TRY_OR_DIE(JS_SetProperty(cx, gbl, "exports", &exportsVal));

    TRY_OR_DIE(JS_DefineProperties(cx, module, native_modules_exports_props));
    TRY_OR_DIE(JS_SetProperty(cx, gbl, "module", &moduleVal));
    TRY_OR_DIE(JS_SetProperty(cx, module, "id", &id));
    TRY_OR_DIE(JS_SetProperty(cx, module, "exports", &exportsVal));

    js::SetFunctionNativeReserved(funObj, 1, exportsVal);

    this->exports = gbl;
    JS_AddObjectRoot(cx, &this->exports);

    return true;
#undef TRY_OR_DIE
}

void NativeJSModules::dirname(std::string &source)
{
    if (source.size() <= 1) {
        return;
    }

    // Remove trailing slash if it exists.
    if (*(source.rbegin()) == '/') {
        source.resize(source.size() - 1);
    }

    source.erase(std::find(source.rbegin(), source.rend(), '/').base(), source.end());
}

char *NativeJSModules::findModulePath(NativeJSModule *parent, NativeJSModule *module)
{
    std::string modulePath;
    NativeJSModules *modules = module->modules;

    if (module->name[0] == '.') {
        // Relative module, only look in current script directory
        modulePath = NativeJSModules::findModuleInPath(module, parent->absoluteDir);
    }  else if (module->name[0] == '/') {
        modulePath = NativeJSModules::findModuleInPath(module, "");
    }  else {
        std::string path = parent->absoluteDir;
        const char *topDir = module->modules->m_TopDir;
        if (topDir == NULL) {
            topDir = "/";
        }

        DPRINT("[findModulePath] absolute topDir=%s dir=%s path=%s\n", topDir, parent->absoluteDir, path.c_str());

        // Look for the module in all parent directory until it's found
        // or if the top level working directory is reached
        bool stop = false;
        do {
            // Try local search path
            for (int i = 0; i < NATIVE_MODULES_PATHS_COUNT && modulePath.empty(); i++) {
                std::string currentPath = path;
                currentPath += modules->m_Paths[i];

                DPRINT("Looking for module %s in %s\n", module->name, currentPath.c_str());
                modulePath = NativeJSModules::findModuleInPath(module, currentPath.c_str());
                DPRINT("module path is %s\n", modulePath.c_str());
            }

            stop = (strcmp(topDir, path.c_str()) == 0);
            // Try again with parent directory
            if (!stop) {
                DPRINT("  Getting parent dir for %s\n", path.c_str());
                NativeJSModules::dirname(path);
                DPRINT("  Parent path is         %s\n", path.c_str());
            }
        } while (modulePath.empty() && !stop);

        if (modulePath.empty()) {
            // Check in system directories if module hasn't been found
            for (int i = 0; modules->m_EnvPaths[i] != NULL && modulePath.empty(); i++) {
                char *tmp = modules->m_EnvPaths[i];
                DPRINT("Looking for module %s in %s\n", module->name, tmp);
                modulePath = NativeJSModules::findModuleInPath(module, tmp);
                DPRINT("module path is %s\n", modulePath.c_str());
            }
        }
    }

    if (modulePath.empty()) {
        return NULL;
    }

    return realpath(modulePath.c_str(), NULL);
}

bool NativeJSModules::getFileContent(const char *file, char **content, size_t *size)
{
    NativePath path(file);
    NativeBaseStream *stream = path.createStream(true);

    if (!stream) {
        return false;
    }

    stream->getContentSync(content, size);

    delete stream;

    return true;
}

std::string NativeJSModules::findModuleInPath(NativeJSModule *module, const char *path) 
{
#define MAX_EXT_SIZE 13
    const char *extensions[] = {NULL, ".js", DSO_EXTENSION, ".json"};

    std::string tmp = std::string(path) + std::string("/") + std::string(module->name);
    size_t len = tmp.length();

    DPRINT("    tmp is %s\n", tmp.c_str());

    for (int i = 0; i < 4; i++) {
        if (extensions[i]) {
            tmp.erase(len);
            tmp += extensions[i];
        }

        DPRINT("    [NativeJSModule] Looking for %s\n", tmp.c_str());

        if (access(tmp.c_str(), F_OK) != 0) {
            continue;
        }

        // XXX : Refactor this code. It's a bit messy.
        switch (i) {
            case 0: // directory or exact filename
                module->m_ModuleType = NativeJSModule::JS;
                struct stat sb;
                if (stat(tmp.c_str(), &sb) == 0 && S_ISDIR(sb.st_mode)) {
                    if (NativeJSModules::loadDirectoryModule(tmp)) {
                        return tmp;
                    }
                    DPRINT("%s\n", tmp.c_str());
                } else {
                    // Exact filename, find the extension
                    size_t pos = tmp.find_last_of('.');
                    // No extension, assume it's a JS file
                    if (pos == std::string::npos) {
                        DPRINT("      No extension found. Assuming JS file\n");
                        module->m_ModuleType = NativeJSModule::JS;
                        return tmp;
                    }

                    for (int j = 1; j < 4; j++) {
                        size_t tmpPos = tmp.find(extensions[j], pos, tmp.length() - pos);
                        if (tmpPos != std::string::npos) {
                            switch (j) {
                                case 1:
                                    module->m_ModuleType = NativeJSModule::JS;
                                    break;
                                case 2:
                                    module->m_ModuleType = NativeJSModule::NATIVE;
                                    break;
                                case 3:
                                    module->m_ModuleType = NativeJSModule::JSON;
                                    break;
                            }
                            return tmp;
                        }
                    }
                }
                continue;
            case 1: // .js
                module->m_ModuleType = NativeJSModule::JS;
                break;
            case 2: // native module
                module->m_ModuleType = NativeJSModule::NATIVE;
                break;
            case 3: // json file
                module->m_ModuleType = NativeJSModule::JSON;
                break;
        }
        DPRINT("    [NativeJSModule] FOUND IT\n");
        return tmp;
    }

    return std::string();
}

bool NativeJSModules::loadDirectoryModule(std::string &dir)
{
    const char *files[] = {"/index.js", "/package.json"};
    size_t len = dir.length();

    for (int i = 0; i < 2; i++) {
        dir.erase(len);
        dir += files[i];

        if (access(dir.c_str(), F_OK) != 0) continue;

        switch (i) {
            case 0: // index.js
                return true;
                break;
            case 1: // package.json
                char *data;
                size_t size;

                if (!NativeJSModules::getFileContent(dir.c_str(), &data, &size)) {
                    fprintf(stderr, "Failed to open %s", dir.c_str());
                    return false;
                }

                Json::Value root;
                Json::Reader reader;
                bool parsingSuccessful = reader.parse(data, data + size, root);

                free(data);

                if (!parsingSuccessful) {
                    fprintf(stderr, "Failed to parse %s\n  %s", dir.c_str(), reader.getFormatedErrorMessages().c_str());
                    return false;
                }

                std::string main = root.get("main", "").asString();
                dir.erase(len);
                dir += std::string("/") + main;

                if (access(dir.c_str(), F_OK) != 0) {
                    fprintf(stderr, "Failed to access file %s", dir.c_str());
                    return false;
                }

                return true;
                break;
        }

    }

    return false;
}

#undef MAX_EXT_SIZE 


JS::Value NativeJSModule::require(char *name)
{
    JS::Value ret;
    NativeJSModule *cmodule;

    ret.setUndefined();

    // require() have been called from the main module
    if (this == this->modules->main) {
        /*
         * This little hack is needed to conform CommonJS : 
         *  - Cyclic deps
         *  - Finding module
         * 
         * Since all files included with NativeJS::LoadScript();
         * share the same module we need to be aware of the real caller.
         * So here we set the filename and path of the caller 
         * 
         * XXX : Another way to handle this case would be to make
         * load() aware of his context by using the same trick 
         * require do.
         */
        JSScript *script;
        unsigned lineno;
        JS_DescribeScriptedCaller(this->cx, &script, &lineno);

        free(this->filePath);
        free(this->absoluteDir);

        // filePath is needed for cyclic deps check
        this->filePath = realpath(strdup(JS_GetScriptFilename(this->cx, script)), NULL);
        // absoluteDir is needed for findModulePath
        NativePath p(this->filePath);
        this->absoluteDir = strdup(p.dir());
        DPRINT("Global scope loading\n");
    } else {
        DPRINT("Module scope loading\n");
    }

    DPRINT("[NativeJSModule] Module %s require(%s)\n", this->name, name);

    NativeJSModule *tmp = new NativeJSModule(cx, this->modules, this, name);
    if (!tmp->init()) {
        JS_ReportError(cx, "Module %s not found\n", name);
        delete tmp;
        return ret;
    }

    // Let's see if the module is in the cache
    NativeJSModule *cached = this->modules->find(tmp);
    if (!cached) {
        DPRINT("Module is not cached\n");
        cmodule = tmp;
    } else {
        DPRINT("Module is cached %s\n", cached->filePath);
        cmodule = cached;
        delete tmp;
    }

    // Is there is a cyclic dependency
    for (NativeJSModule *m = cmodule->parent;;) {
        if (!m || !m->exports) break;

        // Found a cyclic dependency
        if (strcmp(cmodule->filePath, m->filePath) == 0) {
            JSObject *gbl = m->exports;
            jsval module;
            JS_GetProperty(cx, gbl, "module", &module);
            JS_GetProperty(cx, JSVAL_TO_OBJECT(module), "exports", &ret);
            return ret;
        }

        m = m->parent;
    }

    if (!cached) {
        if (!this->modules->init(cmodule)) {
            JS_ReportError(cx, "Failed to initialize module %s\n", cmodule->name);
            return ret;
        }

        if (cmodule->m_ModuleType == JSON || cmodule->m_ModuleType == JS) {
            size_t filesize;
            char *data;

            JSFunction *fn;
            jsval rval;
    
            if (!NativeJSModules::getFileContent(cmodule->filePath, &data, &filesize)) {
                JS_ReportError(cx, "Failed to open module %s\n", cmodule->name);
                return ret;
            }

            if (filesize == 0) {
                free(data);
                ret.setObject(*cmodule->exports);
                return ret;
            }

            if (cmodule->m_ModuleType == JS) {
                fn = JS_CompileFunction(cx, cmodule->exports, 
                        "", 0, NULL, data, strlen(data), cmodule->filePath, 0);
                if (!fn) {
                    free(data);
                    return ret;
                }

                free(data);

                if (!JS_CallFunction(cx, cmodule->exports, fn, 0, NULL, &rval)) {
                    return ret;
                }
            } else {
                // TODO: data leaks here
                size_t len;
                jschar *jchars;
                jsval jsonData;

                if (!JS_DecodeBytes(cx, data, filesize, NULL, &len)) {
                    return ret;
                }

                jchars = (jschar *)JS_malloc(cx, len * sizeof(jschar));
                if (!jchars) {
                    return ret;
                }

                if (!JS_DecodeBytes(cx, data, filesize, jchars, &len)) {
                    JS_free(cx, jchars);
                    return ret;
                }

                if (!JS_ParseJSON(cx, jchars, len, &jsonData)) {
                    JS_free(cx, jchars);
                    return ret;
                }

                JS_free(cx, jchars);

                cmodule->exports = JSVAL_TO_OBJECT(jsonData);
                JS_AddObjectRoot(cx, &cmodule->exports);
            }
        }
    } 

    switch (cmodule->m_ModuleType) {
        case JS:
        {
            JS::Value module;
            JS_GetProperty(cx, cmodule->exports, "module", &module);
            JS_GetProperty(cx, module.toObjectOrNull(), "exports", &ret);
        }
        break;
        case JSON:
        case NATIVE:
        {
            ret = OBJECT_TO_JSVAL(cmodule->exports);
        }
        break;
    }

    return ret;
}

NativeJSModule::~NativeJSModule()
{
    if (this->filePath) {
        this->modules->remove(this);
    }

    if (this->exports != NULL) {
        JS_RemoveObjectRoot(this->cx, &this->exports);
    }

    free(this->name);
    free(this->absoluteDir);
    free(this->filePath);

}

static JSBool native_modules_require(JSContext *cx, unsigned argc, jsval *vp)
{
    JSString *name = NULL;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "S", &name)) {
        return false;
    }

    JSAutoByteString namestr(cx, name);

    JSObject *callee = (JS_CALLEE(cx, vp)).toObjectOrNull();
    JS::Value reserved = js::GetFunctionNativeReserved(callee, 0);
    if (!reserved.isDouble()) {
        JS_ReportError(cx, "InternalError");
        return false;
    }

    NativeJSModule *module = static_cast<NativeJSModule *>(JSVAL_TO_PRIVATE(reserved));

    JS::Value ret = module->require(namestr.ptr());

    JS_SET_RVAL(cx, vp, ret);

    if (ret.isUndefined()) {
        return false;
    }

    return true;
}

void Exports_Finalize(JSFreeOp *fop, JSObject *obj)
{
    void *priv = JS_GetPrivate(obj);
    if (priv != NULL) {
        NativeJSModule *module = static_cast<NativeJSModule *>(priv);
        delete module;
    }
}
