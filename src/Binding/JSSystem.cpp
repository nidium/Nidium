/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "Binding/JSSystem.h"

#if 0
#ifdef NIDIUM_PRODUCT_UI
#include <SystemInterface.h>
#endif
#endif


#include <sys/stat.h>

#if defined(__linux__)
#include <sys/time.h>
#include <sys/resource.h>
#include <linux/stat.h>
#elif defined(__APPLE__)
#include <sys/errno.h>
#endif

namespace Nidium {
namespace Binding {

// {{{ Implementation
bool JSSystem::JS_getOpenFileStats(JSContext *cx, JS::CallArgs &args)
{
    struct rlimit rl;
    struct stat stats;
    JSContext *m_Cx = cx;

    if (getrlimit(RLIMIT_NOFILE, &rl) == -1) {
        JS_ReportWarning(cx, "Couldnt get limits values");
        args.rval().setNull();
        return true;
    }

    JS::RootedObject ret(
        cx, JS_NewObject(cx, nullptr, JS::NullPtr(), JS::NullPtr()));

    NIDIUM_JSOBJ_SET_PROP_INT(ret, "cur", (int)rl.rlim_cur);
    NIDIUM_JSOBJ_SET_PROP_INT(ret, "max", (int)rl.rlim_max);

    int fdcounter = 0, sockcounter = 0, othercount = 0;

    for (size_t i = 0; i <= rl.rlim_cur; i++) {
        if (fstat(i, &stats) == 0) {
            fdcounter++;
            if ((stats.st_mode & S_IFMT) == S_IFSOCK) {
                sockcounter++;
            } else {
                othercount++;
            }
        } else if (errno != EBADF) {
            fdcounter++;
        }
    }

    NIDIUM_JSOBJ_SET_PROP_INT(ret, "open", fdcounter);
    NIDIUM_JSOBJ_SET_PROP_INT(ret, "sockets", sockcounter);
    NIDIUM_JSOBJ_SET_PROP_INT(ret, "files", othercount);

    args.rval().setObjectOrNull(ret);

    return true;
}

#if 0
#ifdef NIDIUM_PRODUCT_UI
bool JSProcess::JSSetter_languagey(JSContext *cx, JS::MutableHandleValue vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

    SystemInterface* interface = SystemInterface::GetInstance();
    const char *clang = interface->getLanguage();

    args.rval().setString(JS_NewStringCopyZ(cx, clang));

    return true;
}
#endif
#endif

#if 0
JSPropertySpec *JSSystem::ListProperties()
{
    static JSPropertySpec props[] = {
#ifdef NIDIUM_PRODUCT_UI
        CLASSMAPPER_PROP_G(JSSystem, language),
#endif

        JS_PS_END
    };
    return props;
}
#endif

JSFunctionSpec *JSSystem::ListMethods()
{
    static JSFunctionSpec funcs[] = {
        CLASSMAPPER_FN(JSSystem, getOpenFileStats, 0),
        JS_FS_END
    };

    return funcs;
}

void JSSystem::RegisterObject(JSContext *cx)
{
    JSSystem::ExposeClass<0>(cx, "System");
    JSSystem::CreateUniqueInstance(cx, new JSSystem(), "NidiumSystem");
}
// }}}

} // namespace Binding
} // namespce Nidium
