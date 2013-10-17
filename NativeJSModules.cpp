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

    this->dir = (char *)dirname(strdup(this->filePath));
    if (!this->dir) {
        return false;
    }
    DPRINT("filepath = %s\n", this->filePath);
    DPRINT("dir = %s\n", this->dir);
    DPRINT("name = %s\n", this->name);

    // For absolute module, if the module name contain directories ("/") 
    // we must remove them for the absoluteDir
    if (this->name[0] != '.' && this->name[0] != '/') {
        int count = 0;
        for (int i = 0; this->name[i]; i++) {
            count += this->name[i] == '/';
        }
        DPRINT("count = %d\n", count);

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

    DPRINT("absolute dir for %s\n", this->absoluteDir);

    return true;
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

    NativeJS::getNativeClass(this->cx)->rootObjectUntilShutdown(exports);

    this->exports = exports;

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

    njs->rootObjectUntilShutdown(gbl);
    JS_SetPrivate(gbl, this);

    JSObject *funObj;
    JSFunction *fun = js::DefineFunctionWithReserved(this->cx, gbl, "require", native_modules_require, 1, 0);
    if (!fun) {
        njs->unrootObject(gbl);
        return false;
    }

    funObj = JS_GetFunctionObject(fun);

    js::SetFunctionNativeReserved(funObj, 0, PRIVATE_TO_JSVAL((void *)this));

    JSObject *exports = JS_NewObject(this->cx, NULL, NULL, NULL);
    JSObject *module = JS_NewObject(this->cx, &native_modules_class, NULL, NULL);

    if (!exports || !module) {
        njs->unrootObject(gbl);
        return false;
    }

    JS::Value id;
    JSString *idstr = JS_NewStringCopyN(cx, this->name, strlen(this->name));
    if (!idstr) {
        njs->unrootObject(gbl);
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

    //// XXX : This has nothing to do here
    char *tmp = strdup(this->filePath);
    char *cfilename = basename(tmp);
    JSString *filename = JS_NewStringCopyN(cx, cfilename, strlen(cfilename));
    JSString *dirname = JS_NewStringCopyN(cx, this->dir, strlen(this->dir));

    free(tmp);

    // __filename and __dirname is needed to conform NodeJS require();
    TRY_OR_DIE(JS_DefineProperty(cx, gbl, "__filename", STRING_TO_JSVAL(filename), 
            NULL, NULL, JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE));
    TRY_OR_DIE(JS_DefineProperty(cx, gbl, "__dirname", STRING_TO_JSVAL(dirname), 
            NULL, NULL, JSPROP_PERMANENT | JSPROP_READONLY | JSPROP_ENUMERATE));
    ////

    js::SetFunctionNativeReserved(funObj, 1, exportsVal);

    this->exports = gbl;

    return true;
#undef TRY_OR_DIE
}

char *NativeJSModules::findModulePath(NativeJSModule *parent, NativeJSModule *module)
{
#define MAX_EXT_SIZE 9
#define PATHS_COUNT 2
    char *modulePath = NULL;

    if (module->name[0] == '.') {
        // Relative module, only look in current script directory
        modulePath = NativeJSModules::findModuleInPath(module, parent->dir);
    }  else if (module->name[0] == '/') {
        modulePath = NativeJSModules::findModuleInPath(module, "");
    }  else {
        DPRINT("absolute module\n");
        // TODO : Paths handling will need to be updated for supporting
        // CommonJS "paths" specification
        int end = strlen(parent->absoluteDir);
        char *cwd = realpath(".", NULL);
        char *paths[PATHS_COUNT];
        memset(paths, 0, sizeof(char*) * PATHS_COUNT);

        // Setup search paths
        paths[0] = (char *)"/";                 // Current working directory
        paths[1] = (char *)"/node_modules";    // Root modules directory

        DPRINT("nativeRoot %s\n", cwd);
        // NodeJS compatibility : we need to look for module in all
        // parent directory until current working directory is reached
        char *path = (char *)calloc(sizeof(char), strlen(parent->absoluteDir) + strlen("/node_modules") + 1);
        if (!path) {
            return NULL;
        }
        memcpy(path, parent->absoluteDir, (end + 1) * sizeof(char));

        DPRINT("absolute dir is %s\n", parent->absoluteDir);
        DPRINT("path is %s\n", path);

        bool stop = false;
        do {
            for (int i = 0; i < PATHS_COUNT && !modulePath; i++) {
                path[end] = '\0'; // Reset
                strcat(path, paths[i]);
                DPRINT("path looking %s\n", path);
                modulePath = NativeJSModules::findModuleInPath(module, path);
                DPRINT("module path is %s\n", modulePath);
            }
            // Go to parent directory
            path[end] = '\0';
            stop = (strcmp(cwd, path) == 0);
            if (!stop) {
                DPRINT("asking real path for %s\n", path);
                path = dirname(path);
                end = strlen(path);
                DPRINT("new path is %s\n", path);
                if (!path) break;
            }
        } while (!modulePath && !stop);

        // XXX : temp hack for looking inside modules directory. 
        // Remove me once CommonJS paths argument is supported
        if (!modulePath) {
            path[0] = '\0';
            strcat(path, (char *)"modules");
            modulePath = NativeJSModules::findModuleInPath(module, path);
        }

        free(path);
        free(cwd);
    }

    if (!modulePath) {
        return NULL;
    }

    char *rpath = realpath(modulePath, NULL);

    free(modulePath);

    return rpath;
#undef PATHS_COUNT
}

char *NativeJSModules::findModuleInPath(NativeJSModule *module, const char *path) 
{
    // FIXME : NodeJS compatibility, instead of looking for index.js 
    // we should look for package.json, read it and find the main package file
    #define _MTOSTR(s) #s
    #define MTOSTR(s) _MTOSTR(s)
    const char *extensions[] = { MTOSTR(DSO_EXTENSION), ".js", "/index.js", NULL};
    #undef MTOSTR
    #undef _MTROSTR

    size_t len = strlen(module->name);
    char *tmp = (char *)calloc(sizeof(char), strlen(path) + len + (len > MAX_EXT_SIZE  ? len + 1 : MAX_EXT_SIZE) + 2);
    if (!tmp) {
        return NULL;
    }

    DPRINT("path : %s\n", path);
    DPRINT("module->name: %s\n", module->name);
    strcat(tmp, path);
    strcat(tmp, "/");
    strcat(tmp, module->name);

    size_t end = strlen(tmp);
    DPRINT("tmp is %s\n", tmp);

    for (int j = 0; j < 4; j++) {
        if (extensions[j]) {
            const char *c = &extensions[j][0];
            for (int k = 0; k < MAX_EXT_SIZE && *c != '\0'; k++) {
                tmp[end + k] = *c++;
            }
        } else {
            tmp[end] = '\0';
        }

        DPRINT("    [NativeJSModule] Looking for %s\n", tmp);

        if (access(tmp, F_OK) == 0) {
            struct stat sb;
            if (stat(tmp, &sb) == 0 && S_ISDIR(sb.st_mode)) {
                // Extra check for NodeJS compatibility : 
                // If the module name is a directory, let's check if the module name
                // exist in that directory
                DPRINT("it's a directory, extra check : ");
                strcat(tmp, "/");
                strcat(tmp, module->name);
                DPRINT("%s\n", tmp);
                if (access(tmp, F_OK) != 0) {
                    continue;
                }
            }
            DPRINT("    [NativeJSModule] FOUND IT\n");
            if (j == 0) {
                module->native = true;
            }
            return tmp;
        }
    }

    free(tmp);
    return NULL;
}

#undef MAX_EXT_SIZE 


JS::Value NativeJSModule::require(char *name)
{
    JS::Value ret;
    NativeJSModule *cmodule;

    ret.setNull();

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
        this->absoluteDir = dirname(strdup(this->filePath));
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
        cmodule = tmp;
    } else {
        DPRINT("Module is cached\n");
        cmodule = cached;
        delete tmp;
    }

    // Is there is a cyclic dependency
    for (NativeJSModule *m = cmodule->parent;;) {
        if (!m) break;

        // Found a cyclic dependency
        if (strcmp(cmodule->filePath, m->filePath) == 0) {
            DPRINT("Cyclic dependency found\n");
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

        if (!cmodule->native) {
            FILE *fd;
            size_t readsize;
            size_t filesize;
            char *data;

            JSFunction *fn;
            jsval *rval;

            // TODO : Replace fopen/fseek/fread with future NativeSyncFileIO
            fd = fopen(cmodule->filePath, "r");
            if (!fd) {
                JS_ReportError(cx, "Failed to open module %s\n", cmodule->name);
                return ret;
            }

            fseek(fd, 0L, SEEK_END);
            filesize = ftell(fd);
            fseek(fd, 0L, SEEK_SET);

            data = (char *)malloc(filesize + 1);

            readsize = fread(data, sizeof(char), filesize < 4096 ? filesize : 40986, fd);

            fclose(fd);

            if (readsize < 1) {
                JS_ReportError(cx, "Failde to read module %s\n", cmodule->name);
                free(data);
                return ret;
            }

            fn = JS_CompileFunction(cx, cmodule->exports, 
                    "", 0, NULL, data, strlen(data), cmodule->filePath, 0);
            if (!fn) {
                free(data);
                return ret;
            }

            free(data);

            if (!JS_CallFunction(cx, cmodule->exports, fn, 0, NULL, rval)) {
                return ret;
            }
        }
    } 

    if (cmodule->native) {
        ret = OBJECT_TO_JSVAL(cmodule->exports);
    } else {
        JS::Value module;
        JS_GetProperty(cx, cmodule->exports, "module", &module);
        JS_GetProperty(cx, module.toObjectOrNull(), "exports", &ret);
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
        JS_ReportError(cx, "InternalError");
        return JS_FALSE;
    }

    NativeJSModule *module = static_cast<NativeJSModule *>(JSVAL_TO_PRIVATE(reserved));

    JS::Value ret = module->require(namestr.ptr());

    JS_SET_RVAL(cx, vp, ret);

    if (ret.isNull()) {
        return JS_FALSE;
    }

    JS::Value foo;
    JS_GetProperty(cx, ret.toObjectOrNull(), "foo", &foo);
    //printf("foo is %s\n", foo.toString());


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
