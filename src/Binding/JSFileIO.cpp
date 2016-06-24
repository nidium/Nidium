/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "Binding/JSFileIO.h"

// sync API should have a different constructor (new FileSync)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ape_netlib.h>

#include "IO/Stream.h"
#include "Binding/JSUtils.h"

using Nidium::Core::Path;
using Nidium::Core::SharedMessages;
using Nidium::IO::Stream;
using Nidium::IO::File;

namespace Nidium {
namespace Binding {

// {{{ Preamble
#define FILE_ROOT_DEBUG 0

enum FileProp {
    kFileProp_FileSize,
    kFileProp_FileName,
    kFileProp_Binary, // not used
    kFileProp_Async   // not used
};

#define NJSFIO_GETTER(obj) (static_cast<JSFileIO *>(JS_GetPrivate(obj)))

static void File_Finalize(JSFreeOp *fop, JSObject *obj);

static bool nidium_file_prop_get(JSContext *cx, JS::HandleObject obj,
    uint8_t id, JS::MutableHandleValue vp);

#if 0
static bool nidium_file_prop_set(JSContext *cx, JS::HandleObject obj,
    JS::HandleId id, bool strict, JS::MutableHandleValue vp);
#endif

JSClass File_class = {
    "File", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_DeletePropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, File_Finalize,
    nullptr, nullptr, nullptr, nullptr, JSCLASS_NO_INTERNAL_MEMBERS
};

template<>
JSClass *JSExposer<JSFileIO>::jsclass = &File_class;

static bool nidium_file_open(JSContext *cx, unsigned argc, JS::Value *vp);
static bool nidium_file_openSync(JSContext *cx, unsigned argc, JS::Value *vp);
static bool nidium_file_readSync(JSContext *cx, unsigned argc, JS::Value *vp);
static bool nidium_file_read(JSContext *cx, unsigned argc, JS::Value *vp);
static bool nidium_file_seek(JSContext *cx, unsigned argc, JS::Value *vp);
static bool nidium_file_close(JSContext *cx, unsigned argc, JS::Value *vp);
static bool nidium_file_closeSync(JSContext *cx, unsigned argc, JS::Value *vp);
static bool nidium_file_write(JSContext *cx, unsigned argc, JS::Value *vp);
static bool nidium_file_isDir(JSContext *cx, unsigned argc, JS::Value *vp);
static bool nidium_file_listFiles(JSContext *cx, unsigned argc, JS::Value *vp);
static bool nidium_file_rmrf(JSContext *cx, unsigned argc, JS::Value *vp);

static bool nidium_file_readFileSync(JSContext *cx, unsigned argc, JS::Value *vp);
static bool nidium_file_readFile(JSContext *cx, unsigned argc, JS::Value *vp);

static JSPropertySpec File_props[] = {
    NIDIUM_JS_PSG("filesize", kFileProp_FileSize, nidium_file_prop_get),
    NIDIUM_JS_PSG("filename", kFileProp_FileName, nidium_file_prop_get),
    JS_PS_END
};

static JSFunctionSpec File_funcs[] = {
    JS_FN("open", nidium_file_open, 1, NIDIUM_JS_FNPROPS),
    JS_FN("openSync", nidium_file_openSync, 1, NIDIUM_JS_FNPROPS),
    JS_FN("readSync", nidium_file_readSync, 1, NIDIUM_JS_FNPROPS),
    JS_FN("read", nidium_file_read, 2, NIDIUM_JS_FNPROPS),
    JS_FN("seek", nidium_file_seek, 2, NIDIUM_JS_FNPROPS),
    JS_FN("close", nidium_file_close, 0, NIDIUM_JS_FNPROPS),
    JS_FN("closeSync", nidium_file_closeSync, 0, NIDIUM_JS_FNPROPS),
    JS_FN("write", nidium_file_write, 1, NIDIUM_JS_FNPROPS),
    JS_FN("isDir", nidium_file_isDir, 0, NIDIUM_JS_FNPROPS),
    JS_FN("listFiles", nidium_file_listFiles, 1, NIDIUM_JS_FNPROPS),
    JS_FN("rmrf", nidium_file_rmrf, 0, NIDIUM_JS_FNPROPS),
    JS_FS_END
};

static JSFunctionSpec File_static_funcs[] = {
    JS_FN("readSync", nidium_file_readFileSync, 1, NIDIUM_JS_FNPROPS),
    JS_FN("read",     nidium_file_readFile, 2, NIDIUM_JS_FNPROPS),
    JS_FS_END
};

static void File_Finalize(JSFreeOp *fop, JSObject *obj)
{
    JSFileIO *NJSFIO = static_cast<JSFileIO *>(JS_GetPrivate(obj));
    if (NJSFIO != NULL) {
#if FILE_ROOT_DEBUG
        printf("Finalize %p\n", obj);
#endif
        File *file = NJSFIO->getFile();
        delete file;
        delete NJSFIO;
    }
}

// }}}

// {{{ JSFileAsyncReader
class JSFileAsyncReader : public Core::Messages
{
public:
    static int DeleteStream(void *arg) {
        Stream *stream = static_cast<Stream *>(arg);

        delete stream;

        return 0;
    }

    void onMessage(const SharedMessages::Message &msg)
    {

        JSContext *cx = static_cast<JSContext *>(m_Args[0].toPtr());
        ape_global *ape = (ape_global *)JS_GetContextPrivate(cx);
        JS::AutoValueArray<2> params(cx);
        params[0].setNull();
        params[1].setUndefined();
        JS::RootedValue rval(cx);

        Stream *stream = static_cast<Stream *>(m_Args[2].toPtr());

        JS::RootedObject callback(cx, static_cast<JSObject *>(m_Args[7].toPtr()));

        char *encoding = static_cast<char *>(m_Args[1].toPtr());

        switch (msg.event()) {
            case Stream::kEvents_Error:
            {
                Stream::Errors err =
                static_cast<Stream::Errors>(msg.m_Args[0].toInt());
                printf("Got an error : %d\n", err);
                params[0].setString(JS_NewStringCopyZ(cx, "Stream error"));
            }
                break;
            case Stream::kEvents_ReadBuffer:
            {
                JS::RootedValue ret(cx);
                buffer *buf = static_cast<buffer *>(msg.m_Args[0].toPtr());
                if (JSUtils::StrToJsval(cx, reinterpret_cast<const char *>(buf->data),
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

        NidiumJS::GetObject(cx)->unrootObject(callback);

        stream->setListener(NULL);

        free(encoding);

        delete this;

        APE_timer_create(ape, 0, JSFileAsyncReader::DeleteStream, stream);
    }

    Core::Args m_Args;
};

// }}}

// {{{ JSFileIO
bool JSFileIO::HandleError(JSContext *cx, const SharedMessages::Message &msg,
    JS::MutableHandleValue vals)
{
    switch (msg.event()) {
        case File::kEvents_OpenError:
            // todo : replace by error object
            vals.setString(JS_NewStringCopyZ(cx, "Open error"));
            break;
        case File::kEvents_SeekError:
            vals.setString(JS_NewStringCopyZ(cx, "Seek error"));
            break;
        case File::kEvents_ReadError:
            vals.setString(JS_NewStringCopyZ(cx, "read error"));
            break;
        case File::kEvents_WriteError:
            vals.setString(JS_NewStringCopyZ(cx, "write error"));
            break;
        default:
            vals.setNull(); // no error
            return false;
    }

    return true;
}

static const char *JSFileIO_dirtype_to_str(const dirent *entry)
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

bool JSFileIO::callbackForMessage(JSContext *cx,
    const SharedMessages::Message &msg, JSObject *thisobj,
    const char *encoding)
{
    JS::AutoValueArray<2> params(cx);
    JS::RootedValue rval(cx);

    params[1].setUndefined();
    JSContext *m_Cx = cx;

    JS::RootedObject jsthis(cx, thisobj);
    if (!JSFileIO::HandleError(cx, msg, params[0])) {
        switch (msg.event()) {
            case File::kEvents_ReadSuccess:
            {
                buffer *buf = static_cast<buffer *>(msg.m_Args[0].toPtr());
                JSUtils::StrToJsval(cx, reinterpret_cast<const char *>(buf->data),
                    buf->used, params[1], encoding);
                break;
            }
            case File::kEvents_WriteSuccess:
                params[1].setDouble(msg.m_Args[0].toInt64());
                break;
            case File::kEvents_ListFiles:
            {
                File::DirEntries *entries = (File::DirEntries *)msg.m_Args[0].toPtr();
                JS::RootedObject arr(cx, JS_NewArrayObject(cx, entries->size));

                for (int i = 0; i < entries->size; i++) {
                    JS::RootedObject entry(cx, JS_NewObject(cx, NULL, JS::NullPtr(), JS::NullPtr()));

                    JS::RootedValue val(cx, OBJECT_TO_JSVAL(entry));
                    JS_SetElement(cx, arr, i, val);

                    JS::RootedString name(cx, JS_NewStringCopyZ(cx, entries->lst[i].d_name));
                    NIDIUM_JSOBJ_SET_PROP_STR(entry, "name", name);

                    NIDIUM_JSOBJ_SET_PROP_CSTR(entry, "type",
                        JSFileIO_dirtype_to_str(&entries->lst[i]));

                }

                params[1].setObject(*arr);
            }
        }
    }
    JSObject *callback = static_cast<JSObject *>(msg.m_Args[7].toPtr());

    if (!this->getFile()->m_TaskQueued) {
#if FILE_ROOT_DEBUG
        printf("Unroot %p\n", thisobj);
#endif
        NidiumJS::GetObject(cx)->unrootObject(jsthis);
    }

    if (!callback) {
        return false;
    }

    if (JS_ObjectIsCallable(cx, callback)) {

        JSAutoRequest ar(cx); // TODO: Why do we need a request here?
        JS::RootedValue cb(cx, OBJECT_TO_JSVAL(callback));
        JS_CallFunctionValue(cx, jsthis, cb, params, &rval);
    }

    NidiumJS::GetObject(cx)->unrootObject(callback);

    return true;
}

void JSFileIO::onMessage(const SharedMessages::Message &msg)
{
    this->callbackForMessage(m_Cx, msg, m_JSObject, m_Encoding);
}

JSObject *JSFileIO::GenerateJSObject(JSContext *cx, const char *path)
{
    JS::RootedObject ret(cx, JS_NewObject(cx, &File_class, JS::NullPtr(), JS::NullPtr()));
    File *file;
    JSFileIO *NJSFIO;

    NJSFIO = new JSFileIO(ret, cx, path);
    file = new File(path);
    file->setListener(NJSFIO);

    NJSFIO->setFile(file);

    JS_DefineFunctions(cx, ret, File_funcs);
    JS_DefineProperties(cx, ret, File_props);

    JS_SetPrivate(ret, NJSFIO);

    return ret;
}

File *JSFileIO::GetFileFromJSObject(JSContext *cx, JS::HandleObject jsobj)
{
    JSFileIO *nfio = static_cast<JSFileIO *>(JS_GetInstancePrivate(cx, jsobj,
        &File_class, NULL));

    return nfio ? nfio->getFile() : NULL;
}

// }}}

// {{{ Implementation

static bool nidium_file_prop_get(JSContext *cx, JS::HandleObject obj,
    uint8_t id, JS::MutableHandleValue vp)
{
    File *file = NJSFIO_GETTER(obj)->getFile();

    switch(id) {
        case kFileProp_FileSize:
            vp.set(JS_NumberValue(file->getFileSize()));
            break;
        case kFileProp_FileName:
            vp.set(STRING_TO_JSVAL(JS_NewStringCopyZ(cx, file->getFullPath())));
            break;
        default:break;

    }

    return true;
}

#if 0
// currently no props
static bool nidium_file_prop_set(JSContext *cx, JS::HandleObject obj,
    JS::HandleId id, bool strict, JS::MutableHandleValue vp)
{
    JSFileIO *nfio = NJSFIO_GETTER(obj);

    if (nfio == NULL) {
        return true;
    }

    return true;
}
#endif

static bool nidium_File_constructor(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NIDIUM_JS_CONSTRUCTOR_PROLOGUE()

    File *file;
    JSFileIO *NJSFIO;

    JS::RootedString url(cx);
    JS::RootedObject opt(cx);
    JS::RootedObject ret(cx, JS_NewObjectForConstructor(cx, &File_class, args));

    if (!JS_ConvertArguments(cx, args, "S/o", url.address(), opt.address())) {
        return false;
    }

    JSAutoByteString curl(cx, url);

    NJSFIO = new JSFileIO(ret, cx, curl.ptr());
    if (!NJSFIO->getPath() || !NJSFIO->allowSyncStream()) {
        JS_ReportError(cx, "FileIO : Invalid file path");
        return false;
    }

    file = new File(NJSFIO->getPath());
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

static bool nidium_file_isDir(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NIDIUM_JS_PROLOGUE_CLASS_NO_RET(JSFileIO, &File_class);

    File *file = CppObj->getFile();

    if (!file->isOpen()) {
        JS_ReportError(cx, "File has not been opened");
        return false;
    }

    args.rval().setBoolean(file->isDir());
#if FILE_ROOT_DEBUG
    printf("Catching %s %d\n", file->getFullPath(), file->isDir());
#endif
    return true;
}

static bool nidium_file_rmrf(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject caller(cx, JS_THIS_OBJECT(cx, vp));
    JSFileIO *NJSFIO;
    File *file;

    if (JS_InstanceOf(cx, caller, &File_class, &args) == false) {
        return false;
    }

    NJSFIO = static_cast<JSFileIO *>(JS_GetPrivate(caller));

    file = NJSFIO->getFile();

    file->rmrf();

    return true;
}

static bool nidium_file_seek(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::RootedValue callback(cx);
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject caller(cx, JS_THIS_OBJECT(cx, vp));

    JSFileIO *NJSFIO;
    File *file;
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

    NidiumJS::GetObject(cx)->rootObjectUntilShutdown(callback.toObjectOrNull());

    NJSFIO = static_cast<JSFileIO *>(JS_GetPrivate(caller));

    file = NJSFIO->getFile();

    file->seek(seek_pos, callback.toObjectOrNull());

    NidiumJS::GetObject(cx)->rootObjectUntilShutdown(caller);

    return true;
}

static bool nidium_file_listFiles(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::RootedValue callback(cx);
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject caller(cx, JS_THIS_OBJECT(cx, vp));
    JSFileIO *NJSFIO;
    File *file;

    if (JS_InstanceOf(cx, caller, &File_class, &args) == false) {
        return false;
    }

    NJSFIO = static_cast<JSFileIO *>(JS_GetPrivate(caller));

    NIDIUM_JS_CHECK_ARGS("listFiles", 1);

    if (!JS_ConvertValue(cx, args[0], JSTYPE_FUNCTION, &callback)) {
        JS_ReportError(cx, "listFiles() bad callback");
        return false;
    }

    file = NJSFIO->getFile();

    NidiumJS::GetObject(cx)->rootObjectUntilShutdown(callback.toObjectOrNull());

    /*
        If the directory is not open, open it anyway
    */
    if (!file->isOpen()) {
        file->open("r");
    }
    file->listFiles(callback.toObjectOrNull());

    NidiumJS::GetObject(cx)->rootObjectUntilShutdown(caller);

    return true;
}

// {{{ Async implementation

static bool nidium_file_write(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::RootedValue callback(cx);
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject caller(cx, JS_THIS_OBJECT(cx, vp));

    JSFileIO *NJSFIO;
    File *file;

    if (JS_InstanceOf(cx, caller, &File_class, &args) == false) {
        return false;
    }

    NJSFIO = static_cast<JSFileIO *>(JS_GetPrivate(caller));

    NIDIUM_JS_CHECK_ARGS("write", 2);

    if (!JS_ConvertValue(cx, args[1], JSTYPE_FUNCTION, &callback)) {
        JS_ReportError(cx, "write() bad callback");
        return false;
    }

    file = NJSFIO->getFile();

    if (args[0].isString()) {
        //printf("got a string to write\n");
        JS::RootedString str(cx, args[0].toString());
        JSAutoByteString cstr(cx, str);
        size_t len = strlen(cstr.ptr());

        NidiumJS::GetObject(cx)->rootObjectUntilShutdown(callback.toObjectOrNull());

        file->write(cstr.ptr(), len, callback.toObjectOrNull());

    } else if (args[0].isObject()) {
        JS::RootedObject jsobj(cx, args[0].toObjectOrNull());

        if (jsobj == NULL || !JS_IsArrayBufferObject(jsobj)) {
            JS_ReportError(cx, "INVALID_VALUE : only accept string or ArrayBuffer");
            return false;
        }
        uint32_t len = JS_GetArrayBufferByteLength(jsobj);
        uint8_t *data = JS_GetArrayBufferData(jsobj);

        NidiumJS::GetObject(cx)->rootObjectUntilShutdown(callback.toObjectOrNull());

        file->write(reinterpret_cast<char *>(data), len, callback.toObjectOrNull());

    } else {
        JS_ReportError(cx, "INVALID_VALUE : only accept string or ArrayBuffer");
        return false;
    }

    NidiumJS::GetObject(cx)->rootObjectUntilShutdown(caller);

    return true;
}

static bool nidium_file_read(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::RootedValue callback(cx);
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject caller(cx, JS_THIS_OBJECT(cx, vp));

    JSFileIO *NJSFIO;
    File *file;
    double read_size;

    if (JS_InstanceOf(cx, caller, &File_class, &args) == false) {
        return false;
    }

    if (!JS_ConvertArguments(cx,  args, "d", &read_size)) {
        return false;
    }

    NJSFIO = static_cast<JSFileIO *>(JS_GetPrivate(caller));

    NIDIUM_JS_CHECK_ARGS("read", 2);

    if (!JS_ConvertValue(cx, args[1], JSTYPE_FUNCTION, &callback)) {
        JS_ReportError(cx, "read() Bad callback");
        return false;
    }

    file = NJSFIO->getFile();

    NidiumJS::GetObject(cx)->rootObjectUntilShutdown(callback.toObjectOrNull());

    file->read(static_cast<uint64_t>(read_size), callback.toObjectOrNull());
#if FILE_ROOT_DEBUG
    printf("Root read %p\n", caller);
#endif
    NidiumJS::GetObject(cx)->rootObjectUntilShutdown(caller);

    return true;
}

static bool nidium_file_close(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject caller(cx, JS_THIS_OBJECT(cx, vp));

    JSFileIO *NJSFIO;
    File *file;

    if (JS_InstanceOf(cx, caller, &File_class, &args) == false) {
        return false;
    }

    NJSFIO = static_cast<JSFileIO *>(JS_GetPrivate(caller));

    file = NJSFIO->getFile();

    file->close();
#if FILE_ROOT_DEBUG
    printf("Root close %p\n", caller);
#endif
    NidiumJS::GetObject(cx)->rootObjectUntilShutdown(caller);

    return true;
}

static bool nidium_file_open(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::RootedValue callback(cx);
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    JS::RootedObject caller(cx, JS_THIS_OBJECT(cx, vp));
    JSFileIO *NJSFIO;
    File *file;
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

    NJSFIO = static_cast<JSFileIO *>(JS_GetPrivate(caller));

    file = NJSFIO->getFile();

    JSAutoByteString cmodes(cx, modes);

    NidiumJS::GetObject(cx)->rootObjectUntilShutdown(callback.toObjectOrNull());
    file->open(cmodes.ptr(), callback.toObjectOrNull());
#if FILE_ROOT_DEBUG
    printf("Root open %p\n", caller);
#endif
    NidiumJS::GetObject(cx)->rootObjectUntilShutdown(caller);

    return true;
}

static bool nidium_file_readFile(JSContext *cx, unsigned argc, JS::Value *vp)
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
    NidiumJS::GetObject(cx)->rootObjectUntilShutdown(&callback.toObject());

    Stream *stream = Stream::Create(Path(cfilename.ptr()));

    if (!stream) {
        JS_ReportError(cx, "couldn't open stream");
        return false;
    }

    NIDIUM_JS_GET_OPT_TYPE(opt, "encoding", String) {
        JSAutoByteString encoding(cx, __curopt.toString());
        cencoding = strdup(encoding.ptr());
    }

    JSFileAsyncReader *async = new JSFileAsyncReader();
    async->m_Args[0].set(cx);
    async->m_Args[1].set(cencoding);
    async->m_Args[2].set(stream);

    /* XXX RootedObject */
    async->m_Args[7].set(callback.toObjectOrNull());

    stream->setListener(async);
    stream->getContent();

    return true;
}

// }}}

// {{{ Sync implementation
static bool nidium_file_openSync(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NIDIUM_JS_PROLOGUE_CLASS_NO_RET(JSFileIO, &File_class);

    NIDIUM_JS_CHECK_ARGS("openSync", 1);

    if (!args[0].isString()) {
        JS_ReportError(cx, "First argument must be a string");
        return false;
    }

    if (!CppObj->allowSyncStream()) {
        JS_ReportError(cx, "Can't open this file for sync read");
        return false;
    }

    JSAutoByteString mode(cx, args[0].toString());
    int err = 0;

    File *file = CppObj->getFile();

    if (!file->openSync(mode.ptr(), &err)) {
        JS_ReportError(cx, "Failed to open file : %s (errno %d)\n", strerror(err), err);
        return false;
    }

    return true;
}
static bool nidium_file_readSync(JSContext *cx, unsigned argc, JS::Value *vp)
{
    NIDIUM_JS_PROLOGUE_CLASS_NO_RET(JSFileIO, &File_class);

    int err;
    char *bufferPtr;
    JS::RootedValue ret(cx);

    File *file = CppObj->getFile();

    if (!CppObj->allowSyncStream()) {
        JS_ReportError(cx, "Can't read this file synchronously");
        return false;
    }

    if (!CppObj->getFile()->isOpen()) {
        int openError = 0;
        if (!CppObj->getFile()->openSync("r", &openError)) {
            JS_ReportError(cx, "Failed to open file : %s (errno %d)", strerror(openError), openError);
            return false;
        }

    }

    uint64_t len = CppObj->getFile()->getFileSize();
    if (argc > 0 && args[0].isNumber()) {
        JS::ToUint64(cx, args[0], &len);
    }

    ssize_t readSize = file->readSync(len, &bufferPtr, &err);
    Core::PtrAutoDelete<char *> buffer(bufferPtr, free);

    if (readSize < 0) {
        if (err == 0) {
            JS_ReportError(cx, "Unable to read file (is it a directory?)");
        } else {
            JS_ReportError(cx, "Failed to read file : %s (errno %d)", strerror(err), err);
        }
        return false;
    }

    if (!JSUtils::StrToJsval(cx, buffer, readSize, &ret, CppObj->m_Encoding)) {
        return false;
    }

    args.rval().set(ret);

    return true;
}

#if 0
static bool nidium_file_writeSync(JSContext *cx, unsigned argc, JS::Value *vp)
{
    return true;
}
#endif

static bool nidium_file_closeSync(JSContext *cx, unsigned argc, JS::Value *vp)
{

    NIDIUM_JS_PROLOGUE_CLASS_NO_RET(JSFileIO, &File_class);

    if (!CppObj->allowSyncStream()) {
        JS_ReportError(cx, "Can't close this file synchronously");
        return false;
    }

    CppObj->getFile()->closeSync();

    return true;
}

static bool nidium_file_readFileSync(JSContext *cx, unsigned argc, JS::Value *vp)
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
    Path path(cfilename.ptr());

    if (!path.GetScheme()->AllowSyncStream()) {
        JS_ReportError(cx, "can't open this file for sync read");
        return false;
    }

    Core::PtrAutoDelete<Stream *> stream(path.CreateStream());

    if (!stream.ptr() || !stream.ptr()->getContentSync(&buf, &len)) {
        args.rval().setNull();
        printf("couldnt read the file\n");
        return true;
    }

    Core::PtrAutoDelete<char *> cbuf(buf, free);
    char *cencoding = NULL;
    JSAutoByteString encoding;

    NIDIUM_JS_GET_OPT_TYPE(opt, "encoding", String) {
        encoding.encodeLatin1(cx, __curopt.toString());
        cencoding = encoding.ptr();
    }

    JS::RootedValue ret(cx);

    if (!JSUtils::StrToJsval(cx, buf, len, &ret, cencoding)) {
        return false;
    }

    args.rval().set(ret);

    return true;
}

// }}}

// }}}

// {{{ Registration
void JSFileIO::RegisterObject(JSContext *cx)
{
    JS::RootedObject global(cx, JS::CurrentGlobalOrNull(cx));
    JS_InitClass(cx, global, JS::NullPtr(), &File_class,
        nidium_File_constructor,
        0, nullptr, nullptr, nullptr, File_static_funcs);
}

// }}}

} // namespace Binding
} // namespace Nidium

