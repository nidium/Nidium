/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "NativeJSFileIO.h"

// sync API should have a different constructor (new FileSync)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

//#include "Core/NativeMessages.h"
#include "IO/Stream.h"
#include "NativeJSUtils.h"

#define FILE_ROOT_DEBUG 0

enum {
    FILE_PROP_FILESIZE,
    FILE_PROP_FILENAME,
    FILE_PROP_BINARY,
    FILE_PROP_ASYNC
};

class NativeJSFileAsyncReader : public NativeMessages
{
public:
    void onMessage(const Nidium::Core::SharedMessages::Message &msg)
    {

        JSContext *cx = (JSContext *)m_Args[0].toPtr();
        JS::AutoValueArray<2> params(cx);
        params[0].setNull();
        params[1].setUndefined();
        JS::RootedValue rval(cx);

        Nidium::IO::Stream *stream = (Nidium::IO::Stream *)m_Args[2].toPtr();

        JS::RootedObject callback(cx, (JSObject *)m_Args[7].toPtr());

        char *encoding = (char *)m_Args[1].toPtr();

        switch (msg.event()) {
            case Nidium::IO::STREAM_ERROR: {
                Nidium::IO::Stream::StreamErrors err =
                (Nidium::IO::Stream::StreamErrors)msg.args[0].toInt();
                printf("Got an error : %d\n", err);
                params[0].setString(JS_NewStringCopyZ(cx, "Stream error"));
            }
                break;
            case Nidium::IO::STREAM_READ_BUFFER:
            {
                JS::RootedValue ret(cx);
                buffer *buf = (buffer *)msg.args[0].toPtr();
                if (NativeJSUtils::strToJsval(cx, (const char *)buf->data,
                    buf->used, &ret, encoding)) {

                    params[1].set(ret);
                }
                break;
            }
            default:
                return;
        }

        if (JS_ObjectIsCallable(cx, callback)) {

            JSAutoRequest ar(cx); // TODO: Why do we need a request here?
            JS::RootedValue cb(cx, OBJECT_TO_JSVAL(callback));
            JS_CallFunctionValue(cx, JS::NullPtr(), cb, params, &rval);
        }

        NativeJS::getNativeClass(cx)->unrootObject(callback);

        stream->setListener(NULL);

        free(encoding);

        delete stream;
        delete this;
    }

    Nidium::Core::Args m_Args;
};

#define NJSFIO_GETTER(obj) ((class NativeJSFileIO *)JS_GetPrivate(obj))

static void File_Finalize(JSFreeOp *fop, JSObject *obj);

static bool native_file_prop_get(JSContext *cx, JS::HandleObject obj,
    uint8_t id, JS::MutableHandleValue vp);

#if 0
static bool native_file_prop_set(JSContext *cx, JS::HandleObject obj,
    JS::HandleId id, bool strict, JS::MutableHandleValue vp);
#endif

JSClass File_class = {
    "File", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_DeletePropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, File_Finalize,
    nullptr, nullptr, nullptr, nullptr, JSCLASS_NO_INTERNAL_MEMBERS
};

template<>
JSClass *Nidium::Binding::JSExposer<NativeJSFileIO>::jsclass = &File_class;

static bool native_file_open(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_file_openSync(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_file_read(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_file_seek(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_file_close(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_file_closeSync(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_file_write(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_file_isDir(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_file_listFiles(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_file_rmrf(JSContext *cx, unsigned argc, JS::Value *vp);

static bool native_file_readFileSync(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_file_readFile(JSContext *cx, unsigned argc, JS::Value *vp);

static JSPropertySpec File_props[] = {
    NIDIUM_JS_PSG("filesize", FILE_PROP_FILESIZE, native_file_prop_get),
    NIDIUM_JS_PSG("filename", FILE_PROP_FILENAME, native_file_prop_get),
    JS_PS_END
};

static JSFunctionSpec File_funcs[] = {
    JS_FN("open", native_file_open, 1, NATIVE_JS_FNPROPS),
    JS_FN("openSync", native_file_openSync, 1, NATIVE_JS_FNPROPS),
    JS_FN("read", native_file_read, 2, NATIVE_JS_FNPROPS),
    JS_FN("seek", native_file_seek, 2, NATIVE_JS_FNPROPS),
    JS_FN("close", native_file_close, 0, NATIVE_JS_FNPROPS),
    JS_FN("closeSync", native_file_closeSync, 0, NATIVE_JS_FNPROPS),
    JS_FN("write", native_file_write, 1, NATIVE_JS_FNPROPS),
    JS_FN("isDir", native_file_isDir, 0, NATIVE_JS_FNPROPS),
    JS_FN("listFiles", native_file_listFiles, 1, NATIVE_JS_FNPROPS),
    JS_FN("rmrf", native_file_rmrf, 0, NATIVE_JS_FNPROPS),
    JS_FS_END
};

static JSFunctionSpec File_static_funcs[] = {
    JS_FN("readSync", native_file_readFileSync, 1, NATIVE_JS_FNPROPS),
    JS_FN("read",     native_file_readFile, 2, NATIVE_JS_FNPROPS),
    JS_FS_END
};

static void File_Finalize(JSFreeOp *fop, JSObject *obj)
{
    NativeJSFileIO *NJSFIO = (NativeJSFileIO *)JS_GetPrivate(obj);
    if (NJSFIO != NULL) {
#if FILE_ROOT_DEBUG
        printf("Finalize %p\n", obj);
#endif
        Nidium::IO::File *file = NJSFIO->getFile();
        delete file;
        delete NJSFIO;
    }
}

static bool native_file_prop_get(JSContext *cx, JS::HandleObject obj,
    uint8_t id, JS::MutableHandleValue vp)
{
    Nidium::IO::File *file = NJSFIO_GETTER(obj)->getFile();

    switch(id) {
        case FILE_PROP_FILESIZE:
            vp.set(JS_NumberValue(file->getFileSize()));
            break;
        case FILE_PROP_FILENAME:
            vp.set(STRING_TO_JSVAL(JS_NewStringCopyZ(cx, file->getFullPath())));
            break;
        default:break;

    }

    return true;
}

#if 0
// currently no props
static bool native_file_prop_set(JSContext *cx, JS::HandleObject obj,
    JS::HandleId id, bool strict, JS::MutableHandleValue vp)
{
    NativeJSFileIO *nfio = NJSFIO_GETTER(obj);

    if (nfio == NULL) {
        return true;
    }

    return true;
}
#endif

static bool native_File_constructor(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::RootedString url(cx);
    Nidium::IO::File *file;
    NativeJSFileIO *NJSFIO;

    JS::RootedObject opt(cx);

    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);

    if (!args.isConstructing()) {
        JS_ReportError(cx, "Bad constructor");
        return false;
    }

    JS::RootedObject ret(cx, JS_NewObjectForConstructor(cx, &File_class, args));

    if (!JS_ConvertArguments(cx, args, "S/o", url.address(), opt.address())) {
        return false;
    }

    JSAutoByteString curl(cx, url);
    NativePath path(curl.ptr());

    if (!path.path()) {
        JS_ReportError(cx, "Nidium::IO::FileIO : Invalid file path");
        return false;
    }

    NJSFIO = new NativeJSFileIO(ret, cx);
    file = new Nidium::IO::File(path.path());
    file->setListener(NJSFIO);

    NIDIUM_JS_INIT_OPT();

    NIDIUM_JS_GET_OPT_TYPE(opt, "encoding", String) {
        JSAutoByteString encoding(cx, __curopt.toString());
        NJSFIO->m_Encoding = strdup(encoding.ptr());
    }

    NJSFIO->setFile(file);

    args.rval().setObjectOrNull(ret);

    JS_DefineFunctions(cx, ret, File_funcs);
    JS_DefineProperties(cx, ret, File_props);

    JS_SetPrivate(ret, NJSFIO);

    return true;
}

static bool native_file_write(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::RootedValue callback(cx);
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject caller(cx, JS_THIS_OBJECT(cx, vp));

    NativeJSFileIO *NJSFIO;
    Nidium::IO::File *file;

    if (JS_InstanceOf(cx, caller, &File_class, &args) == false) {
        return false;
    }

    NJSFIO = (NativeJSFileIO *)JS_GetPrivate(caller);

    NIDIUM_JS_CHECK_ARGS("write", 2);

    if (!JS_ConvertValue(cx, args[1], JSTYPE_FUNCTION, &callback)) {
        JS_ReportError(cx, "write() bad callback");
        return false;
    }

    file = NJSFIO->getFile();

    if (!file->isOpen()) {
        file->open("a+");
    }

    if (args[0].isString()) {
        //printf("got a string to write\n");
        JS::RootedString str(cx, args[0].toString());
        JSAutoByteString cstr(cx, str);
        size_t len = strlen(cstr.ptr());

        NativeJS::getNativeClass(cx)->rootObjectUntilShutdown(callback.toObjectOrNull());

        file->write(cstr.ptr(), len, callback.toObjectOrNull());

    } else if (args[0].isObject()) {
        JS::RootedObject jsobj(cx, args[0].toObjectOrNull());

        if (jsobj == NULL || !JS_IsArrayBufferObject(jsobj)) {
            JS_ReportError(cx, "NATIVE_INVALID_VALUE : only accept string or ArrayBuffer");
            return false;
        }
        uint32_t len = JS_GetArrayBufferByteLength(jsobj);
        uint8_t *data = JS_GetArrayBufferData(jsobj);

        NativeJS::getNativeClass(cx)->rootObjectUntilShutdown(callback.toObjectOrNull());

        file->write((char *)data, len, callback.toObjectOrNull());

    } else {
        JS_ReportError(cx, "NATIVE_INVALID_VALUE : only accept string or ArrayBuffer");
        return false;
    }

    NativeJS::getNativeClass(cx)->rootObjectUntilShutdown(caller);

    return true;
}

static bool native_file_isDir(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject caller(cx, JS_THIS_OBJECT(cx, vp));
    NativeJSFileIO *NJSFIO;
    Nidium::IO::File *file;

    if (JS_InstanceOf(cx, caller, &File_class, &args) == false) {
        return false;
    }

    NJSFIO = (NativeJSFileIO *)JS_GetPrivate(caller);

    file = NJSFIO->getFile();

    args.rval().setBoolean(file->isDir());
#if FILE_ROOT_DEBUG
    printf("Catching %s %d\n", file->getFullPath(), file->isDir());
#endif
    return true;
}

static bool native_file_rmrf(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject caller(cx, JS_THIS_OBJECT(cx, vp));
    NativeJSFileIO *NJSFIO;
    Nidium::IO::File *file;

    if (JS_InstanceOf(cx, caller, &File_class, &args) == false) {
        return false;
    }

    NJSFIO = (NativeJSFileIO *)JS_GetPrivate(caller);

    file = NJSFIO->getFile();

    file->rmrf();

    return true;
}

static bool native_file_listFiles(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::RootedValue callback(cx);
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject caller(cx, JS_THIS_OBJECT(cx, vp));
    NativeJSFileIO *NJSFIO;
    Nidium::IO::File *file;

    if (JS_InstanceOf(cx, caller, &File_class, &args) == false) {
        return false;
    }

    NJSFIO = (NativeJSFileIO *)JS_GetPrivate(caller);

    NIDIUM_JS_CHECK_ARGS("listFiles", 1);

    if (!JS_ConvertValue(cx, args[0], JSTYPE_FUNCTION, &callback)) {
        JS_ReportError(cx, "listFiles() bad callback");
        return false;
    }

    file = NJSFIO->getFile();

    NativeJS::getNativeClass(cx)->rootObjectUntilShutdown(callback.toObjectOrNull());

    /*
        If the directory is not open, open it anyway
    */
    if (!file->isOpen()) {
        file->open("r");
    }
    file->listFiles(callback.toObjectOrNull());

    NativeJS::getNativeClass(cx)->rootObjectUntilShutdown(caller);

    return true;
}

static bool native_file_read(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::RootedValue callback(cx);
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject caller(cx, JS_THIS_OBJECT(cx, vp));

    NativeJSFileIO *NJSFIO;
    Nidium::IO::File *file;
    double read_size;

    if (JS_InstanceOf(cx, caller, &File_class, &args) == false) {
        return false;
    }

    if (!JS_ConvertArguments(cx,  args, "d", &read_size)) {
        return false;
    }

    NJSFIO = (NativeJSFileIO *)JS_GetPrivate(caller);

    NIDIUM_JS_CHECK_ARGS("read", 2);

    if (!JS_ConvertValue(cx, args[1], JSTYPE_FUNCTION, &callback)) {
        JS_ReportError(cx, "read() bad callback");
        return false;
    }

    file = NJSFIO->getFile();

    NativeJS::getNativeClass(cx)->rootObjectUntilShutdown(callback.toObjectOrNull());

    if (!file->isOpen()) {
        file->open("r");
    }

    file->read((uint64_t)read_size, callback.toObjectOrNull());
#if FILE_ROOT_DEBUG
    printf("Root read %p\n", caller);
#endif
    NativeJS::getNativeClass(cx)->rootObjectUntilShutdown(caller);

    return true;
}

static bool native_file_seek(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::RootedValue callback(cx);
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject caller(cx, JS_THIS_OBJECT(cx, vp));

    NativeJSFileIO *NJSFIO;
    Nidium::IO::File *file;
    double seek_pos;

    if (JS_InstanceOf(cx, caller, &File_class, &args) == false) {
        return false;
    }

    if (!JS_ConvertArguments(cx, args, "d", &seek_pos)) {
        return false;
    }

    NIDIUM_JS_CHECK_ARGS("seek", 2);

    if (!JS_ConvertValue(cx, args[1], JSTYPE_FUNCTION, &callback)) {
        JS_ReportError(cx, "seek() bad callback");
        return false;
    }

    NativeJS::getNativeClass(cx)->rootObjectUntilShutdown(callback.toObjectOrNull());

    NJSFIO = (NativeJSFileIO *)JS_GetPrivate(caller);

    file = NJSFIO->getFile();

    file->seek(seek_pos, callback.toObjectOrNull());

    NativeJS::getNativeClass(cx)->rootObjectUntilShutdown(caller);

    return true;
}

static bool native_file_close(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject caller(cx, JS_THIS_OBJECT(cx, vp));

    NativeJSFileIO *NJSFIO;
    Nidium::IO::File *file;

    if (JS_InstanceOf(cx, caller, &File_class, &args) == false) {
        return false;
    }

    NJSFIO = (NativeJSFileIO *)JS_GetPrivate(caller);

    file = NJSFIO->getFile();

    file->close();
#if FILE_ROOT_DEBUG
    printf("Root close %p\n", caller);
#endif
    NativeJS::getNativeClass(cx)->rootObjectUntilShutdown(caller);

    return true;
}

static bool native_file_open(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::RootedValue callback(cx);
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject caller(cx, JS_THIS_OBJECT(cx, vp));
    NativeJSFileIO *NJSFIO;
    Nidium::IO::File *file;
    JS::RootedString modes(cx);

    NIDIUM_JS_CHECK_ARGS("open", 2);

    if (JS_InstanceOf(cx, caller, &File_class, &args) == false) {
        return false;
    }

    if (!JS_ConvertArguments(cx, args, "S", modes.address())) {
        return false;
    }

    if (!JS_ConvertValue(cx, args[1], JSTYPE_FUNCTION, &callback)) {
        JS_ReportError(cx, "open() invalid callback");
        return false;
    }

    NJSFIO = (NativeJSFileIO *)JS_GetPrivate(caller);

    file = NJSFIO->getFile();

    JSAutoByteString cmodes(cx, modes);

    NativeJS::getNativeClass(cx)->rootObjectUntilShutdown(callback.toObjectOrNull());
    file->open(cmodes.ptr(), callback.toObjectOrNull());
#if FILE_ROOT_DEBUG
    printf("Root open %p\n", caller);
#endif
    NativeJS::getNativeClass(cx)->rootObjectUntilShutdown(caller);

    return true;
}

static bool native_file_openSync(JSContext *cx, unsigned argc, JS::Value *vp)
{
    return true;
}
#if 0
static bool native_file_readSync(JSContext *cx, unsigned argc, JS::Value *vp)
{
    return true;
}

static bool native_file_writeSync(JSContext *cx, unsigned argc, JS::Value *vp)
{
    return true;
}
#endif

static bool native_file_closeSync(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject caller(cx, JS_THIS_OBJECT(cx, vp));

    NativeJSFileIO *NJSFIO;
    Nidium::IO::File *file;

    if (JS_InstanceOf(cx, caller, &File_class, &args) == false) {
        return false;
    }

    NJSFIO = (NativeJSFileIO *)JS_GetPrivate(caller);

    file = NJSFIO->getFile();

    file->closeSync();

    return true;
}

static bool native_file_readFileSync(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::RootedString filename(cx);
    JS::RootedObject opt(cx);
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    char *buf;
    size_t len;

    NIDIUM_JS_INIT_OPT();

    if (!JS_ConvertArguments(cx, args, "S/o", filename.address(), opt.address())) {
        return false;
    }

    JSAutoByteString cfilename(cx, filename);
    NativePath path(cfilename.ptr());

    if (!path.getScheme()->allowSyncStream()) {
        JS_ReportError(cx, "can't open this file for sync read");
        return false;
    }

    NativePtrAutoDelete<Nidium::IO::Stream *> stream(path.createStream());

    if (!stream.ptr() || !stream.ptr()->getContentSync(&buf, &len)) {
        args.rval().setNull();
        printf("couldnt read the file\n");
        return true;
    }

    NativePtrAutoDelete<char *> cbuf(buf, free);
    char *cencoding = NULL;
    JSAutoByteString encoding;

    NIDIUM_JS_GET_OPT_TYPE(opt, "encoding", String) {
        encoding.encodeLatin1(cx, __curopt.toString());
        cencoding = encoding.ptr();
    }

    JS::RootedValue ret(cx);

    if (!NativeJSUtils::strToJsval(cx, buf, len, &ret, cencoding)) {
        return false;
    }

    args.rval().set(ret);

    return true;
}

static bool native_file_readFile(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::RootedObject opt(cx);
    JS::RootedObject secondarg(cx);
    JS::RootedString filename(cx);

    NIDIUM_JS_INIT_OPT();

    JS::RootedValue argcallback(cx);
    JS::RootedValue callback(cx);
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    char *cencoding = NULL;

    if (!JS_ConvertArguments(cx, args, "So", filename.address(), secondarg.address())) {
        return false;
    }

    if (JS_TypeOfValue(cx, args[1]) != JSTYPE_FUNCTION) {
        NIDIUM_JS_CHECK_ARGS("read", 3);

        opt = args[1].toObjectOrNull();
        argcallback = args[2];
    } else {
        argcallback = args[1];
    }

    if (!JS_ConvertValue(cx, argcallback, JSTYPE_FUNCTION, &callback)) {
        JS_ReportError(cx, "read() invalid callback");
        return false;
    }

    JSAutoByteString cfilename(cx, filename);
    NativeJS::getNativeClass(cx)->rootObjectUntilShutdown(&callback.toObject());

    Nidium::IO::Stream *stream = Nidium::IO::Stream::create(NativePath(cfilename.ptr()));

    if (!stream) {
        JS_ReportError(cx, "couldn't open stream");
        return false;
    }

    NIDIUM_JS_GET_OPT_TYPE(opt, "encoding", String) {
        JSAutoByteString encoding(cx, __curopt.toString());
        cencoding = strdup(encoding.ptr());
    }

    NativeJSFileAsyncReader *async = new NativeJSFileAsyncReader();
    async->m_Args[0].set(cx);
    async->m_Args[1].set(cencoding);
    async->m_Args[2].set(stream);

    /* XXX RootedObject */
    async->m_Args[7].set(callback.toObjectOrNull());

    stream->setListener(async);
    stream->getContent();

    return true;
}

bool NativeJSFileIO::handleError(JSContext *cx, const Nidium::Core::SharedMessages::Message &msg,
    JS::MutableHandleValue vals)
{
    switch (msg.event()) {
        case Nidium::IO::FILE_OPEN_ERROR:
            // todo : replace by error object
            vals.setString(JS_NewStringCopyZ(cx, "Open error"));
            break;
        case Nidium::IO::FILE_SEEK_ERROR:
            vals.setString(JS_NewStringCopyZ(cx, "Seek error"));
            break;
        case Nidium::IO::FILE_READ_ERROR:
            vals.setString(JS_NewStringCopyZ(cx, "read error"));
            break;
        case Nidium::IO::FILE_WRITE_ERROR:
            vals.setString(JS_NewStringCopyZ(cx, "write error"));
            break;
        default:
            vals.setNull(); // no error
            return false;
    }

    return true;
}

static const char *NativeJSFileIO_dirtype_to_str(const dirent *entry)
{
    switch (entry->d_type) {
        case DT_DIR:
            return "dir";
        case DT_REG:
            return "file";
        case DT_LNK:
            return "link";
        case DT_SOCK:
            return "socket";
        default:
            return "unknown";
    }
}

bool NativeJSFileIO::callbackForMessage(JSContext *cx,
    const Nidium::Core::SharedMessages::Message &msg, JSObject *thisobj,
    const char *encoding)
{
    JS::AutoValueArray<2> params(cx);
    JS::RootedValue rval(cx);

    params[1].setUndefined();
    JSContext *m_Cx = cx;

    JS::RootedObject jsthis(cx, thisobj);
    if (!NativeJSFileIO::handleError(cx, msg, params[0])) {
        switch (msg.event()) {
            case Nidium::IO::FILE_READ_SUCCESS:
            {
                buffer *buf = (buffer *)msg.args[0].toPtr();
                NativeJSUtils::strToJsval(cx, (const char *)buf->data,
                    buf->used, params[1], encoding);
                break;
            }
            case Nidium::IO::FILE_WRITE_SUCCESS:
                params[1].setDouble(msg.args[0].toInt64());
                break;
            case Nidium::IO::FILE_LISTFILES_ENTRIES:
            {
                Nidium::IO::File::DirEntries *entries = (Nidium::IO::File::DirEntries *)msg.args[0].toPtr();
                JS::RootedObject arr(cx, JS_NewArrayObject(cx, entries->size));

                for (int i = 0; i < entries->size; i++) {
                    JS::RootedObject entry(cx, JS_NewObject(cx, NULL, JS::NullPtr(), JS::NullPtr()));

                    JS::RootedValue val(cx, OBJECT_TO_JSVAL(entry));
                    JS_SetElement(cx, arr, i, val);

                    JS::RootedString name(cx, JS_NewStringCopyZ(cx, entries->lst[i].d_name));
                    NIDIUM_JSOBJ_SET_PROP_STR(entry, "name", name);

                    NIDIUM_JSOBJ_SET_PROP_CSTR(entry, "type",
                        NativeJSFileIO_dirtype_to_str(&entries->lst[i]));

                }

                params[1].setObject(*arr);
            }
        }
    }
    JSObject *callback = (JSObject *)msg.args[7].toPtr();

    if (!this->getFile()->m_TaskQueued) {
#if FILE_ROOT_DEBUG
        printf("Unroot %p\n", thisobj);
#endif
        NativeJS::getNativeClass(cx)->unrootObject(jsthis);
    }

    if (!callback) {
        return false;
    }

    if (JS_ObjectIsCallable(cx, callback)) {

        JSAutoRequest ar(cx); // TODO: Why do we need a request here?
        JS::RootedValue cb(cx, OBJECT_TO_JSVAL(callback));
        JS_CallFunctionValue(cx, jsthis, cb, params, &rval);
    }

    NativeJS::getNativeClass(cx)->unrootObject(callback);

    return true;
}

void NativeJSFileIO::onMessage(const Nidium::Core::SharedMessages::Message &msg)
{
    this->callbackForMessage(m_Cx, msg, m_JSObject, m_Encoding);
}

void NativeJSFileIO::registerObject(JSContext *cx)
{
    JS::RootedObject global(cx, JS::CurrentGlobalOrNull(cx));
    JS_InitClass(cx, global, JS::NullPtr(), &File_class,
        native_File_constructor,
        0, nullptr, nullptr, nullptr, File_static_funcs);
}

JSObject *NativeJSFileIO::generateJSObject(JSContext *cx, const char *path)
{
    JS::RootedObject ret(cx, JS_NewObject(cx, &File_class, JS::NullPtr(), JS::NullPtr()));
    Nidium::IO::File *file;
    NativeJSFileIO *NJSFIO;

    NJSFIO = new NativeJSFileIO(ret, cx);
    file = new Nidium::IO::File(path);
    file->setListener(NJSFIO);

    NJSFIO->setFile(file);

    JS_DefineFunctions(cx, ret, File_funcs);
    JS_DefineProperties(cx, ret, File_props);

    JS_SetPrivate(ret, NJSFIO);

    return ret;
}

Nidium::IO::File *NativeJSFileIO::GetFileFromJSObject(JSContext *cx, JS::HandleObject jsobj)
{
    NativeJSFileIO *nfio = (NativeJSFileIO *)JS_GetInstancePrivate(cx, jsobj,
        &File_class, NULL);

    return nfio ? nfio->getFile() : NULL;
}

