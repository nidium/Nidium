#include <jsfriendapi.h>
#include <jsapi.h>
#include <jsdbgapi.h>
#include <libgen.h>
#include <unistd.h>
#include <errno.h> 

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

char *NativeJSModules::normalizeName(const char *name)
{
    char *ret;
    // test if module name ends with .js extension
    // otherwise add it
    size_t len = strlen(name);
    const char *ext = ".js";
    bool found = true;
    if (len > 3) {
        for (int i = 3; i > -1; i--) {
            if (name[len-i] != ext[3 - i]) {
                found = false;
                break;
            }
        }
    } else {
        found = false;
    }

    if (!found) { 
        ret = (char *)calloc(sizeof(char), len + 4);
        if (!ret) {
            return NULL;
        }

        strcat(ret, name);
        strcat(ret, ".js");
    } else {
        ret = strdup(name);
    }

    return ret;
}

NativeJSModule::NativeJSModule(JSContext *cx, NativeJSModules *main, NativeJSModule *parent, const char *name) 
    : dir(NULL), absoluteDir(NULL), filePath(NULL), fileName(NULL), name(strdup(name)), exports(NULL),
      cx(cx), main(main), parent(parent)
{
}

bool NativeJSModule::init()
{
    if (!this->name || strlen(this->name) == 0) return false;

    this->fileName = this->main->normalizeName(this->name);
    if (!this->fileName) {
        return false;
    }

    if (this->parent) {
        this->filePath = this->parent->findModulePath(this);
    } else {
        this->filePath = strdup(this->fileName);
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
    if (this->fileName[0] != '.') {
        int count = 0;
        for (int i = 0; this->fileName[i]; i++) {
            count += this->fileName[i] == '/';
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
        cmodule = new NativeJSModule(this->cx, this, NULL, name);
        if (!cmodule || !cmodule->init()) {
            return NULL;
        }

        if (!this->init(cmodule)) {
            return NULL;
        }
    }

    return cmodule ? cmodule->exports : scope;
}

bool NativeJSModules::init(NativeJSModule *module)
{
    if (!module->initJS()) {
        return false;
    }

    this->add(module);

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
    // Setup search paths
    char *modulePath = NULL;
    char *paths[PATHS_COUNT];

    memset(paths, 0, sizeof(char*) * PATHS_COUNT);

    // Relative module, only look in current script directory
    if (module->name[0] == '.') {
        paths[0] = this->dir;// FIXME
        //if (!paths[0]) paths[0] = (char *)"."; 
    } else {
        paths[0] = (char *)".";                         // Native root
        paths[1] = this->absoluteDir;                   // Search path of the module
        paths[PATHS_COUNT-2] = (char *)"node_modules";  // Compatibility with NodeJS npm
        paths[PATHS_COUNT-1] = (char *)"modules";       // Root modules directory
    }

    // Check if we can find the module in paths array
    for (int i = 0; i < PATHS_COUNT; i++) {
        if (!paths[i]) continue;

        char *tmp = (char *)calloc(sizeof(char), strlen(paths[i]) + strlen(module->fileName) + 2);
        if (!tmp) {
            JS_ReportOutOfMemory(cx);
            return NULL;
        }
        strcat(tmp, paths[i]);
        strcat(tmp, "/");
        strcat(tmp, module->fileName);
        //printf("    [NativeJSModule] Looking for %s\n", tmp);

        if (access(tmp, F_OK) == 0) {

            //printf("    [NativeJSModule] FOUND IT\n");
            modulePath = tmp;
            break;
        }
        free(tmp);
    }

    if (!modulePath) {
        JS_ReportError(this->cx, "No module named %s", module->name);
        return NULL;
    }

    char *rpath = realpath(modulePath, NULL);

    free(modulePath);

    return rpath;
#undef PATHS_COUNT
}

JS::Value NativeJSModule::require(char *name)
{
    JS::Value ret;
    NativeJSModule *cmodule;

    ret.setNull();

    //printf("[NativeJSModule] Module %s ", this->name);

    NativeJSModule *tmp = new NativeJSModule(cx, this->main, this, name);
    if (!tmp->init()) {
        JS_ReportError(cx, "Module %s not found\n", name);
        delete tmp;
        return ret;
    }

    // First let's see if the module is in the cache
    NativeJSModule *cached = this->main->find(tmp);
    if (!cached) {
        cmodule = tmp;
    } else {
        cmodule = cached;
        delete tmp;
    }

    // Is there is a cyclic dependency
    for (NativeJSModule *m = cmodule->parent;;) {
        if (!m || !m->parent) break;

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
        // Create all objects/methods on the module 
        if (!this->main->init(cmodule)) {
            JS_ReportError(cx, "Failed to initialize module %s\n", cmodule->name);
            return ret;
        }
        if (!NJS->LoadScript(cmodule->filePath, cmodule->exports)) {
            return ret;
        } 
    }

    // And return module.exports
    JS::Value module;
    JS_GetProperty(cx, cmodule->exports, "module", &module);
    JS_GetProperty(cx, module.toObjectOrNull(), "exports", &ret);
    
    return ret;
}

NativeJSModule::~NativeJSModule()
{
    if (this->filePath) {
        this->main->remove(this);
    }

    free(this->name);
    free(this->absoluteDir);
    free(this->fileName);
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
