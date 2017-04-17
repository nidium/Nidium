/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "Binding/JSAV.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "Frontend/Context.h"
#include "Binding/JSConsole.h"
#include "Binding/JSUtils.h"
#include "Binding/JSAudioContext.h"
#include "Binding/ClassMapper.h"

using namespace Nidium::AV;

extern "C" {
#include <libavformat/avformat.h>
}

namespace Nidium {
namespace Binding {


// TODO : Need to handle nodes GC, similar to
//        https://dvcs.w3.org/hg/audio/raw-file/tip/webaudio/specification.html#lifetime-AudioNode
// TODO : When stop/pause/kill fade out sound

// {{{ JSAVSourceEventInterface
void JSAVSourceEventInterface::listenSourceEvents(
    AV::AVSourceEventInterface *eventInterface)
{
    eventInterface->eventCallback(JSAVSourceEventInterface::onEvent, this);
}

void JSAVSourceEventInterface::onEvent(const struct AVSourceEvent *cev)
{
    JSAVSourceEventInterface *source
        = static_cast<JSAVSourceEventInterface *>(cev->m_Custom);
    source->postMessage((void *)cev, cev->m_Ev);
}

void JSAVSourceEventInterface::onMessage(const SharedMessages::Message &msg)
{
    if (this->isReleased()) return;

    const char *evName;
    JSContext *cx = this->getJSContext();

    this->onSourceMessage(msg);

    switch (msg.event()) {
        case SOURCE_EVENT_PAUSE:
            evName = "pause";
            break;
        case SOURCE_EVENT_PLAY:
            evName = "play";
            break;
        case SOURCE_EVENT_STOP:
            evName = "stop";
            break;
        case SOURCE_EVENT_EOF:
            evName = "end";
            break;
        case SOURCE_EVENT_ERROR:
            evName = "error";
            break;
        case SOURCE_EVENT_BUFFERING:
            evName = "buffering";
            break;
        case SOURCE_EVENT_READY:
            evName = "ready";
            break;
        default:
            return;
    }

    JS::RootedValue ev(cx);
    AVSourceEvent *cmsg = static_cast<struct AVSourceEvent *>(msg.dataPtr());

    if (cmsg->m_Ev == SOURCE_EVENT_ERROR) {
        int errorCode        = cmsg->m_Args[0].toInt();
        const char *errorStr = AV::AVErrorsStr[errorCode];
        JS::RootedObject evObj(
            cx, JSEvents::CreateErrorEventObject(cx, errorCode, errorStr));
        ev.setObject(*evObj);
    } else if (cmsg->m_Ev == SOURCE_EVENT_BUFFERING) {
        JS::RootedObject evObj(cx, JSEvents::CreateEventObject(cx));
        JSObjectBuilder evBuilder(cx, evObj);

        evBuilder.set("filesize", cmsg->m_Args[0].toInt());
        evBuilder.set("startByte", cmsg->m_Args[1].toInt());
        evBuilder.set("bufferedBytes", cmsg->m_Args[2].toInt());

        ev = evBuilder.jsval();
    }

    delete cmsg;

    if (!ev.isNull()) {
        this->fireJSEvent(evName, &ev);
    }
}
// }}}

// {{{ JSAVSourceBase
void CopyMetaDataToJS(AVDictionary *dict, JSContext *cx, JS::HandleObject obj)
{
    AVDictionaryEntry *tag = NULL;

    if (!dict) return;

    while ((tag = av_dict_get(dict, "", tag, AV_DICT_IGNORE_SUFFIX))) {
        JS::RootedString val(
            cx, JS_NewStringCopyN(cx, tag->value, strlen(tag->value)));
        JS::RootedValue value(cx, JS::StringValue(val));
        JS_DefineProperty(cx, obj, tag->key, value, JSPROP_ENUMERATE
                                                        | JSPROP_READONLY
                                                        | JSPROP_PERMANENT);
    }
}

bool JSAVSourceBase::JS__open(JSContext *cx, JS::CallArgs &args)
{
    AVSource *source = this->getSource();

    JS::RootedValue src(cx, args[0]);

    int ret = -1;

    if (src.isString()) {
        JSAutoByteString csrc(cx, src.toString());
        ret = source->open(csrc.ptr());
    } else if (src.isObject()) {
        JS::RootedObject arrayBuff(cx, src.toObjectOrNull());
        if (!JS_IsArrayBufferObject(arrayBuff)) {
            JS_ReportError(cx, "Data is not an ArrayBuffer\n");
            return false;
        }
        int length           = JS_GetArrayBufferByteLength(arrayBuff);
        this->m_ArrayContent = JS_StealArrayBufferContents(cx, arrayBuff);
        ret                  = source->open(m_ArrayContent, length);
    } else {
        JS_ReportError(cx, "Invalid argument", ret);
        return false;
    }

    if (ret != 0) {
        JS_ReportError(cx, "Failed to open stream %d\n", ret);
        return false;
    }

    return true;
}

bool JSAVSourceBase::JS__play(JSContext *cx, JS::CallArgs &args)
{
    this->getSource()->play();

    // play() may call the JS "onready" callback in a synchronous way
    // thus, if an exception happen in the callback, we should return false
    return !JS_IsExceptionPending(cx);
}

bool JSAVSourceBase::JS__pause(JSContext *cx, JS::CallArgs &args)
{
    this->getSource()->pause();

    return !JS_IsExceptionPending(cx);
}

bool JSAVSourceBase::JS__stop(JSContext *cx, JS::CallArgs &args)
{
    this->getSource()->stop();

    return !JS_IsExceptionPending(cx);
}

bool JSAVSourceBase::JS__close(JSContext *cx, JS::CallArgs &args)
{
    this->getSource()->close();

    return !JS_IsExceptionPending(cx);
}

bool JSAVSourceBase::JSGetter__position(JSContext *cx,
                                        JS::MutableHandleValue vp)
{
    vp.setDouble(this->getSource()->getClock());

    return true;
}

bool JSAVSourceBase::JSSetter__position(JSContext *cx,
                                        JS::MutableHandleValue vp)
{
    if (vp.isNumber()) {
        this->getSource()->seek(vp.toNumber());
    }

    return true;
}

bool JSAVSourceBase::JSGetter__duration(JSContext *cx,
                                        JS::MutableHandleValue vp)
{
    vp.setDouble(this->getSource()->getDuration());
    return true;
}

bool JSAVSourceBase::JSGetter__metadata(JSContext *cx,
                                        JS::MutableHandleValue vp)
{
    AVFormatContext *avctx = this->getSource()->getAVFormatContext();

    if (avctx != NULL) {
        AVDictionary *cmetadata = avctx->metadata;
        JS::RootedObject metadata(cx, JS_NewPlainObject(cx));

        if (cmetadata) {
            CopyMetaDataToJS(cmetadata, cx, metadata);
        }

        JS::RootedObject arr(cx, JS_NewArrayObject(cx, 0));
        JS_DefineProperty(cx, metadata, "streams", arr, JSPROP_ENUMERATE
                                                            | JSPROP_READONLY
                                                            | JSPROP_PERMANENT);

        for (int i = 0; i < avctx->nb_streams; i++) {
            JS::RootedObject streamMetaData(cx, JS_NewPlainObject(cx));

            CopyMetaDataToJS(avctx->streams[i]->metadata, cx, streamMetaData);

            JS_DefineElement(cx, arr, i, streamMetaData, 0);
        }

        /*
        for (int i = 0; i < avctx->nb_chapters; i++) {
            // TODO
        }
        */

        // XXX : Not tested
        /*
        if (avctx->nb_programs) {
            JS::RootedObject arr(cx, JS_NewArrayObject(cx, 0));
            JS_DefineProperty(cx, metadata, "programs", arr,
        JSPROP_ENUMERATE|JSPROP_READONLY|JSPROP_PERMANENT);

            for (int i = 0; i < avctx->nb_programs; i++) {
                JS::RootedObject progMetaData(cx, JS_NewObject(cx,
        nullptr, nullptr,
        nullptr));

                CopyMetaDataToJS(avctx->programs[i]->metadata, cx,
        progMetaData);

                JS_DefineElement(cx, arr, i,
        JS::ObjectValue(*progMetaData), nullptr,
        nullptr, 0);
            }
        }
        */

        vp.setObjectOrNull(metadata);
    } else {
        vp.setUndefined();
    }

    return true;
}

bool JSAVSourceBase::JSGetter__bitrate(JSContext *cx, JS::MutableHandleValue vp)
{
    vp.setInt32(this->getSource()->getBitrate());

    return true;
}
// }}}

} // namespace Binding
} // namespace Nidium
