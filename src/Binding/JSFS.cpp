/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "Binding/JSFS.h"

#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifndef _MSC_VER
#include <dirent.h>
#endif

using Nidium::Core::SharedMessages;
using Nidium::Core::Task;

namespace Nidium {
namespace Binding {

// {{{ JSFSAsyncHandler
class JSFSAsyncHandler : public JSAsyncHandler
{
public:
    JSFSAsyncHandler(JSContext *ctx) : JSAsyncHandler(ctx)
    {
    }
    enum Message
    {
        kMessage_ReadDir = 1
    };

    void onMessage(const SharedMessages::Message &msg)
    {
        switch (msg.event()) {
            case kMessage_ReadDir: {
                dirent *cur          = static_cast<dirent *>(msg.dataPtr());
                JSObject *callback   = this->getCallback(0);
                JSContext *cx, *m_Cx = this->getJSContext();

                cx = m_Cx;
                JS::RootedValue rval(cx);
                JS::AutoValueArray<1> params(cx);

                JS::RootedObject param(cx, JS_NewPlainObject(cx));
                JS::RootedString str(cx, JS_NewStringCopyZ(cx, cur->d_name));

                NIDIUM_JSOBJ_SET_PROP_STR(param, "name", str);
                // NIDIUM_JSOBJ_SET_PROP_CSTR(param, "type",
                // JSFS_dirtype_to_str(cur));

                params[0].setObject(*param);

                JS::RootedValue cb(cx, JS::ObjectValue(*callback));

                JS_CallFunctionValue(cx, nullptr, cb, params, &rval);

                free(cur);
                break;
            }
            default:
                break;
        }
    }

    static void readDirTask(Task *task)
    {
        JSFSAsyncHandler *handler
            = static_cast<JSFSAsyncHandler *>(task->getObject());

        DIR *dir;

        if (!(dir
              = opendir(static_cast<const char *>(task->m_Args[0].toPtr())))) {
            return;
        }

        dirent *cur;

        while ((cur = readdir(dir)) != NULL) {
            if (strcmp(cur->d_name, ".") == 0
                || strcmp(cur->d_name, "..") == 0) {
                continue;
            }
            dirent *curcpy = static_cast<dirent *>(malloc(sizeof(dirent)));
            memcpy(curcpy, cur, sizeof(dirent));
            handler->postMessage(curcpy, kMessage_ReadDir);
        }

        closedir(dir);
    }

    void onMessageLost(const SharedMessages::Message &msg)
    {
        switch (msg.event()) {
            case kMessage_ReadDir:
                free(msg.dataPtr());
                break;
            default:
                break;
        }
    }
};
// }}}

// {{{ Implementation
bool JSFS::JS_readDir(JSContext *cx, JS::CallArgs &args)
{
    JS::RootedString path(cx);

    if (!JS_ConvertArguments(cx, args, "S", path.address())) {
        return false;
    }

    if (!JSUtils::ReportIfNotFunction(cx, args[1])) {
        return false;
    }

    JSAutoByteString cpath(cx, path);

    JSFSAsyncHandler *handler = new JSFSAsyncHandler(cx);

    Task *task = new Task();
    task->setFunction(JSFSAsyncHandler::readDirTask);
    task->m_Args[0].set(strdup(cpath.ptr()));

    handler->setCallback(0, args[1].toObjectOrNull());
    handler->addTask(task);

    return true;
}

JSFunctionSpec *JSFS::ListMethods()
{
    static JSFunctionSpec funcs[] = {
        CLASSMAPPER_FN(JSFS, readDir, 2),

        JS_FS_END
    };

    return funcs;
}

void JSFS::RegisterObject(JSContext *cx)
{
    JSFS::ExposeClass(cx, "NidiumFS");
    JS::RootedObject FSObject(cx,
    JSFS::CreateUniqueInstance(cx, new JSFS(), "fs"));


}
// }}}

} // namespace Binding
} // namespace Nidium

