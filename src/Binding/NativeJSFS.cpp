/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "NativeJSFS.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

enum {
    NATIVE_JSFS_MSG_READDIR_FILE = 1
};

static bool native_fs_readDir(JSContext *cx, unsigned argc, JS::Value *vp);

static JSClass fs_class = {
    "fs", 0,
    JS_PropertyStub, JS_DeletePropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, NULL,
    nullptr, nullptr, nullptr, nullptr, JSCLASS_NO_INTERNAL_MEMBERS
};

static JSFunctionSpec FS_static_funcs[] = {
    JS_FN("readDir",     native_fs_readDir, 2, NATIVE_JS_FNPROPS),
    JS_FS_END
};

class NativeJSFSAsyncHandler : public Nidium::Binding::JSAsyncHandler
{
public:
    NativeJSFSAsyncHandler(JSContext *ctx) : Nidium::Binding::JSAsyncHandler(ctx) {

    }

    void onMessage(const NativeSharedMessages::Message &msg) {
        switch(msg.event()) {
            case NATIVE_JSFS_MSG_READDIR_FILE:
            {
                dirent *cur = (dirent *)msg.dataPtr();
                JSObject *callback = this->getCallback(0);
                JSContext *cx, *m_Cx = this->getJSContext();

                cx = m_Cx;
                JS::RootedValue rval(cx);
                JS::AutoValueArray<1> params(cx);

                JS::RootedObject param(cx, JS_NewObject(cx, NULL, JS::NullPtr(), JS::NullPtr()));
                JS::RootedString str(cx, JS_NewStringCopyZ(cx, cur->d_name));

                JSOBJ_SET_PROP_STR(param, "name", str);
                //JSOBJ_SET_PROP_CSTR(param, "type", NativeJSFS_dirtype_to_str(cur));

                params[0].setObject(*param);

                JS::RootedValue cb(cx, OBJECT_TO_JSVAL(callback));

                JS_CallFunctionValue(cx, JS::NullPtr(), cb, params, &rval);

                free(cur);
                break;
            }
            default:
                break;
        }
    }

    void onMessageLost(const NativeSharedMessages::Message &msg)
    {
        switch (msg.event()) {
            case NATIVE_JSFS_MSG_READDIR_FILE:
                free(msg.dataPtr());
                break;
        }
    }
};

void NativeJSFS_readDir_Task(NativeTask *task)
{
    NativeJSFSAsyncHandler *handler = (NativeJSFSAsyncHandler *)task->getObject();

    DIR *dir;

    if (!(dir = opendir((const char *)task->args[0].toPtr()))) {
        return;
    }

    dirent *cur;

    while ((cur = readdir(dir)) != NULL) {
        if (strcmp(cur->d_name, ".") == 0 || strcmp(cur->d_name, "..") == 0) {
            continue;
        }
        dirent *curcpy = (dirent *)malloc(sizeof(dirent));
        memcpy(curcpy, cur, sizeof(dirent));
        handler->postMessage(curcpy, NATIVE_JSFS_MSG_READDIR_FILE);
    }

    closedir(dir);
}

static bool native_fs_readDir(JSContext *cx, unsigned argc, JS::Value *vp)
{
    return true;  //@FIXME why is this returning immed?

    JS::RootedValue callback(cx);
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject caller(cx, JS_THIS_OBJECT(cx, vp));
    JS::RootedString path(cx);

    NIDIUM_JS_CHECK_ARGS("readDir", 2);

    if (!JS_ConvertArguments(cx, args, "S", path.address())) {
        return false;
    }

    if (!JS_ConvertValue(cx, args[1], JSTYPE_FUNCTION, &callback)) {
        JS_ReportError(cx, "open() invalid callback");
        return false;
    }

    JSAutoByteString cpath(cx, path);

    NativeJSFSAsyncHandler *handler = new NativeJSFSAsyncHandler(cx);
    printf("Calling with cx : %p\n", cx);

    NativeTask *task = new NativeTask();
    task->setFunction(NativeJSFS_readDir_Task);
    task->args[0].set(strdup(cpath.ptr()));

    handler->setCallback(0, callback.toObjectOrNull());
    handler->addTask(task);

    return true;
}

void NativeJSFS::registerObject(JSContext *cx)
{
    JS::RootedObject global(cx, JS::CurrentGlobalOrNull(cx));
    JS::RootedObject fsObj(cx, JS_DefineObject(cx, global, "fs",
        &fs_class , NULL, JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY));

    JS_DefineFunctions(cx, fsObj, FS_static_funcs);
}

