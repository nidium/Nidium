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
#include <sys/types.h>

#include <algorithm>

#include <json/json.h>
#include <jsfriendapi.h>

#include "Frontend/NML.h"
#include "Binding/NidiumJS.h"
#include "Binding/JSUtils.h"
#include "Binding/ThreadLocalContext.h"
#include "IO/Stream.h"

#define NIDIUM_MODULES_PATHS_COUNT 2

using Nidium::IO::Stream;
using Nidium::Core::Path;
using Nidium::Core::PtrAutoDelete;

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
    : m_AbsoluteDir(nullptr), m_FilePath(nullptr), m_Name(strdup(name)),
      m_ModuleType(JSModule::kModuleType_None), m_Cached(false),
      m_Exports(nullptr), m_Parent(parent), m_Modules(modules), m_Cx(cx)
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
    if (!m_Name || !m_Parent || strlen(m_Name) == 0) return false;
    DPRINT("JSModule init name = %s\n", m_Name);

    if (!this->findModulePath()) {
        // Module not found
        return false;
    }

    DPRINT("filepath = %s\n", m_FilePath->path());
    DPRINT("name = %s\n", m_Name);

    DPRINT("absolute dir for %s\n", m_AbsoluteDir);

    return true;
}

bool JSModule::initNative()
{
    JS::RootedObject exports(m_Cx, JS_NewPlainObject(m_Cx));

    if (!exports) {
        return false;
    }

    void *module = dlopen(m_FilePath->path(), RTLD_LAZY);
    if (!module) {
        NLOG("Failed to open module : %s\n", dlerror());
        return false;
    }

    register_module_t registerModule = reinterpret_cast<register_module_t>(
        dlsym(module, "__NidiumRegisterModule"));
    if (registerModule && !registerModule(m_Cx, exports)) {
        NLOG("Failed to register module\n");
        return false;
    }

    m_Exports = exports;
    NidiumLocalContext::RootObjectUntilShutdown(m_Exports);

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
    NidiumLocalContext::RootObjectUntilShutdown(m_Exports);

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
        m_Cx, JS_NewPlainObject(m_Cx));
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
    NidiumLocalContext::RootObjectUntilShutdown(m_Exports);

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
        // The main module is attached to all the script loaded without
        // require(), so here we need to find out the real path & name of
        // the caller to know where to look for the module
        unsigned lineno;
        JS::AutoFilename filename;
        JS::DescribeScriptedCaller(m_Cx, &filename, &lineno);

        delete m_FilePath;
        m_FilePath = nullptr;
        free(m_AbsoluteDir);

        if (filename.get()) {
            m_FilePath = new Path(filename.get(), true);
            m_AbsoluteDir = strdup(m_FilePath->dir());
        } else {
            m_AbsoluteDir = strdup(Path::GetCwd());
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
        DPRINT("Module is cached %s\n", cached->m_FilePath->path());
        cmodule = cached;
        delete tmp;
    }

    // Is there is a cyclic dependency
    for (JSModule *m = cmodule->m_Parent;;) {
        if (!m || !m->m_Exports) break;

        // Found a cyclic dependency
        if (strcmp(cmodule->m_FilePath->path(), m->m_FilePath->path()) == 0) {
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
            || cmodule->m_ModuleType == JSModule::kModuleType_JS
            || cmodule->m_ModuleType == JSModule::kModuleType_NidiumComponent) {
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

            if (cmodule->m_ModuleType == JSModule::kModuleType_NidiumComponent) {
                JS::RootedValue loader(m_Cx);
                JS::RootedValue requireVal(m_Cx);
                JS::RootedObject require(m_Cx);

                JS::RootedObject gbl(m_Cx, JS::CurrentGlobalOrNull(m_Cx));
                JS::RootedObject expObj(m_Cx, cmodule->m_Exports);

                JS_GetProperty(m_Cx, gbl, "require", &requireVal);

                if (!requireVal.isObject()) {
                    JS_ReportError(m_Cx, "Invalid environement, require() not available");
                    return ret;
                }

                require.set(requireVal.toObjectOrNull());

                if (JS_GetProperty(m_Cx, require, "ComponentLoader", &loader)
                    && loader.isObject()
                    && JS::IsCallable(&loader.toObject())) {

                    JS::RootedObject lst(m_Cx);
                    JS::RootedValue filename(m_Cx);
                    JS::RootedValue rval(m_Cx);
                    JS::AutoValueArray<2> args(m_Cx);

                    JSUtils::StrToJsval(m_Cx, m_FilePath->path(), strlen(cmodule->m_FilePath->path()), &filename, "utf-8");

                    lst.set(Frontend::NML::BuildLST(m_Cx, data));
                    if (!lst) {
                        JS_ReportError(m_Cx, "Failed to parse NML");
                        return ret;
                    }

                    args[0].set(filename);
                    args[1].setObjectOrNull(lst);

                    if (!JS_CallFunctionValue(m_Cx, expObj, loader, args, &rval)) {
                        return ret;
                    }

                    ret.set(rval);

                    return ret;
                } else {
                    JS_ReportError(m_Cx, "No ComponentLoader defined");
                    return ret;
                }
            } else if (cmodule->m_ModuleType == JSModule::kModuleType_JS) {
                JS::RootedObject expObj(m_Cx, cmodule->m_Exports);
                JS::CompileOptions options(m_Cx);
                options.setFileAndLine(cmodule->m_FilePath->path(), 1).setUTF8(true);

                JS::AutoObjectVector scopeChain(m_Cx);
                scopeChain.append(expObj);

                if (!JS::CompileFunction(m_Cx, scopeChain, options, NULL, 0, NULL,
                                         data, strlen(data), &fn)) {
                    return ret;
                }

                if (!JS_CallFunction(m_Cx, expObj, fn,
                                     JS::HandleValueArray::empty(), &rval)) {
                    return ret;
                }
            } else {
                size_t len;
                char16_t *jchars;
                JS::RootedValue jsonData(m_Cx);

                if (!JS_DecodeBytes(m_Cx, data, filesize, NULL, &len)) {
                    return ret;
                }

                jchars = static_cast<char16_t *>(
                    JS_malloc(m_Cx, len * sizeof(char16_t)));
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

                cmodule->m_Exports = jsonData.toObjectOrNull();
                NidiumLocalContext::RootObjectUntilShutdown(cmodule->m_Exports);
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
    delete m_FilePath;
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
            m_EnvPaths[i] = strdup(token);
            i++;
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

bool JSModule::findModulePath()
{
    std::string modulePath;
    JSModules *modules = m_Modules;
    const char *topDir = Path::GetCwd();

    if (m_Name[0] == '.') {
        // Relative module, only look in current script directory
        modulePath = JSModules::FindModuleInPath(this, m_Parent->m_AbsoluteDir);
    } else if (m_Name[0] == '/') {
        modulePath = JSModules::FindModuleInPath(this, topDir);
    } else {
        std::string path = m_Parent->m_AbsoluteDir;

        DPRINT("[FindModulePath] absolute topDir=%s dir=%s path=%s\n", topDir,
               m_Parent->m_AbsoluteDir, path.c_str());

        // Check if the module is not a JS embedded module
        modulePath = JSModules::FindModuleInPath(this, "embed://lib/");

        // Check if module is not a native embedded module
        if (modulePath.empty() &&
                m_Modules->m_EmbeddedModules.get(m_Name) != nullptr) {
            m_ModuleType = JSModule::kModuleType_NativeEmbedded;
            modulePath = m_Name;
        }

        // Look for the module in all parent directory until it's found
        // or if the top level working directory is reached
        bool stop = false;
        do {
            // Try local search path
            for (int i = 0;
                 i < NIDIUM_MODULES_PATHS_COUNT && modulePath.empty(); i++) {
                std::string currentPath = path;
                currentPath += modules->m_Paths[i];

                DPRINT("Looking for module %s in %s\n", m_Name,
                       currentPath.c_str());
                modulePath
                    = JSModules::FindModuleInPath(this, currentPath.c_str());
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
                DPRINT("Looking for module %s in %s\n", m_Name, tmp);
                modulePath = JSModules::FindModuleInPath(this, tmp);
                DPRINT("module path is %s\n", modulePath.c_str());
            }
        }
    }

    if (modulePath.empty()) {
        return false;
    }

    Path *p = new Path(modulePath.c_str(), true);
    Path::schemeInfo *scheme = p->GetScheme();

    if (!scheme->AllowSyncStream || !scheme->AllowLocalFileStream) {
        return false;
    } else if (scheme->GetBaseDir() != nullptr &&
            m_ModuleType != JSModule::kModuleType_NativeEmbedded) {
        // File is a real file on disk (not virtualized by NFS for instance)
        // Run realpath on the file to get the resolved path
        // (this is needed for handling cyclic deps checking and caching)
        char *tmp = realpath(p->path(), NULL);
        if (tmp) {
            m_FilePath = new Path(tmp, true);
        }

        free(tmp);
        delete p;

        if (m_FilePath == nullptr) {
            return false;
        }
    } else {
        m_FilePath = p;
    }

    m_AbsoluteDir = strdup(m_FilePath->dir());

    return true;
}

void JSModules::add(JSModule *module)
{
    m_Cache.set(module->m_FilePath->path(), module);
    module->m_Cached = true;
}

void JSModules::remove(JSModule *module)
{
    m_Cache.erase(module->m_FilePath->path());
}

JSModule *JSModules::find(JSModule *module)
{
    return m_Cache.get(module->m_FilePath->path());
}

bool JSModules::GetFileContent(Path *p, char **content, size_t *size)
{
    Stream *stream = p->CreateStream(true);
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
    const char *extensions[] = { NULL, ".js", DSO_EXTENSION, ".json", ".nc" };

    std::string tmp
        = std::string(path) + std::string("/") + std::string(module->m_Name);
    size_t len = tmp.length();

    DPRINT("    tmp is %s\n", tmp.c_str());

    for (int i = 0; i < 5; i++) {
        if (extensions[i]) {
            tmp.erase(len);
            tmp += extensions[i];
        }

        DPRINT("    [JSModule] Looking for %s\n", tmp.c_str());

        Path p(tmp.c_str(), true /* allowAll */);
        PtrAutoDelete<Stream *> stream(Stream::Create(p.path()));

        if (!stream.ptr() || !stream.ptr()->exists()) {
            continue;
        }


        switch (i) {
            case 0: // directory or exact filename
                module->m_ModuleType = JSModule::kModuleType_JS;
                if (stream.ptr()->isDir()) {
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

                    for (int j = 1; j < 5; j++) {
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
                                case 4:
                                    module->m_ModuleType
                                        = JSModule::kModuleType_NidiumComponent;
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
            case 4: // Nidium component file
                module->m_ModuleType = JSModule::kModuleType_NidiumComponent;
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

        Path p(dir.c_str());
        PtrAutoDelete<Stream *> stream(Stream::Create(p.path()));

        if (!stream.ptr() || !stream.ptr()->exists()) continue;

        switch (i) {
            case 0: // index.js
                return true;
                break;
            case 1: // package.json
                char *data = NULL;
                size_t size;
                Path p(dir.c_str());

                if (!JSModules::GetFileContent(&p, &data, &size)
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
                std::string entrypoint = dir.substr(0, len) + std::string("/") + main;
                PtrAutoDelete<Stream *> streamEntrypoint(Stream::Create(entrypoint.c_str()));

                if (streamEntrypoint.ptr()->exists()) {
                    dir = entrypoint;
                    return true;
                } else {
                    fprintf(stderr, "Failed to access file %s\n", entrypoint.c_str());
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
    JS::RootedString name(cx);

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

    /*
        XXX Why do we have to unwrap the value from its RootedValue?
    */
    JSModule *module = static_cast<JSModule *>(reserved.address()->toPrivate());

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
