#include <jsfriendapi.h>
#include <jsapi.h>
#include <jsdbgapi.h>
#include <libgen.h>
#include <unistd.h>
#include <errno.h> 
#include <dlfcn.h> 

#include <NativeJSExposer.h>
#include <NativeJSModules.h>
#include <NativeJS.h>

#define NJS ((class NativeJS *)JS_GetRuntimePrivate(JS_GetRuntime(cx)))

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

static JSPropertySpec native_modules_props[] = {
    {"id", 0, 
        JSPROP_ENUMERATE|JSPROP_PERMANENT|JSPROP_READONLY, 
        JSOP_NULLWRAPPER, JSOP_NULLWRAPPER},
    {"exports", 0, 
        0, 
        JSOP_NULLWRAPPER, JSOP_NULLWRAPPER},
    {0, 0, 0, JSOP_NULLWRAPPER, JSOP_NULLWRAPPER}
};

static JSBool native_modules_require(JSContext *cx, unsigned argc, jsval *vp);

NativeJSModule::NativeJSModule(JSContext *cx, NativeJSModules *modules, NativeJSModule *parent, const char *name) 
    : dir(NULL), absoluteDir(NULL), filePath(NULL), name(strdup(name)), native(false), exports(NULL),
      cx(cx), modules(modules), parent(parent)
{
}

bool NativeJSModule::init()
{
    if (!this->name || strlen(this->name) == 0) return false;

    if (this->parent) {
        this->filePath = this->parent->findModulePath(this);
    } else {
        this->filePath = realpath(this->name, NULL);
    }
    if (!this->filePath) {
        return false;
    }

    this->dir = (char *)dirname(strdup(this->filePath));
    if (!this->dir) {
        return false;
    }

    // For absolute module, if the module name contain directories ("/") 
    // we must remove them for the absoluteDir
    if (this->name[0] != '.') {
        int count = 0;
        for (int i = 0; this->name[i]; i++) {
            count += this->name[i] == '/';
        }

        // module name contains a "/"
        if (count > 0) {
            int dirCount = 0;
            int last = 0;
            size_t len = strlen(this->filePath);

            for (int i = len; i > -1; i--) {
                if (this->filePath[i] == '/') {
                    last = i;
                    if (dirCount == count) {
                        break;
                    }
                    dirCount++;
                }
            }

            this->absoluteDir = (char *)malloc(sizeof(char) * last + 1);
            if (!this->absoluteDir) {
                return false;
            }

            memcpy(this->absoluteDir, this->filePath, last);

            this->absoluteDir[last] = '\0';
        } else {
            this->absoluteDir = strdup(this->dir);
            if (!this->absoluteDir) {
                return false;
            }
        }
    } else {
        this->absoluteDir = strdup(this->dir);
        if (!this->absoluteDir) {
            return false;
        }
    }

    return true;
}

JSObject *NativeJSModules::init(JSObject *scope, const char *name)
{
    NativeJSModule *cmodule = NULL;
    // Top level scope, we need to create a module for it
    if (scope == JS_GetGlobalObject(cx)) {
        if (!this->main) {
            cmodule = new NativeJSModule(this->cx, this, NULL, name);
            if (!cmodule || !cmodule->init()) {
                return NULL;
            }

            if (!this->init(cmodule)) {
                return NULL;
            }

            this->main = cmodule;
        } else {
            cmodule = this->main;
        }
    }

    return cmodule ? cmodule->exports : scope;
}

bool NativeJSModules::init(NativeJSModule *module)
{
    if (module->native) {
        if (!module->initNative()) {
            return false;
        }
    } else {
        if (!module->initJS()) {
            return false;
        }
    }

    // Main "module" is not a real module
    // so we don't want have it in cache
    if (module != this->main) {
        this->add(module);
    }

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
        return false;
    }

    register_module_t registerModuleObject = (register_module_t)dlsym(module, "__NativeRegisterModuleObject");
    if (registerModuleObject) {
        registerModuleObject(this->cx, exports);
    }

    register_module_t registerModule = (register_module_t)dlsym(module, "__NativeRegisterModule");
    if (registerModule) {
        registerModule(this->cx, exports);
    }

    this->exports = exports;

    return true;
}

bool NativeJSModule::initJS()
{
#define TRY_OR_DIE(call) if (call == JS_FALSE) { return NULL; }
    JSObject *gbl = JS_NewObject(this->cx, &native_modules_exports_class, NULL, NULL);
    JSObject *exports = JS_NewObject(this->cx, NULL, NULL, NULL);
    JSObject *module = JS_NewObject(this->cx, &native_modules_class, NULL, NULL);

    if (!gbl || !exports || !module) {
        return false;
    }

    JS_SetPrivate(gbl, this);

    JSObject *funObj;
    JSFunction *fun = js::DefineFunctionWithReserved(this->cx, gbl, "require", native_modules_require, 1, 0);
    if (!fun) {
        return false;
    }

    funObj = JS_GetFunctionObject(fun);

    js::SetFunctionNativeReserved(funObj, 0, PRIVATE_TO_JSVAL((void *)this));

    //JSCompartment *ca = JS_EnterCompartment(cx, gbl);
    //JS_WrapObject(cx, gbl)
    jsval exportsVal = OBJECT_TO_JSVAL(exports);
    jsval moduleVal  = OBJECT_TO_JSVAL(module);

    JS::Value id;
    JSString *idstr = JS_NewStringCopyN(cx, this->name, strlen(this->name));
    if (!idstr) {
        return false;
    }
    id.setString(idstr);

    // Root the global and
    NJS->rootObjectUntilShutdown(gbl);

    // And add all objects to require reserved slot so they won't be GC
    JS::Value roots[2];
    roots[0] = exportsVal;
    roots[1] = moduleVal;
    roots[2] = id;

    JSObject *rootsObj = JS_NewArrayObject(cx, 2, roots);
    js::SetFunctionNativeReserved(funObj, 1, exportsVal);

    TRY_OR_DIE(JS_DefineProperties(cx, gbl, native_modules_exports_props));
    TRY_OR_DIE(JS_SetProperty(cx, gbl, "exports", &exportsVal));

    TRY_OR_DIE(JS_DefineProperties(cx, module, native_modules_exports_props));
    TRY_OR_DIE(JS_SetProperty(cx, gbl, "module", &moduleVal));
    TRY_OR_DIE(JS_SetProperty(cx, module, "id", &id));
    TRY_OR_DIE(JS_SetProperty(cx, module, "exports", &exportsVal));

    this->exports = gbl;

    return true;
#undef TRY_OR_DIE
}

char *NativeJSModule::findModulePath(NativeJSModule *module)
{
#define PATHS_COUNT 4
#define MAX_EXT_SIZE 4
    // Setup search paths
    char *modulePath = NULL;
    char *paths[PATHS_COUNT];
    #define MTOSTR(s) #s
    const char *extensions[] = { MTOSTR(DSO_EXTENSION), ".js", NULL};
    #undef MTOSTR
    
    memset(paths, 0, sizeof(char*) * PATHS_COUNT);

    // TODO : Path handling will need to be updated to 
    // support CommonJS "path" specification
    if (module->name[0] == '.') {
        // Relative module, only look in current script directory
        paths[0] = this->dir;
    } else {
        paths[0] = this->absoluteDir;                   // Module search path
        paths[1] = (char *)".";                         // Native root
        paths[PATHS_COUNT-2] = (char *)"node_modules";  // Compatibility with NodeJS npm
        paths[PATHS_COUNT-1] = (char *)"modules";       // Root modules directory
    }

    // Check for existence of each module extensions in each paths
    bool found = false;
    for (int i = 0; i < PATHS_COUNT && !found; i++) {
        if (!paths[i]) continue;

        char *tmp = (char *)calloc(sizeof(char), strlen(paths[i]) + strlen(module->name) + MAX_EXT_SIZE + 3);
        if (!tmp) {
            JS_ReportOutOfMemory(cx);
            return NULL;
        }

        strcat(tmp, paths[i]);
        strcat(tmp, "/");
        strcat(tmp, module->name);

        size_t end = strlen(tmp);
        for (int j = 0; j < 3; j++) {
            if (extensions[j]) {
                const char *c = &extensions[j][0];
                for (int k = 0; k < MAX_EXT_SIZE && *c != '\0'; k++) {
                    tmp[end + k] = *c++;
                }
            }

            //printf("    [NativeJSModule] Looking for %s\n", tmp);

            if (access(tmp, F_OK) == 0) {
                //printf("    [NativeJSModule] FOUND IT\n");
                if (j == 0) {
                    module->native = true;
                }
                modulePath = tmp;
                found = true;
                break;
            }
        }

        if (!found) {
            free(tmp);
        }
    }

    if (!modulePath) {
        JS_ReportError(this->cx, "No module named %s", module->name);
        return NULL;
    }

    char *rpath = realpath(modulePath, NULL);

    free(modulePath);

    return rpath;
#undef PATHS_COUNT
#undef MAX_EXT_SIZE 
}

JS::Value NativeJSModule::require(char *name)
{
    JS::Value ret;
    NativeJSModule *cmodule;

    ret.setNull();

    // require() have been called from the main module
    if (this == this->modules->main) {
        /*
         * This little hack is need to conform CommonJS : 
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
        const char *filename;

        JS_DescribeScriptedCaller(this->cx, &script, &lineno);

        free(this->filePath);
        free(this->absoluteDir);

        // filePath is needed for cyclic deps check
        this->filePath = realpath(strdup(JS_GetScriptFilename(this->cx, script)), NULL);
        // absoluteDir is needed for findModulePath
        this->absoluteDir = dirname(strdup(this->filePath));
    }

    //printf("[NativeJSModule] Module %s require(%s)\n", this->name, name);

    NativeJSModule *tmp = new NativeJSModule(cx, this->modules, this, name);
    if (!tmp->init()) {
        JS_ReportError(cx, "Module %s not found\n", name);
        delete tmp;
        return ret;
    }

    // Let's see if the module is in the cache
    NativeJSModule *cached = this->modules->find(tmp);
    if (!cached) {
        cmodule = tmp;
    } else {
        cmodule = cached;
        delete tmp;
    }

    // Is there is a cyclic dependency
    for (NativeJSModule *m = cmodule->parent;;) {
        if (!m) break;

        // Found a cyclic dependency
        if (strcmp(cmodule->filePath, m->filePath) == 0) {
            // return exports
            JSObject *gbl = m->exports;
            jsval module;
            JS_GetProperty(cx, gbl, "module", &module);
            JS_GetProperty(cx, JSVAL_TO_OBJECT(module), "exports", &ret);
            return ret;
        }

        m = m->parent;
    }


    if (!cached) {
        // Create all objects/methods on the module scope
        if (!this->modules->init(cmodule)) {
            JS_ReportError(cx, "Failed to initialize module %s\n", cmodule->name);
            return ret;
        }

        if (cmodule->native) {
            ret = OBJECT_TO_JSVAL(cmodule->exports);
        } else {
            if (!NJS->LoadScript(cmodule->filePath, cmodule->exports)) {
                return ret;
            } 

            JS::Value module;
            JS_GetProperty(cx, cmodule->exports, "module", &module);
            JS_GetProperty(cx, module.toObjectOrNull(), "exports", &ret);
        }
    }
 
    return ret;
}

NativeJSModule::~NativeJSModule()
{
    if (this->filePath) {
        this->modules->remove(this);
    }

    free(this->name);
    free(this->absoluteDir);
    free(this->filePath);

}

static JSBool native_modules_require(JSContext *cx, unsigned argc, jsval *vp)
{
    JSString *name = NULL;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "S", &name)) {
        return JS_TRUE;
    }

    JSAutoByteString namestr(cx, name);

    JSObject *callee = (JS_CALLEE(cx, vp)).toObjectOrNull();
    JS::Value reserved = js::GetFunctionNativeReserved(callee, 0);
    if (!reserved.isDouble()) {
        JS_ReportError(cx,  "InternalError");
        return JS_FALSE;
    }

    NativeJSModule *module = static_cast<NativeJSModule *>(JSVAL_TO_PRIVATE(reserved));

    JS::Value ret = module->require(namestr.ptr());

    JS_SET_RVAL(cx, vp, ret);

    if (ret.isNull()) {
        return JS_FALSE;
    }


    return JS_TRUE;
}

void Exports_Finalize(JSFreeOp *fop, JSObject *obj)
{
    void *priv = JS_GetPrivate(obj);
    if (priv != NULL) {
        NativeJSModule *module = static_cast<NativeJSModule *>(priv);
        delete module;
    }
}
