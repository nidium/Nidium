/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "Binding/JSFile.h"

// sync API should have a different constructor (new FileSync)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ape_netlib.h>
#include <js/Conversions.h>

#ifdef _MSC_VER
#include "Port/MSWindows.h"
#else
#include <unistd.h>
#endif

#include "IO/Stream.h"
#include "Binding/JSUtils.h"
#include "Binding/ThreadLocalContext.h"

#include <prio.h>

using Nidium::Core::Path;
using Nidium::Core::SharedMessages;
using Nidium::IO::Stream;
using Nidium::IO::File;

namespace Nidium {
namespace Binding {

JSFile::~JSFile()
{
    if (m_Encoding) {
        free(m_Encoding);
    }

    File *file = this->getFile();
    if (file) {
        delete file;
    }
}

class JSFileAsyncReader : public Core::Messages
{
public:
    static int DeleteStream(void *arg)
    {
        Stream *stream = static_cast<Stream *>(arg);

        delete stream;

        return 0;
    }

    void onMessage(const SharedMessages::Message &msg)
    {

        JSContext *cx   = static_cast<JSContext *>(m_Args[0].toPtr());
        ape_global *ape = (ape_global *)JS_GetContextPrivate(cx);
        JS::AutoValueArray<2> params(cx);
        params[0].setNull();
        params[1].setUndefined();
        JS::RootedValue rval(cx);

        Stream *stream = static_cast<Stream *>(m_Args[2].toPtr());

        nidiumRootedThingRef *ref = (nidiumRootedThingRef *)m_Args[7].toPtr();

        if (!ref) {
            return;
        }

        JS::RootedObject callback(cx, ref->get());

        char *encoding = static_cast<char *>(m_Args[1].toPtr());

        switch (msg.event()) {
            case Stream::kEvents_Error: {
                Stream::Errors err
                    = static_cast<Stream::Errors>(msg.m_Args[0].toInt());
                ndm_logf(NDM_LOG_ERROR, "JSFile", "Got an error : %d", err);
                params[0].setString(JS_NewStringCopyZ(cx, "Stream error"));
            } break;
            case Stream::kEvents_ReadBuffer: {
                JS::RootedValue ret(cx);
                buffer *buf = static_cast<buffer *>(msg.m_Args[0].toPtr());
                if (JSUtils::StrToJsval(
                        cx, reinterpret_cast<const char *>(buf->data),
                        buf->used, &ret, encoding)) {

                    params[1].set(ret);
                }
                break;
            }
            default:
                return;
        }

        if (JS::IsCallable(callback)) {

            JSAutoRequest ar(cx); // TODO: Why do we need a request here?
            JS::RootedValue cb(cx, JS::ObjectValue(*callback));
            JS_CallFunctionValue(cx, nullptr, cb, params, &rval);
        }

        NidiumLocalContext::UnrootObject(ref);

        stream->setListener(NULL);

        free(encoding);

        delete this;

        APE_timer_create(ape, 0, JSFileAsyncReader::DeleteStream, stream);
    }

    Core::Args m_Args;
};

bool JSFile::HandleError(JSContext *cx,
                         const SharedMessages::Message &msg,
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

static const char *JSFile_dirtype_to_str(const PRDirEntry *entry)
{
    PRFileInfo info;
    if (PR_GetFileInfo(entry->name, &info) == PR_SUCCESS) {
        switch (info.type) {
        case PR_FILE_DIRECTORY:
            return "dir";
        case PR_FILE_FILE:
            return "file";
        case PR_FILE_OTHER:
            return "other";
        default:
            break;
        }
    }
    return "unknown";
}

void JSFile::onMessage(const SharedMessages::Message &msg)
{
    JSContext *cx = m_Cx;

    JS::AutoValueArray<2> params(cx);
    JS::RootedValue rval(cx);

    params[1].setUndefined();

    if (!JSFile::HandleError(cx, msg, params[0])) {
        switch (msg.event()) {
            case File::kEvents_ReadSuccess: {
                buffer *buf = static_cast<buffer *>(msg.m_Args[0].toPtr());
                JSUtils::StrToJsval(cx,
                                    reinterpret_cast<const char *>(buf->data),
                                    buf->used, params[1], m_Encoding);
                break;
            }
            case File::kEvents_WriteSuccess:
                params[1].setDouble(msg.m_Args[0].toInt64());
                break;
            case File::kEvents_ListFiles: {
                File::DirEntries *entries
                    = (File::DirEntries *)msg.m_Args[0].toPtr();
                JS::RootedObject arr(cx, JS_NewArrayObject(cx, entries->size));

                for (int i = 0; i < entries->size; i++) {
                    JS::RootedObject entry(cx, JS_NewPlainObject(cx));

                    JS::RootedValue val(cx, JS::ObjectValue(*entry));
                    JS_SetElement(cx, arr, i, val);

                    JS::RootedString name(
                        cx, JS_NewStringCopyZ(cx, entries->lst[i].name));
                    NIDIUM_JSOBJ_SET_PROP_STR(entry, "name", name);

                    NIDIUM_JSOBJ_SET_PROP_CSTR(
                        entry, "type", JSFile_dirtype_to_str(&entries->lst[i]));
                }

                params[1].setObject(*arr);
            }
        }
    }

    nidiumRootedThingRef *ref = (nidiumRootedThingRef *)msg.m_Args[7].toPtr();

    if (!ref) {
        return;
    }

    JS::RootedObject callback(cx, ref->get());

    if (JS::IsCallable(callback)) {

        JSAutoRequest ar(cx); // TODO: Why do we need a request here?
        JS::RootedValue cb(cx, JS::ObjectValue(*callback));
        JS::RootedObject jsthis(cx, getJSObject());

        JS_CallFunctionValue(cx, jsthis, cb, params, &rval);
    }

    if (!this->getFile()->hasTaskOrMessagePending()) {
        this->unroot();
    }

    NidiumLocalContext::UnrootObject(ref);

}

JSObject *JSFile::GenerateJSObject(JSContext *cx, const char *path)
{
    File *file;
    JSFile *jsfile;

    jsfile = new JSFile(path);
    file = new File(path);
    file->setListener(jsfile);
    file->setAutoClose(false);

    jsfile->setFile(file);

    JS::RootedObject ret(cx, JSFile::CreateObject(cx, jsfile));

    return ret;
}

File *JSFile::GetFileFromJSObject(JSContext *cx, JS::HandleObject jsobj)
{
    JSFile *nfio = JSFile::GetInstance(jsobj);

    return nfio ? nfio->getFile() : NULL;
}


bool JSFile::JSGetter_filesize(JSContext *cx, JS::MutableHandleValue vp)
{
    File *file = this->getFile();

    vp.set(JS_NumberValue(file->getFileSize()));

    return true;
}

bool JSFile::JSGetter_filename(JSContext *cx, JS::MutableHandleValue vp)
{
    File *file = this->getFile();

    vp.setString(JS_NewStringCopyZ(cx, file->getFullPath()));

    return true;
}

bool JSFile::JS_isDir(JSContext *cx, JS::CallArgs &args)
{
    File *file = this->getFile();

    if (!file->isOpen()) {
        JS_ReportError(cx, "File has not been opened");
        return false;
    }

    args.rval().setBoolean(file->isDir());

    return true;
}

bool JSFile::JS_rm(JSContext *cx, JS::CallArgs &args)
{
    this->getFile()->rm();

    return true;
}

bool JSFile::JS_rmrf(JSContext *cx, JS::CallArgs &args)
{
    this->getFile()->rmrf();

    return true;
}

bool JSFile::JS_listFiles(JSContext *cx, JS::CallArgs &args)
{
    File *file = this->getFile();

    if (!JSUtils::ReportIfNotFunction(cx, args[0])) {
        return false;
    }

    JS::RootedObject cb(cx, args[0].toObjectOrNull());

    nidiumRootedThingRef *ref =
        NidiumLocalContext::RootNonHeapObjectUntilShutdown(cb);

    /*
        If the directory is not open, open it anyway
    */
    if (!file->isOpen()) {
        file->open("r");
    }

    ndm_logf(NDM_LOG_DEBUG, "JSFile", "List file to %p", ref);
    file->listFiles(ref);

    this->root();

    return true;
}

bool JSFile::JS_seek(JSContext *cx, JS::CallArgs &args)
{
    double seek_pos;

    if (!JS_ConvertArguments(cx, args, "d", &seek_pos)) {
        return false;
    }

    if (!JSUtils::ReportIfNotFunction(cx, args[1])) {
        return false;
    }

    nidiumRootedThingRef *ref =
        NidiumLocalContext::RootNonHeapObjectUntilShutdown(args[1].toObjectOrNull());

    this->getFile()->seek(seek_pos, ref);

    this->root();

    return true;
}

bool JSFile::JS_write(JSContext *cx, JS::CallArgs &args)
{
    File *file = this->getFile();

    if (!JSUtils::ReportIfNotFunction(cx, args[1])) {
        return false;
    }

    if (args[0].isString()) {
        JS::RootedString str(cx, args[0].toString());
        JSAutoByteString cstr;

        if (strcmp(m_Encoding, "utf8") == 0) {
            cstr.encodeUtf8(cx, str);
        } else {
            cstr.encodeLatin1(cx, str);
        }

        nidiumRootedThingRef *ref =
            NidiumLocalContext::RootNonHeapObjectUntilShutdown(args[1].toObjectOrNull());

        file->write(cstr.ptr(), cstr.length(), ref);
    } else if (args[0].isObject()) {
        JS::RootedObject jsobj(cx, args[0].toObjectOrNull());

        if (jsobj == NULL || !JS_IsArrayBufferObject(jsobj)) {
            JS_ReportError(cx,
                           "INVALID_VALUE : only accept string or ArrayBuffer");
            return false;
        }
        uint32_t len  = JS_GetArrayBufferByteLength(jsobj);

        bool shared;
        JS::AutoCheckCannotGC nogc;
        uint8_t *data = JS_GetArrayBufferData(jsobj, &shared, nogc);

        nidiumRootedThingRef *ref =
            NidiumLocalContext::RootNonHeapObjectUntilShutdown(args[1].toObjectOrNull());

        file->write(reinterpret_cast<char *>(data), len, ref);
    } else {
        JS_ReportError(cx, "INVALID_VALUE : only accept string or ArrayBuffer");
        return false;
    }

    this->root();

    return true;
}

bool JSFile::JS_read(JSContext *cx, JS::CallArgs &args)
{
    double read_size;

    if (!JS_ConvertArguments(cx, args, "d", &read_size)) {
        return false;
    }

    if (!JSUtils::ReportIfNotFunction(cx, args[1])) {
        return false;
    }

    nidiumRootedThingRef *ref =
        NidiumLocalContext::RootNonHeapObjectUntilShutdown(args[1].toObjectOrNull());

    this->getFile()->read(static_cast<uint64_t>(read_size), ref);

    this->root();

    return true;
}

bool JSFile::JS_close(JSContext *cx, JS::CallArgs &args)
{
    this->getFile()->close();

    this->root();

    return true;
}

bool JSFile::JS_open(JSContext *cx, JS::CallArgs &args)
{
    JS::RootedString modes(cx);

    if (!JS_ConvertArguments(cx, args, "S", modes.address())) {
        return false;
    }

    ndm_logf(NDM_LOG_DEBUG, "JSFile", "Argc is %d", args.length());

    if (!JSUtils::ReportIfNotFunction(cx, args[1])) {
        return false;
    }

    JSAutoByteString cmodes(cx, modes);

    nidiumRootedThingRef *ref =
        NidiumLocalContext::RootNonHeapObjectUntilShutdown(args[1].toObjectOrNull());

    this->getFile()->open(cmodes.ptr(), ref);

    this->root();

    return true;
}

bool JSFile::JSStatic_read(JSContext *cx, JS::CallArgs &args)
{
    JS::RootedObject opt(cx);
    JS::RootedObject secondarg(cx);
    JS::RootedString filename(cx);

    NIDIUM_JS_INIT_OPT();

    JS::RootedValue argcallback(cx);

    char *cencoding   = NULL;

    if (!JS_ConvertArguments(cx, args, "So", filename.address(),
                             secondarg.address())) {
        return false;
    }

    if (JS_TypeOfValue(cx, args[1]) != JSTYPE_FUNCTION) {

        if (!args.requireAtLeast(cx, "read", 3)) {
            return false;
        }

        opt         = args[1].toObjectOrNull();
        argcallback = args[2];
    } else {
        argcallback = args[1];
    }

    if (!JSUtils::ReportIfNotFunction(cx, argcallback)) {
        return false;
    }

    JSAutoByteString cfilename(cx, filename);

    nidiumRootedThingRef *ref =
        NidiumLocalContext::RootNonHeapObjectUntilShutdown(&argcallback.toObject());

    Stream *stream = Stream::Create(Path(cfilename.ptr()));

    if (!stream) {
        JS_ReportError(cx, "Couldn't open stream");
        return false;
    }

    NIDIUM_JS_GET_OPT_TYPE(opt, "encoding", String)
    {
        JSAutoByteString encoding(cx, __curopt.toString());
        cencoding = strdup(encoding.ptr());
    }

    JSFileAsyncReader *async = new JSFileAsyncReader();
    async->m_Args[0].set(cx);
    async->m_Args[1].set(cencoding);
    async->m_Args[2].set(stream);

    /* XXX RootedObject */
    async->m_Args[7].set(ref);

    stream->setListener(async);
    stream->getContent();

    return true;
}


bool JSFile::JSStatic_readSync(JSContext *cx, JS::CallArgs &args)
{
    JS::RootedString filename(cx);
    JS::RootedObject opt(cx);

    char *buf;
    size_t len;

    NIDIUM_JS_INIT_OPT();

    if (!JS_ConvertArguments(cx, args, "S/o", filename.address(),
                             opt.address())) {
        return false;
    }

    JSAutoByteString cfilename(cx, filename);
    Path path(cfilename.ptr());

    if (!path.GetScheme()->AllowSyncStream()) {
        JS_ReportError(cx, "Can't open this file for sync read");
        return false;
    }

    Core::PtrAutoDelete<Stream *> stream(path.CreateStream());

    if (!stream.ptr() || !stream.ptr()->getContentSync(&buf, &len)) {
        args.rval().setNull();
        ndm_log(NDM_LOG_ERROR, "JSFile", "Couldn't read the file");
        return true;
    }

    Core::PtrAutoDelete<char *> cbuf(buf, free);
    char *cencoding = NULL;
    JSAutoByteString encoding;

    NIDIUM_JS_GET_OPT_TYPE(opt, "encoding", String)
    {
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
// {{{ Sync implementation
bool JSFile::JS_openSync(JSContext *cx, JS::CallArgs &args)
{
    if (!args[0].isString()) {
        JS_ReportError(cx, "First argument must be a string");
        return false;
    }

    if (!this->allowSyncStream()) {
        JS_ReportError(cx, "Can't open this file for sync read");
        return false;
    }

    JSAutoByteString mode(cx, args[0].toString());
    int err = 0;

    File *file = this->getFile();

    if (!file->openSync(mode.ptr(), &err) || err != 0) {
        JS_ReportError(cx, "Failed to open file : %s (errno %d)\n",
                       strerror(err), err);
        return false;
    }

    return true;
}

bool JSFile::JS_readSync(JSContext *cx, JS::CallArgs &args)
{
    int err;
    char *bufferPtr;
    JS::RootedValue ret(cx);

    File *file = this->getFile();

    if (!this->allowSyncStream()) {
        JS_ReportError(cx, "Can't read this file synchronously");
        return false;
    }

    if (!file->isOpen()) {
        int openError = 0;
        if (!file->openSync("r", &openError)) {
            JS_ReportError(cx, "Failed to open file : %s (errno %d)",
                           strerror(openError), openError);
            return false;
        }
    }

    uint64_t len = file->getFileSize();
    if (args.length() > 0 && args[0].isNumber()) {
        JS::ToUint64(cx, args[0], &len);
    }

    ssize_t readSize = file->readSync(len, &bufferPtr, &err);
    Core::PtrAutoDelete<char *> buffer(bufferPtr, free);

    if (readSize < 0) {
        if (err == 0) {
            JS_ReportError(cx, "Unable to read file : %s",
                           !file->isOpen() ? "not opened"
                                           : "is it a directory?");
        } else {
            JS_ReportError(cx, "Failed to read file : %s (errno %d)",
                           strerror(err), err);
        }
        return false;
    }

    if (!JSUtils::StrToJsval(cx, buffer, readSize, &ret, m_Encoding)) {
        return false;
    }

    args.rval().set(ret);

    return true;
}

bool JSFile::JS_seekSync(JSContext *cx, JS::CallArgs &args)
{
    File *file = this->getFile();

    size_t seekPos;
    int err;
    int ret;

    if (!args[0].isNumber()) {
        JS_ReportError(cx, "First argument must be a number");
        return false;
    }

    seekPos = static_cast<size_t>(args[0].toNumber());
    ret     = file->seekSync(seekPos, &err);

    if (ret != 0) {
        char errStr[2048];
        if (err == 0) {
            snprintf(errStr, 2047, "Unable to seek to %zd : %s", seekPos,
                     "Not opened or file is a directory");
        } else {
            snprintf(errStr, 2047, "Failed to seek to %zd : %s (errno %d)",
                     seekPos, strerror(err), err);
        }

        JS_ReportError(cx, errStr);
        return false;
    }

    return true;
}

bool JSFile::JS_writeSync(JSContext *cx, JS::CallArgs &args)
{
    File *file = this->getFile();
    int err, ret;

    if (args[0].isString()) {
        JS::RootedString str(cx, args[0].toString());
        JSAutoByteString cstr;

        if (strcmp(m_Encoding, "utf8") == 0) {
            cstr.encodeUtf8(cx, str);
        } else {
            cstr.encodeLatin1(cx, str);
        }

        ret =file->writeSync(cstr.ptr(), cstr.length(), &err);
    } else if (args[0].isObject()) {
        JS::RootedObject jsobj(cx, args[0].toObjectOrNull());

        if (jsobj == NULL || !JS_IsArrayBufferObject(jsobj)) {
            JS_ReportError(cx,
                           "INVALID_VALUE : only accept string or ArrayBuffer");
            return false;
        }

        uint32_t len  = JS_GetArrayBufferByteLength(jsobj);

        bool shared;
        JS::AutoCheckCannotGC nogc;
        uint8_t *data = JS_GetArrayBufferData(jsobj, &shared, nogc);

        ret = file->writeSync(reinterpret_cast<char *>(data), len, &err);
    } else {
        JS_ReportError(cx, "INVALID_VALUE : only accept string or ArrayBuffer");
        return false;
    }

    if (err != 0 || ret < 0) {
        JS_ReportError(cx, "Failed to write : %s",
                       (ret < 0 ? "Not opened or file is a directory" : strerror(err)));
        return false;
    }

    return true;
}

bool JSFile::JS_closeSync(JSContext *cx, JS::CallArgs &args)
{
    if (!this->allowSyncStream()) {
        JS_ReportError(cx, "Can't close this file synchronously");
        return false;
    }

    this->getFile()->closeSync();

    return true;
}

JSFile *JSFile::Constructor(JSContext *cx,
    JS::CallArgs &args, JS::HandleObject obj)
{
    File *file;
    JSFile *jsfile;

    JS::RootedString url(cx);
    JS::RootedObject opt(cx);

    if (!JS_ConvertArguments(cx, args, "S/o", url.address(), opt.address())) {
        return nullptr;
    }

    JSAutoByteString curl(cx, url);

    jsfile = new JSFile(curl.ptr());

    if (!jsfile->getPath() || !jsfile->allowSyncStream()) {
        JS_ReportError(cx, "FileIO : Invalid file path");
        return nullptr;
    }

    file = new File(jsfile->getPath());
    file->setListener(jsfile);
    file->setAutoClose(false);

    NIDIUM_JS_INIT_OPT();

    NIDIUM_JS_GET_OPT_TYPE(opt, "encoding", String)
    {
        JSAutoByteString encoding(cx, __curopt.toString());
        jsfile->m_Encoding = strdup(encoding.ptr());
    }

    jsfile->setFile(file);

    return jsfile;
}

JSPropertySpec *JSFile::ListProperties()
{
    static JSPropertySpec props[] = {
        CLASSMAPPER_PROP_G(JSFile, filesize),
        CLASSMAPPER_PROP_G(JSFile, filename),
        JS_PS_END
    };

    return props;
}

JSFunctionSpec *JSFile::ListMethods()
{
    static JSFunctionSpec funcs[] = {
        CLASSMAPPER_FN(JSFile, open, 2),
        CLASSMAPPER_FN(JSFile, openSync, 1),
        CLASSMAPPER_FN(JSFile, read, 2),
        CLASSMAPPER_FN(JSFile, readSync, 0),
        CLASSMAPPER_FN(JSFile, seek, 2),
        CLASSMAPPER_FN(JSFile, seekSync, 1),
        CLASSMAPPER_FN(JSFile, close, 0),
        CLASSMAPPER_FN(JSFile, closeSync, 0),
        CLASSMAPPER_FN(JSFile, write, 2),
        CLASSMAPPER_FN(JSFile, writeSync, 1),

        CLASSMAPPER_FN(JSFile, isDir, 0),
        CLASSMAPPER_FN(JSFile, listFiles, 1),

        CLASSMAPPER_FN(JSFile, rm, 0),
        CLASSMAPPER_FN(JSFile, rmrf, 0),
        JS_FS_END
    };

    return funcs;
}

JSFunctionSpec *JSFile::ListStaticMethods()
{
    static JSFunctionSpec funcs[] = {
        CLASSMAPPER_FN_STATIC(JSFile, read, 2),
        CLASSMAPPER_FN_STATIC(JSFile, readSync, 1),
        JS_FS_END
    };

    return funcs;
}

void JSFile::RegisterObject(JSContext *cx)
{
    JSFile::ExposeClass<1>(cx, "File");
}



} // namespace Binding
} // namespace Nidium
