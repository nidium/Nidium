/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "Binding/JSModules.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <dlfcn.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <algorithm>

#include <json/json.h>
#include <jsfriendapi.h>

#include "Binding/NidiumJS.h"
#include "IO/Stream.h"

#define NIDIUM_MODULES_PATHS_COUNT 2

using Nidium::IO::Stream;
using Nidium::Core::Path;

namespace Nidium {
namespace Binding {

typedef bool (*register_module_t)(JSContext *cx, JS::HandleObject exports);
// {{{ Preamble
#if 0
#define DPRINT(...) printf(__VA_ARGS__)
#else
#define DPRINT(...) (void)0
#endif

static void Exports_Finalize(JSFreeOp *fop, JSObject *obj);
Core::Hash<void *> JSModules::m_EmbeddedModules;

static JSClass nidium_modules_exports_class = { "Exports",
                                                JSCLASS_HAS_PRIVATE,
                                                nullptr,
                                                nullptr,
                                                nullptr,
                                                nullptr,
                                                nullptr,
                                                nullptr,
                                                nullptr,
                                                Exports_Finalize};

static JSClass nidium_modules_class = { "Module",
                                        0 };

#if 0
static JSPropertySpec nidium_modules_exports_props[] = {
    {"exports", 0, JSOP_NULLWRAPPER, JSOP_NULLWRAPPER},
    {"module", 0, JSOP_NULLWRAPPER, JSOP_NULLWRAPPER},
    JS_PS_END
};
#endif

static bool nidium_modules_require(JSContext *cx, unsigned argc, JS::Value *vp);
// }}}

// {{{ JSModule
JSModule::JSModule(JSContext *cx,
                   JSModules *modules,
                   JSModule *parent,
                   const char *name)
    : m_AbsoluteDir(NULL), m_FilePath(NULL), m_Name(strdup(name)),
      m_ModuleType(JSModule::kModuleType_None), m_Cached(false),
      m_Exports(NULL), m_Parent(parent), m_Modules(modules), m_Cx(cx)
{
}

bool JSModule::initMain()
{
    m_Name = strdup("__MAIN__");
    JS::RootedObject global(m_Cx, JS::CurrentGlobalOrNull(m_Cx));

    JS::RootedFunction fun(
        m_Cx, js::DefineFunctionWithReserved(m_Cx, global, "require",
                                             nidium_modules_require, 1, 0));

    if (!fun) {
        return false;
    }

    JS::RootedObject funObj(m_Cx, JS_GetFunctionObject(fun));

    js::SetFunctionNativeReserved(funObj, 0,
                                  JS::PrivateValue(static_cast<void *>(this)));

    m_Exports = NULL; // Main module is not a real module, thus no exports

    return true;
}

bool JSModule::init()
{
    if (!m_Name || strlen(m_Name) == 0) return false;
    DPRINT("= JSModule INIT\n");
    DPRINT("name = %s\n", m_Name);

    if (m_Parent) {
        m_FilePath = JSModules::FindModulePath(m_Parent, this);
    } else {
        m_FilePath = realpath(m_Name, NULL);
    }

    if (!m_FilePath) {
        // Module not found
        return false;
    }

    if (m_ModuleType != kModuleType_NativeEmbedded) {
        Path p(m_FilePath, false, true);

        if (!p.dir()) {
            return false;
        }

        m_AbsoluteDir = strdup(p.dir());
    }

    DPRINT("filepath = %s\n", m_FilePath);
    DPRINT("name = %s\n", m_Name);

    DPRINT("absolute dir for %s\n", m_AbsoluteDir);

    return true;
}

bool JSModule::initNative()
{
    JS::RootedObject exports(m_Cx, JS_NewObject(m_Cx, nullptr));

    if (!exports) {
        return false;
    }

    void *module = dlopen(m_FilePath, RTLD_LAZY);
    if (!module) {
        printf("Failed to open module : %s\n", dlerror());
        return false;
    }

    register_module_t registerModule = reinterpret_cast<register_module_t>(
        dlsym(module, "__NidiumRegisterModule"));
    if (registerModule && !registerModule(m_Cx, exports)) {
        printf("Failed to register module\n");
        return false;
    }

    m_Exports = exports;
    NidiumJS::RootObjectUntilShutdown(m_Exports);

    return true;
}

bool JSModule::initNativeEmbedded()
{
    JSModules::EmbeddedCallback registerCallback
        = JSModules::FindEmbedded(m_Name);

    if (!registerCallback) return false;

    JS::RootedObject obj(m_Cx, registerCallback(m_Cx));
    if (!obj) return false;

    m_Exports = obj;
    NidiumJS::RootObjectUntilShutdown(m_Exports);

    return true;
}

bool JSModule::initJS()
{
#define TRY_OR_DIE(call) \
    if (call == false) { \
        return false;    \
    }

    JS::RootedObject gbl(m_Cx,
        JS_NewObject(m_Cx, &nidium_modules_exports_class));

    if (!gbl) {
        return false;
    }

    JS_SetPrivate(gbl, this);

    JS::RootedObject funObj(m_Cx);
    JSFunction *fun = js::DefineFunctionWithReserved(
        m_Cx, gbl, "require", nidium_modules_require, 1, 0);
    if (!fun) {
        return false;
    }

    funObj = JS_GetFunctionObject(fun);

    js::SetFunctionNativeReserved(funObj, 0,
                                  JS::PrivateValue(static_cast<void *>(this)));

    JS::RootedObject exports(
        m_Cx, JS_NewObject(m_Cx, nullptr));
    JS::RootedObject module(m_Cx, JS_NewObject(m_Cx, &nidium_modules_class));

    if (!exports || !module) {
        return false;
    }

    JS::RootedValue id(m_Cx);
    JS::RootedString idstr(m_Cx,
                           JS_NewStringCopyN(m_Cx, m_Name, strlen(m_Name)));
    if (!idstr) {
        return false;
    }

    id.setString(idstr);

    JS::RootedValue exportsVal(m_Cx, JS::ObjectValue(*exports));
    JS::RootedValue moduleVal(m_Cx, JS::ObjectValue(*module));
#if 0
    TRY_OR_DIE(JS_DefineProperties(m_Cx, gbl, nidium_modules_exports_props));
#endif
    TRY_OR_DIE(JS_SetProperty(m_Cx, gbl, "exports", exportsVal));

#if 0
    //TRY_OR_DIE(JS_DefineProperties(m_Cx, module, nidium_modules_exports_props));
#endif
    TRY_OR_DIE(JS_SetProperty(m_Cx, gbl, "module", moduleVal));
    TRY_OR_DIE(JS_SetProperty(m_Cx, module, "id", id));
    TRY_OR_DIE(JS_SetProperty(m_Cx, module, "exports", exportsVal));

    js::SetFunctionNativeReserved(funObj, 1, exportsVal);

    m_Exports = gbl;
    NidiumJS::RootObjectUntilShutdown(m_Exports);

    return true;
#undef TRY_OR_DIE
}

JS::Value JSModule::require(char *name)
{
    JS::RootedValue ret(m_Cx);
    JSModule *cmodule;

    ret.setUndefined();

    // require() have been called from the main module
    if (this == m_Modules->m_Main) {
        /*
         * This little hack is needed to conform CommonJS :
         *  - Cyclic deps
         *  - Finding module
         *
         * Since all files included with NidiumJS::LoadScript();
         * share the same module we need to be aware of the real caller.
         * So here we set the filename and path of the caller
         *
         * XXX : Another way to handle this case would be to make
         * load() aware of his context by using the same trick
         * require do.
         */
        unsigned lineno;
        JS::AutoFilename filename;
        JS::DescribeScriptedCaller(m_Cx, &filename, &lineno);

        free(m_FilePath);
        free(m_AbsoluteDir);
        // filePath is needed for cyclic deps check
        m_FilePath = realpath(filename.get(), NULL);

        if (m_FilePath == NULL) {
            m_AbsoluteDir = strdup(Path::GetCwd());
        } else {
            // absoluteDir is needed for FindModulePath
            Path p(m_FilePath, false, true);
            m_AbsoluteDir = strdup(p.dir());
            DPRINT("Global scope loading\n");
        }
    } else {
        DPRINT("Module scope loading\n");
    }

    DPRINT("[JSModule] Module %s require(%s)\n", m_Name, name);

    JSModule *tmp = new JSModule(m_Cx, m_Modules, this, name);
    if (!tmp->init()) {
        JS_ReportError(m_Cx, "Module %s not found\n", name);
        delete tmp;
        return ret;
    }

    // Let's see if the module is in the cache
    JSModule *cached = m_Modules->find(tmp);
    if (!cached) {
        DPRINT("Module is not cached\n");
        cmodule = tmp;
    } else {
        DPRINT("Module is cached %s\n", cached->m_FilePath);
        cmodule = cached;
        delete tmp;
    }

    // Is there is a cyclic dependency
    for (JSModule *m = cmodule->m_Parent;;) {
        if (!m || !m->m_Exports) break;

        // Found a cyclic dependency
        if (strcmp(cmodule->m_FilePath, m->m_FilePath) == 0) {
            JS::RootedObject gbl(m_Cx, m->m_Exports);
            JS::RootedValue module(m_Cx);
            JS_GetProperty(m_Cx, gbl, "module", &module);

            JS::RootedObject modObj(m_Cx, &module.toObject());
            JS_GetProperty(m_Cx, modObj, "exports", &ret);
            return ret;
        }

        m = m->m_Parent;
    }

    if (!cached) {
        if (!m_Modules->init(cmodule)) {
            JS_ReportError(m_Cx, "Failed to initialize module %s\n",
                           cmodule->m_Name);
            return ret;
        }

        if (cmodule->m_ModuleType == JSModule::kModuleType_JSON
            || cmodule->m_ModuleType == JSModule::kModuleType_JS) {
            size_t filesize;
            char *data;

            JS::RootedFunction fn(m_Cx);
            JS::RootedValue rval(m_Cx);

            if (!JSModules::GetFileContent(cmodule->m_FilePath, &data,
                                           &filesize)
                || data == NULL) {
                JS_ReportError(m_Cx, "Failed to open module %s\n",
                               cmodule->m_Name);
                return ret;
            }

            Core::PtrAutoDelete<> npad(data, free);

            if (filesize == 0) {
                ret.setObject(*cmodule->m_Exports);
                return ret;
            }

            if (cmodule->m_ModuleType == JSModule::kModuleType_JS) {
                JS::RootedObject expObj(m_Cx, cmodule->m_Exports);
                JS::CompileOptions options(m_Cx);
                options.setFileAndLine(cmodule->m_FilePath, 1).setUTF8(true);

                fn = JS::CompileFunction(m_Cx, expObj, options, NULL, 0, NULL,
                                         data, strlen(data));

                if (!fn) {
                    return ret;
                }

                if (!JS_CallFunction(m_Cx, expObj, fn,
                                     JS::HandleValueArray::empty(), &rval)) {
                    return ret;
                }
            } else {
                size_t len;
                uint16_t *jchars;
                JS::RootedValue jsonData(m_Cx);

                if (!JS_DecodeBytes(m_Cx, data, filesize, NULL, &len)) {
                    return ret;
                }

                jchars = static_cast<uint16_t *>(
                    JS_malloc(m_Cx, len * sizeof(uint16_t)));
                if (!jchars) {
                    return ret;
                }

                if (!JS_DecodeBytes(m_Cx, data, filesize, jchars, &len)) {
                    JS_free(m_Cx, jchars);
                    return ret;
                }

                if (!JS_ParseJSON(m_Cx, jchars, len, &jsonData)) {
                    JS_free(m_Cx, jchars);
                    return ret;
                }

                JS_free(m_Cx, jchars);

                cmodule->m_Exports.set(jsonData.toObjectOrNull());
                NidiumJS::RootObjectUntilShutdown(cmodule->m_Exports);
            }
        }
    }

    switch (cmodule->m_ModuleType) {
        case JSModule::kModuleType_JS: {
            JS::RootedValue module(m_Cx);
            JS::RootedObject expObj(m_Cx, cmodule->m_Exports);
            JS_GetProperty(m_Cx, expObj, "module", &module);
            JS::RootedObject modObj(m_Cx, &module.toObject());
            JS_GetProperty(m_Cx, modObj, "exports", &ret);
        } break;
        case JSModule::kModuleType_JSON:
        case JSModule::kModuleType_Native:
        case JSModule::kModuleType_NativeEmbedded: {
            ret = JS::ObjectValue(*cmodule->m_Exports);
        } break;
    }

    return ret;
}

JSModule::~JSModule()
{
    if (m_FilePath && m_Cached) {
        m_Modules->remove(this);
    }

    free(m_Name);
    free(m_AbsoluteDir);
    free(m_FilePath);
}
// }}}

// {{{ JSModules
bool JSModules::init()
{
    char *paths = getenv("NIDIUM_REQUIRE_PATH");

    if (paths) {
        char *token;
        char *originalPaths = strdup(paths);
        char *tmp           = originalPaths;
        int i               = 0;

        while ((token = strsep(&tmp, ":")) != NULL && i < 63) {
            if (i > 63) {
            } else {
                m_EnvPaths[i] = strdup(token);
                i++;
            }
        }

        if (token != NULL && i == 63) {
            fprintf(stderr,
                    "Warning : require path ignored %s."
                    " A maximum of 63 search path is allowed. All subsequent "
                    "paths will be ignored.\n",
                    token);
        }

        m_EnvPaths[i] = NULL;

        free(originalPaths);
    } else {
        m_EnvPaths[0] = NULL;
    }

    m_Main = new JSModule(m_Cx, this, NULL, "MAIN");

    if (!m_Main) return false;

    return m_Main->initMain();
}

bool JSModules::init(JSModule *module)
{
    switch (module->m_ModuleType) {
        case JSModule::kModuleType_NativeEmbedded:
            if (!module->initNativeEmbedded()) {
                return false;
            }
            break;
        case JSModule::kModuleType_Native:
            if (!module->initNative()) {
                return false;
            }
            break;
        case JSModule::kModuleType_JS:
            if (!module->initJS()) {
                return false;
            }
            break;
    }

    this->add(module);

    return true;
}

void JSModules::RegisterEmbedded(const char *name,
                                 EmbeddedCallback registerCallback)
{
    JSModules::m_EmbeddedModules.set(
        name, reinterpret_cast<void *>(registerCallback));
}

JSModules::EmbeddedCallback JSModules::FindEmbedded(const char *name)
{
    return reinterpret_cast<EmbeddedCallback>(m_EmbeddedModules.get(name));
}

void JSModules::DirName(std::string &source)
{
    if (source.size() <= 1) {
        return;
    }

    // Remove trailing slash if it exists.
    if (*(source.rbegin()) == '/') {
        source.resize(source.size() - 1);
    }

    source.erase(std::find(source.rbegin(), source.rend(), '/').base(),
                 source.end());
}

char *JSModules::FindModulePath(JSModule *parent, JSModule *module)
{
    std::string modulePath;
    JSModules *modules = module->m_Modules;
    const char *topDir = Path::GetCwd();

    if (module->m_Name[0] == '.') {
        // Relative module, only look in current script directory
        modulePath = JSModules::FindModuleInPath(module, parent->m_AbsoluteDir);
    } else if (module->m_Name[0] == '/') {
        modulePath = JSModules::FindModuleInPath(module, topDir);
    } else {
        std::string path = parent->m_AbsoluteDir;

        DPRINT("[FindModulePath] absolute topDir=%s dir=%s path=%s\n", topDir,
               parent->m_AbsoluteDir, path.c_str());

        // Look for the module in all parent directory until it's found
        // or if the top level working directory is reached
        bool stop = false;
        do {
            // Try local search path
            for (int i = 0;
                 i < NIDIUM_MODULES_PATHS_COUNT && modulePath.empty(); i++) {
                std::string currentPath = path;
                currentPath += modules->m_Paths[i];

                DPRINT("Looking for module %s in %s\n", module->m_Name,
                       currentPath.c_str());
                modulePath
                    = JSModules::FindModuleInPath(module, currentPath.c_str());
                DPRINT("module path is %s\n", modulePath.c_str());
            }

            stop = (strcmp(topDir, path.c_str()) >= 0);
            // Try again with parent directory
            if (!stop) {
                DPRINT("  Getting parent dir for %s\n", path.c_str());
                JSModules::DirName(path);
                DPRINT("  Parent path is         %s\n", path.c_str());
            }
        } while (modulePath.empty() && !stop);

        // Check in system directories if module hasn't been found
        if (modulePath.empty()) {
            for (int i = 0;
                 modules->m_EnvPaths[i] != NULL && modulePath.empty(); i++) {
                char *tmp = modules->m_EnvPaths[i];
                DPRINT("Looking for module %s in %s\n", module->m_Name, tmp);
                modulePath = JSModules::FindModuleInPath(module, tmp);
                DPRINT("module path is %s\n", modulePath.c_str());
            }
        }

        // Check if module is not an embedded module
        if (m_EmbeddedModules.get(module->m_Name) != nullptr) {
            char *ret = nullptr;
            if (asprintf(&ret, "embedded_module://%s", module->m_Name) != -1) {
                module->m_ModuleType = JSModule::kModuleType_NativeEmbedded;
                return ret;
            } else {
                return nullptr;
            }
        }
    }

    if (modulePath.empty()) {
        return NULL;
    }

    return realpath(modulePath.c_str(), NULL);
}

bool JSModules::GetFileContent(const char *file, char **content, size_t *size)
{
    Path path(file, false, true);
    Stream *stream = path.CreateStream(true);

    if (!stream) {
        return false;
    }

    bool ret = stream->getContentSync(content, size);

    delete stream;

    return ret;
}

std::string JSModules::FindModuleInPath(JSModule *module, const char *path)
{
#define MAX_EXT_SIZE 13
    const char *extensions[] = { NULL, ".js", DSO_EXTENSION, ".json" };

    std::string tmp
        = std::string(path) + std::string("/") + std::string(module->m_Name);
    size_t len = tmp.length();

    DPRINT("    tmp is %s\n", tmp.c_str());

    for (int i = 0; i < 4; i++) {
        if (extensions[i]) {
            tmp.erase(len);
            tmp += extensions[i];
        }

        DPRINT("    [JSModule] Looking for %s\n", tmp.c_str());

        if (access(tmp.c_str(), F_OK) != 0) {
            continue;
        }

        // XXX : Refactor this code. It's a bit messy.
        switch (i) {
            case 0: // directory or exact filename
                module->m_ModuleType = JSModule::kModuleType_JS;
                struct stat sb;
                if (stat(tmp.c_str(), &sb) == 0 && S_ISDIR(sb.st_mode)) {
                    if (JSModules::LoadDirectoryModule(tmp)) {
                        return tmp;
                    }
                    DPRINT("%s\n", tmp.c_str());
                } else {
                    // Exact filename, find the extension
                    size_t pos = tmp.find_last_of('.');
                    // No extension, assume it's a JS file
                    if (pos == std::string::npos) {
                        DPRINT("      No extension found. Assuming JS file\n");
                        module->m_ModuleType = JSModule::kModuleType_JS;
                        return tmp;
                    }

                    for (int j = 1; j < 4; j++) {
                        size_t tmpPos
                            = tmp.find(extensions[j], pos, len - 1 - pos);
                        if (tmpPos != std::string::npos) {
                            switch (j) {
                                case 1:
                                    module->m_ModuleType
                                        = JSModule::kModuleType_JS;
                                    break;
                                case 2:
                                    module->m_ModuleType
                                        = JSModule::kModuleType_Native;
                                    break;
                                case 3:
                                    module->m_ModuleType
                                        = JSModule::kModuleType_JSON;
                                    break;
                            }
                            return tmp;
                        }
                    }
                }
                continue;
            case 1: // .js
                module->m_ModuleType = JSModule::kModuleType_JS;
                break;
            case 2: // native module
                module->m_ModuleType = JSModule::kModuleType_Native;
                break;
            case 3: // json file
                module->m_ModuleType = JSModule::kModuleType_JSON;
                break;
        }
        DPRINT("    [JSModule] FOUND IT\n");
        return tmp;
    }

    return std::string();
}

bool JSModules::LoadDirectoryModule(std::string &dir)
{
    const char *files[] = { "/index.js", "/package.json" };
    size_t len          = dir.length();

    for (int i = 0; i < 2; i++) {
        dir.erase(len);
        dir += files[i];

        if (access(dir.c_str(), F_OK) != 0) continue;

        switch (i) {
            case 0: // index.js
                return true;
                break;
            case 1: // package.json
                char *data = NULL;
                size_t size;

                if (!JSModules::GetFileContent(dir.c_str(), &data, &size)
                    || data == NULL) {
                    fprintf(stderr, "Failed to open %s\n", dir.c_str());
                    return false;
                }

                Core::PtrAutoDelete<> npad(data, free);

                Json::Value root;
                Json::Reader reader;
                bool parsingSuccessful = reader.parse(data, data + size, root);

                if (!parsingSuccessful) {
                    fprintf(stderr, "Failed to parse %s\n  %s\n", dir.c_str(),
                            reader.getFormatedErrorMessages().c_str());
                    return false;
                }

                std::string main = root.get("main", "").asString();
                dir.erase(len);
                dir += std::string("/") + main;

                if (access(dir.c_str(), F_OK) != 0) {
                    fprintf(stderr, "Failed to access file %s\n", dir.c_str());
                    return false;
                }

                return true;
                break;
        }
    }

    return false;
}

#undef MAX_EXT_SIZE
// }}}

// {{{ Implementation
static bool nidium_modules_require(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::RootedString name(cx, NULL);

    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    if (!JS_ConvertArguments(cx, args, "S", name.address())) {
        return false;
    }

    JSAutoByteString namestr(cx, name);

    JSObject *callee = &args.callee();
    JS::RootedValue reserved(cx, js::GetFunctionNativeReserved(callee, 0));

    if (!reserved.isDouble()) {
        JS_ReportError(cx, "InternalError");
        return false;
    }

    JSModule *module = static_cast<JSModule *>(JSVAL_TO_PRIVATE(reserved));

    JS::RootedValue ret(cx, module->require(namestr.ptr()));

    args.rval().set(ret);

    if (ret.isUndefined()) {
        return false;
    }

    return true;
}

void Exports_Finalize(JSFreeOp *fop, JSObject *obj)
{
    void *priv = JS_GetPrivate(obj);
    if (priv != NULL) {
        JSModule *module = static_cast<JSModule *>(priv);
        delete module;
    }
}
// }}}

} // namespace Binding
} // namespace Nidium
