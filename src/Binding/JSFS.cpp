/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "Binding/JSFS.h"

#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "IO/FileSystem.h"

using Nidium::Core::SharedMessages;
using Nidium::Core::Task;
using Nidium::IO::FileSystem::mkdirp;

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

bool JSFS::JS_isDir(JSContext *cx, JS::CallArgs &args)
{
    JS::RootedString path(cx);

    if (!JS_ConvertArguments(cx, args, "S", path.address())) {

        args.rval().setBoolean(false);

        return true;

    }

    JSAutoByteString cpath(cx, path);

    struct stat statbuf;

    if (stat(strdup(cpath.ptr()), &statbuf) == -1) {

        args.rval().setBoolean(false);

        return true;

    }

    args.rval().setBoolean(S_ISDIR(statbuf.st_mode));

    return true;

}

bool JSFS::JS_isFile(JSContext *cx, JS::CallArgs &args)
{
    JS::RootedString path(cx);

    if (!JS_ConvertArguments(cx, args, "S", path.address())) {

        args.rval().setBoolean(false);

        return true;

    }

    JSAutoByteString cpath(cx, path);

    struct stat statbuf;

    if (stat(strdup(cpath.ptr()), &statbuf) == -1) {

        args.rval().setBoolean(false);

        return true;

    }

    args.rval().setBoolean(S_ISREG(statbuf.st_mode));

    return true;

}

bool JSFS::JS_createDirSync(JSContext *cx, JS::CallArgs &args)
{
    JS::RootedString path(cx);

    if (!JS_ConvertArguments(cx, args, "S", path.address())) {

        args.rval().setBoolean(false);

        return true;

    }


    JSAutoByteString cpath(cx, path);
    struct stat statbuf;

    stat(strdup(cpath.ptr()), &statbuf);

    if (S_ISDIR(statbuf.st_mode)) {

        args.rval().setBoolean(true);

        return true;

    }


    mkdirp(cpath.ptr());
    stat(strdup(cpath.ptr()), &statbuf);

    args.rval().setBoolean(S_ISDIR(statbuf.st_mode));

    return false;

}

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

bool JSFS::JS_removeSync(JSContext *cx, JS::CallArgs &args)
{
    JS::RootedString path(cx);

    if (!JS_ConvertArguments(cx, args, "S", path.address())) {

        args.rval().setBoolean(false);

        return true;

    }

    JSAutoByteString cpath(cx, path);

    int retval = remove(strdup(cpath.ptr()));

    args.rval().setBoolean(retval == 0);

    return true;

}

JSFunctionSpec *JSFS::ListMethods()
{
    static JSFunctionSpec funcs[] = {
        CLASSMAPPER_FN(JSFS, createDirSync, 1),
        CLASSMAPPER_FN(JSFS, isDir, 1),
        CLASSMAPPER_FN(JSFS, isFile, 1),
        CLASSMAPPER_FN(JSFS, readDir, 2),
        CLASSMAPPER_FN(JSFS, removeSync, 1),

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

