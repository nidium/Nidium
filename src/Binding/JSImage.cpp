/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "Binding/JSImage.h"

#include <stdio.h>

#include <ape_netlib.h>

#include "Binding/JSFile.h"
#include "Binding/JSUtils.h"

#include "Graphics/Image.h"

using Nidium::Core::SharedMessages;
using Nidium::Core::Path;
using Nidium::Graphics::Image;
using Nidium::IO::Stream;
using Nidium::IO::File;

namespace Nidium {
namespace Binding {

bool JSImage::JSObjectIs(JSContext *cx, JS::HandleObject obj)
{
    return JSImage::InstanceOf(obj);
}

Image *JSImage::JSObjectToImage(JS::HandleObject obj)
{
    JSImage *jsimage = GetInstance(obj);
    if (jsimage == nullptr) {
        return nullptr;
    }

    return jsimage->m_Image;
}

static int delete_stream(void *arg)
{
    Stream *stream = (Stream *)arg;

    delete stream;

    return 0;
}

void JSImage::onMessage(const SharedMessages::Message &msg)
{

    switch (msg.event()) {
        case Stream::kEvents_Error: {
            JS::RootedObject eventObj(m_Cx, JSEvents::CreateEventObject(m_Cx));
            JS::RootedValue eventValue(m_Cx);
            JSObjectBuilder obj(m_Cx, eventObj);
            char err[128];

            eventValue.setObjectOrNull(eventObj);

            snprintf(err, 128, "Stream error. Code : %d\n",
                     msg.m_Args[0].toInt());
            obj.set("error", err);

            this->fireJSEvent("error", &eventValue);

            // XXX : Should we delete the stream here ?

            break;
        }
        case Stream::kEvents_ReadBuffer: {
            ape_global *ape = (ape_global *)JS_GetContextPrivate(m_Cx);
            JS::RootedObject eventObj(m_Cx, JSEvents::CreateEventObject(m_Cx));
            JS::RootedValue eventValue(m_Cx);
            JSObjectBuilder obj(m_Cx, eventObj);

            eventValue.setObjectOrNull(eventObj);

            if (this->setupWithBuffer((buffer *)msg.m_Args[0].toPtr())) {
                this->fireJSEvent("load", &eventValue);
            } else {
                obj.set("error", "Invalid data");
                this->fireJSEvent("error", &eventValue);
            }

            timer_dispatch_async(delete_stream, m_Stream);
            m_Stream = NULL;

            break;
        }
    }
}

bool JSImage::setupWithBuffer(buffer *buf)
{
    if (buf->used == 0) {
        this->unroot();

        return false;
    }

    Image *ImageObject = new Image(buf->data, buf->used);
    if (ImageObject->m_Image == NULL) {
        delete ImageObject;
        this->unroot();

        return false;
    }

    m_Image = ImageObject;
    JS::RootedObject obj(m_Cx, m_Instance);
    JS::RootedValue widthVal(m_Cx, INT_TO_JSVAL(ImageObject->getWidth()));
    JS::RootedValue heightVal(m_Cx, INT_TO_JSVAL(ImageObject->getHeight()));
    JS_DefineProperty(m_Cx, obj, "width", widthVal,
                      JSPROP_PERMANENT | JSPROP_READONLY);
    JS_DefineProperty(m_Cx, obj, "height", heightVal,
                      JSPROP_PERMANENT | JSPROP_READONLY);

    this->unroot();

    return true;
}


JSObject *
JSImage::BuildImageObject(JSContext *cx, Image *image, const char name[])
{
    JSImage *nimg = new JSImage();

    nimg->m_Image = image;

    JS::RootedObject ret(cx, JSImage::CreateObject(cx, nimg));

    JS::RootedValue widthVal(cx, INT_TO_JSVAL(image->getWidth()));
    JS::RootedValue heightVal(cx, INT_TO_JSVAL(image->getHeight()));
    JS::RootedString jstr(cx, JS_NewStringCopyZ(cx, (name ? name : "unknown")));
    JS::RootedValue nameVal(cx, STRING_TO_JSVAL(jstr));

    JS_DefineProperty(cx, ret, "width", widthVal,
                      JSPROP_PERMANENT | JSPROP_READONLY);
    JS_DefineProperty(cx, ret, "height", heightVal,
                      JSPROP_PERMANENT | JSPROP_READONLY);
    JS_DefineProperty(cx, ret, "src", nameVal,
                      JSPROP_PERMANENT | JSPROP_READONLY);

    return ret;
}

bool JSImage::JSSetter_src(JSContext *cx, JS::MutableHandleValue vp)
{
    if (vp.isString()) {
        JS::RootedString vpStr(cx, JS::ToString(cx, vp));
        JSAutoByteString imgPath(cx, vpStr);

        this->root();

        Stream *stream = Stream::Create(Path(imgPath.ptr()));

        if (stream == NULL) {
            JS_ReportError(cx, "Invalid path");
            return false;
        }

        m_Stream = stream;

        stream->setListener(this);
        stream->getContent();
    } else if (vp.isObject()) {
        JS::RootedObject fileObj(cx, vp.toObjectOrNull());
        JSFile *jsfile = JSFile::GetInstance(fileObj);
        File *file = jsfile ? jsfile->getFile() : nullptr;

        if (!file) {
            vp.setNull();
            return true;
        }

        jsfile->root();

        Stream *stream = Stream::Create(file->getFullPath());
        if (stream == NULL) {
            return true;
        }
        m_Stream = stream;
        stream->setListener(this);
        stream->getContent();

    } else {
        vp.setNull();
        return true;
    }
    return true;
}

bool JSImage::JSGetter_src(JSContext *cx, JS::MutableHandleValue vp)
{


    return true;
}

JSImage::JSImage()
    : m_Image(NULL), m_Stream(NULL)
{
}

JSImage::~JSImage()
{
    if (m_Image != NULL) {
        delete m_Image;
    }
    if (m_Stream) {
        m_Stream->setListener(NULL);
        delete m_Stream;
    }
}


JSPropertySpec *JSImage::ListProperties()
{
    static JSPropertySpec props[] = {
        CLASSMAPPER_PROP_GS(JSImage, src),
        JS_PS_END
    };

    return props;
}


JSImage *JSImage::Constructor(JSContext *cx, JS::CallArgs &args,
        JS::HandleObject obj)
{
    return new JSImage();
}

void JSImage::RegisterObject(JSContext *cx)
{
    JSImage::ExposeClass(cx, "Image");
}

} // namespace Binding
} // namespace Nidium
