/*
    NativeJS Core Library
    Copyright (C) 2014 Anthony Catel <paraboul@gmail.com>

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

#include "NativeJSFS.h"

#include <dirent.h>

enum {
    NATIVE_JSFS_MSG_READDIR_FILE = 1
};

static JSBool native_fs_readDir(JSContext *cx, unsigned argc, jsval *vp);

static JSClass fs_class = {
    "fs", 0,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, NULL,
    JSCLASS_NO_OPTIONAL_MEMBERS
};

static JSFunctionSpec FS_static_funcs[] = {
    JS_FN("readDir",     native_fs_readDir, 2, 0),
    JS_FS_END
};


class NativeJSFSAsyncHandler : public NativeJSAsyncHandler
{
public:
    NativeJSFSAsyncHandler(JSContext *ctx) : NativeJSAsyncHandler(ctx) {

    }

    void onMessage(const NativeSharedMessages::Message &msg) {
        switch(msg.event()) {
            case NATIVE_JSFS_MSG_READDIR_FILE:
            {
                JS::Value rval, arg;

                dirent *cur = (dirent *)msg.dataPtr();
                JSObject *callback = this->getCallback(0);
                JSContext *cx, *m_Cx = this->getJSContext();

                cx = m_Cx;

                JSObject *param = JS_NewObject(cx, NULL, NULL, NULL);

                JSOBJ_SET_PROP_STR(param, "name",
                    JS_NewStringCopyZ(cx, cur->d_name));

                //JSOBJ_SET_PROP_CSTR(param, "type", NativeJSFS_dirtype_to_str(cur));

                arg = OBJECT_TO_JSVAL(param);

                JS_CallFunctionValue(cx, NULL,
                    OBJECT_TO_JSVAL(callback), 1, &arg, &rval);

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

static JSBool native_fs_readDir(JSContext *cx, unsigned argc, jsval *vp)
{
    return true;
    jsval callback;
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject caller(cx, &args.thisv().toObject());
    JSString *path;

    NATIVE_CHECK_ARGS("readDir", 2);

    if (!JS_ConvertArguments(cx, 1, args.array(), "S", &path)) {
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
    JSObject *fsObj;
    fsObj = JS_DefineObject(cx, JS_GetGlobalObject(cx), "fs",
        &fs_class , NULL, JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_READONLY);

    JS_DefineFunctions(cx, fsObj, FS_static_funcs);
}

