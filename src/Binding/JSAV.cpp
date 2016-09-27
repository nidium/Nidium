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
#include "Macros.h"

using namespace Nidium::AV;

extern "C" {
#include <libavformat/avformat.h>
}

namespace Nidium {
namespace Binding {


// TODO : Need to handle nodes GC, similar to
//        https://dvcs.w3.org/hg/audio/raw-file/tip/webaudio/specification.html#lifetime-AudioNode
// TODO : Need to handle video GC
// TODO : When stop/pause/kill fade out sound



extern void
reportError(JSContext *cx, const char *message, JSErrorReport *report);

// {{{ Implementation
bool JSTransferableFunction::prepare(JSContext *cx, JS::HandleValue val)
{
    if (!JS_WriteStructuredClone(cx, val, &m_Data, &m_Bytes, nullptr, nullptr,
                                 JS::NullHandleValue)) {
        return false;
    }

    return true;
}

bool JSTransferableFunction::call(JS::HandleObject obj,
                                  JS::HandleValueArray params,
                                  JS::MutableHandleValue rval)
{
    if (m_Data != NULL) {
        if (!this->transfert()) {
            return false;
        }
    }

    JS::RootedValue fun(m_DestCx, m_Fn.get());

    return JS_CallFunctionValue(m_DestCx, obj, fun, params, rval);
}

bool JSTransferableFunction::transfert()
{
    JS::RootedValue fun(m_DestCx);

    bool ok = JS_ReadStructuredClone(m_DestCx, m_Data, m_Bytes,
                                     JS_STRUCTURED_CLONE_VERSION, &fun, nullptr,
                                     NULL);

    JS_ClearStructuredClone(m_Data, m_Bytes, nullptr, NULL);

    m_Data  = NULL;
    m_Bytes = 0;
    m_Fn.set(fun);

    return ok;
}

JSTransferableFunction::~JSTransferableFunction()
{
    JSAutoRequest ar(m_DestCx);

    if (m_Data != NULL) {
        JS_ClearStructuredClone(m_Data, m_Bytes, nullptr, NULL);
    }

    JS::RootedValue fun(m_DestCx, m_Fn);
    if (!fun.isUndefined()) {
        fun.setUndefined();
    }
}

const char *JSAVEventRead(int ev)
{
    switch (ev) {
        case CUSTOM_SOURCE_SEND:
            return "message";
        case SOURCE_EVENT_PAUSE:
            return "pause";
            break;
        case SOURCE_EVENT_PLAY:
            return "play";
            break;
        case SOURCE_EVENT_STOP:
            return "stop";
            break;
        case SOURCE_EVENT_EOF:
            return "end";
            break;
        case SOURCE_EVENT_ERROR:
            return "error";
            break;
        case SOURCE_EVENT_BUFFERING:
            return "buffering";
            break;
        case SOURCE_EVENT_READY:
            return "ready";
            break;
        default:
            return NULL;
            break;
    }
}

void CopyMetaDataToJS(AVDictionary *dict, JSContext *cx, JS::HandleObject obj)
{
    AVDictionaryEntry *tag = NULL;

    if (!dict) return;

    while ((tag = av_dict_get(dict, "", tag, AV_DICT_IGNORE_SUFFIX))) {
        JS::RootedString val(
            cx, JS_NewStringCopyN(cx, tag->value, strlen(tag->value)));
        JS::RootedValue value(cx, STRING_TO_JSVAL(val));
        JS_DefineProperty(cx, obj, tag->key, value, JSPROP_ENUMERATE
                                                        | JSPROP_READONLY
                                                        | JSPROP_PERMANENT);
    }
}
// {{{ JSAVSource
bool JSAVSource::PropSetter(AVSource *source,
                            uint8_t id,
                            JS::MutableHandleValue vp)
{
    switch (id) {
        case SOURCE_PROP_POSITION:
            if (vp.isNumber()) {
                source->seek(vp.toNumber());
            }
            break;
        default:
            break;
    }

    return true;
}

bool JSAVSource::PropGetter(AVSource *source,
                            JSContext *cx,
                            uint8_t id,
                            JS::MutableHandleValue vp)
{
    switch (id) {
        case SOURCE_PROP_POSITION:
            vp.setDouble(source->getClock());
            break;
        case SOURCE_PROP_DURATION:
            vp.setDouble(source->getDuration());
            break;
        case SOURCE_PROP_BITRATE:
            vp.setInt32(source->getBitrate());
            break;
        case SOURCE_PROP_METADATA: {
            AVFormatContext *avctx = source->getAVFormatContext();

            if (avctx != NULL) {
                JS::RootedObject metadata(
                    cx,
                    JS_NewObject(cx, nullptr, JS::NullPtr(), JS::NullPtr()));
                AVDictionary *cmetadata = avctx->metadata;

                if (cmetadata) {
                    CopyMetaDataToJS(cmetadata, cx, metadata);
                }

                JS::RootedObject arr(cx, JS_NewArrayObject(cx, 0));
                JS_DefineProperty(cx, metadata, "streams", arr,
                                  JSPROP_ENUMERATE | JSPROP_READONLY
                                      | JSPROP_PERMANENT);

                for (int i = 0; i < avctx->nb_streams; i++) {
                    JS::RootedObject streamMetaData(
                        cx, JS_NewObject(cx, nullptr, JS::NullPtr(),
                                         JS::NullPtr()));

                    CopyMetaDataToJS(avctx->streams[i]->metadata, cx,
                                     streamMetaData);

                    JS_DefineElement(cx, arr, i,
                                     OBJECT_TO_JSVAL(streamMetaData), nullptr,
                                     nullptr, 0);
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
                nullptr, JS::NullPtr(),
                JS::NullPtr()));

                        CopyMetaDataToJS(avctx->programs[i]->metadata, cx,
                progMetaData);

                        JS_DefineElement(cx, arr, i,
                OBJECT_TO_JSVAL(progMetaData), nullptr,
                nullptr, 0);
                    }
                }
                */

                vp.setObject(*metadata);
            } else {
                vp.setUndefined();
            }
        } break;
        default:
            vp.setUndefined();
            break;
    }

    return true;
}
// }}}

// }}}

} // namespace Binding
} // namespace Nidium
