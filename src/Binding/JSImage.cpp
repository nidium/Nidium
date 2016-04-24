#include "Binding/JSImage.h"

#include <stdio.h>

#include <ape_netlib.h>

#include <Binding/JSFileIO.h>

#include "Graphics/SkImage.h"

namespace Nidium {
namespace Binding {

// {{{ Preamble
#define IMAGE_RESERVED_SLOT 0

enum {
    IMAGE_PROP_SRC = IMAGE_RESERVED_SLOT,
    IMAGE_NPROP
};

#define NATIVE_IMAGE_GETTER(obj) (static_cast<class NativeJSImage *>(JS_GetPrivate(obj)))
#define IMAGE_FROM_CALLEE(nimg) \
    JS::RootedObject parent(cx, JS_GetParent(&args.callee())); \
    NativeJSImage *nimg = static_cast<NativeJSImage *>(JS_GetPrivate(parent));

static void Image_Finalize(JSFreeOp *fop, JSObject *obj);
static bool native_image_shiftHue(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_image_markColorInAlpha(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_image_desaturate(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_image_print(JSContext *cx, unsigned argc, JS::Value *vp);

static bool native_image_prop_set(JSContext *cx, JS::HandleObject obj, uint8_t id,
    bool strict, JS::MutableHandleValue vp);

static JSClass Image_class = {
    "Image", JSCLASS_HAS_PRIVATE | JSCLASS_HAS_RESERVED_SLOTS(IMAGE_NPROP+1),
    JS_PropertyStub, JS_DeletePropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, Image_Finalize,
    nullptr, nullptr, nullptr, nullptr, JSCLASS_NO_INTERNAL_MEMBERS
};

template<>
JSClass *JSExposer<NativeJSImage>::jsclass = &Image_class;

static JSPropertySpec Image_props[] = {
    NIDIUM_JS_PSS("src", IMAGE_PROP_SRC, native_image_prop_set),
    JS_PS_END
};

static JSFunctionSpec Image_funcs[] = {
    JS_FN("shiftHue", native_image_shiftHue, 2, NIDIUM_JS_FNPROPS),
    JS_FN("markColorInAlpha", native_image_markColorInAlpha, 0, NIDIUM_JS_FNPROPS),
    JS_FN("desaturate", native_image_desaturate, 0, NIDIUM_JS_FNPROPS),
    JS_FN("print", native_image_print, 0, NIDIUM_JS_FNPROPS),
    JS_FS_END
};
// }}}

// {{{ Implementation
static bool native_image_print(JSContext *cx, unsigned argc, JS::Value *vp)
{
    return true;
}

static bool native_image_shiftHue(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    int val;
    int color;
    IMAGE_FROM_CALLEE(nimg);

    if (!JS_ConvertArguments(cx, args, "ii", &val, &color)) {
        return false;
    }

    if (nimg->m_Image) {
        nimg->m_Image->shiftHue(val, color);
    }
    return true;
}

static bool native_image_markColorInAlpha(JSContext *cx,
    unsigned argc, JS::Value *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    IMAGE_FROM_CALLEE(nimg);
    if (nimg->m_Image) {
        nimg->m_Image->markColorsInAlpha();
    }

    return true;
}

static bool native_image_desaturate(JSContext *cx,
    unsigned argc, JS::Value *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    IMAGE_FROM_CALLEE(nimg);
    if (nimg->m_Image) {
        nimg->m_Image->desaturate();
    }

    return true;
}

static bool native_image_prop_set(JSContext *cx, JS::HandleObject obj,
    uint8_t id, bool strict, JS::MutableHandleValue vp)
{
    NativeJSImage *nimg = NATIVE_IMAGE_GETTER(obj);
    switch(id) {
        case IMAGE_PROP_SRC:
        {
            if (vp.isString()) {
                JS::RootedString vpStr(cx, JS::ToString(cx, vp));
                JSAutoByteString imgPath(cx, vpStr);

                NidiumJSObj(cx)->rootObjectUntilShutdown(obj);

                IO::Stream *stream = IO::Stream::create(Core::Path(imgPath.ptr()));

                if (stream == NULL) {
                    JS_ReportError(cx, "Invalid path");
                    return false;
                }

                nimg->m_Stream = stream;

                stream->setListener(nimg);
                stream->getContent();
            } else if (vp.isObject()) {
                JS::RootedObject obj(cx, vp.toObjectOrNull());
                IO::File *file = JSFileIO::GetFileFromJSObject(cx, obj);
                if (!file) {
                    vp.setNull();
                    return true;
                }

                NidiumJSObj(cx)->rootObjectUntilShutdown(obj);

                IO::Stream *stream = IO::Stream::create(file->getFullPath());
                if (stream == NULL) {
                    break;
                }
                nimg->m_Stream = stream;
                stream->setListener(nimg);
                stream->getContent();

            } else {
                vp.setNull();
                return true;
            }
        }
        break;
        default:
            break;
    }
    return true;
}

void Image_Finalize(JSFreeOp *fop, JSObject *obj)
{
    NativeJSImage *img = NATIVE_IMAGE_GETTER(obj);
    if (img != NULL) {
        if (img->m_Stream) {
            img->m_Stream->setListener(NULL);
        }

        delete img;
    }
}

static bool native_Image_constructor(JSContext *cx, unsigned argc, JS::Value *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    NativeJSImage *nimg;

    if (!args.isConstructing()) {
        JS_ReportError(cx, "Bad constructor");
        return false;
    }

    JS::RootedObject ret(cx, JS_NewObjectForConstructor(cx, &Image_class, args));
    nimg = new NativeJSImage(ret, cx);
    JS_SetPrivate(ret, nimg);
    JS_DefineProperties(cx, ret, Image_props);
    JS_DefineFunctions(cx, ret, Image_funcs);

    args.rval().setObjectOrNull(ret);

    return true;
}

bool NativeJSImage::JSObjectIs(JSContext *cx, JS::HandleObject obj)
{
    return obj && JS_GetClass(obj) == &Image_class;
}

Graphics::NativeSkImage *NativeJSImage::JSObjectToNativeSkImage(JS::HandleObject obj)
{
    return NATIVE_IMAGE_GETTER(obj)->m_Image;
}

static int delete_stream(void *arg)
{
    IO::Stream *stream = (IO::Stream *)arg;

    delete stream;

    return 0;
}

void NativeJSImage::onMessage(const Core::SharedMessages::Message &msg)
{
    ape_global *ape = (ape_global *)JS_GetContextPrivate(m_Cx);

    switch (msg.event()) {
        case IO::Stream::EVENT_READ_BUFFER:
        {
            JS::RootedValue onload_callback(m_Cx);
            JS::RootedObject obj(m_Cx, m_JSObject);
            if (this->setupWithBuffer((buffer *)msg.args[0].toPtr())) {
                if (JS_GetProperty(m_Cx, obj, "onload", &onload_callback) &&
                    JS_TypeOfValue(m_Cx, onload_callback) == JSTYPE_FUNCTION) {

                    JS::RootedValue rval(m_Cx);
                    JS_CallFunctionValue(m_Cx, obj, onload_callback, JS::HandleValueArray::empty(), &rval);
                }
            }

            timer_dispatch_async(delete_stream, m_Stream);
            m_Stream = NULL;
            break;
        }
    }
}

bool NativeJSImage::setupWithBuffer(buffer *buf)
{
    if (buf->used == 0) {

        NidiumJSObj(m_Cx)->unrootObject(m_JSObject);
        return false;
    }

    Graphics::NativeSkImage *ImageObject = new Graphics::NativeSkImage(buf->data, buf->used);
    if (ImageObject->m_Image == NULL) {
        delete ImageObject;

        NidiumJSObj(m_Cx)->unrootObject(m_JSObject);
        return false;
    }

    m_Image = ImageObject;
    JS::RootedObject obj(m_Cx, m_JSObject);
    JS::RootedValue widthVal(m_Cx, INT_TO_JSVAL(ImageObject->getWidth()));
    JS::RootedValue heightVal(m_Cx, INT_TO_JSVAL(ImageObject->getHeight()));
    JS_DefineProperty(m_Cx, obj, "width", widthVal, JSPROP_PERMANENT | JSPROP_READONLY);
    JS_DefineProperty(m_Cx, obj, "height", heightVal, JSPROP_PERMANENT | JSPROP_READONLY);

    NidiumJSObj(m_Cx)->unrootObject(m_JSObject);

    return true;
}

#if 0
void NativeJSImage::onGetContent(const char *data, size_t len)
{
    ape_global *ape = (ape_global *)JS_GetContextPrivate(cx);

    if (data == NULL || len == 0) {
        timer_dispatch_async(delete_stream, stream);
        stream = NULL;
        return;
    }

    Graphics::NativeSkImage *ImageObject = new Graphics::NativeSkImage((void *)data, len);
    if (ImageObject->m_Image == NULL) {
        timer_dispatch_async(delete_stream, stream);
        stream = NULL;
        delete ImageObject;
        return;
    }
    m_Image = ImageObject;
    JS::RootedValue widthVal(cx, INT_TO_JSVAL(ImageObject->getWidth()));
    JS::RootedValue heightVal(cx, INT_TO_JSVAL(ImageObject->getHeight()));
    JS_DefineProperty(cx, jsobj, "width", widthVal, JSPROP_PERMANENT | JSPROP_READONLY);

    JS_DefineProperty(cx, jsobj, "height", heightVal, JSPROP_PERMANENT | JSPROP_READONLY);

    JS::RootedValue onload_callback(cx);
    if (JS_GetProperty(cx, jsobj, "onload", &onload_callback) &&
        JS_TypeOfValue(cx, onload_callback) == JSTYPE_FUNCTION) {

        JS::RootedValue rval(cx);
        JS_CallFunctionValue(cx, jsobj, onload_callback, JS::HandleValueArray::empty(), rval.address());
    }

    NidiumJSObj(cx)->unrootObject(jsobj);
    timer_dispatch_async(delete_stream, stream);
    stream = NULL;
}
#endif

JSObject *NativeJSImage::buildImageObject(JSContext *cx, Graphics::NativeSkImage *image,
    const char name[])
{
    JS::RootedValue proto(cx);
    JS::RootedObject global(cx, JS::CurrentGlobalOrNull(cx));
    JS_GetProperty(cx, global, "Image", &proto);
    JS::RootedObject protoObj(cx);
    protoObj.set(&proto.toObject());
    JS::RootedObject ret(cx, JS_NewObject(cx, &Image_class, protoObj, JS::NullPtr()));
    NativeJSImage *nimg = new NativeJSImage(ret, cx);

    nimg->m_Image   = image;

    JS_SetPrivate(ret, nimg);

    printf("Build image object\n");
    JS::RootedValue widthVal(cx, INT_TO_JSVAL(image->getWidth()));
    JS::RootedValue heightVal(cx, INT_TO_JSVAL(image->getHeight()));
    JS::RootedString jstr(cx,  JS_NewStringCopyZ(cx, (name ? name : "unknown")));
    JS::RootedValue nameVal(cx, STRING_TO_JSVAL(jstr));
    JS_DefineProperty(cx, ret, "width", widthVal, JSPROP_PERMANENT | JSPROP_READONLY);
    JS_DefineProperty(cx, ret, "height", heightVal, JSPROP_PERMANENT | JSPROP_READONLY);
    JS_DefineProperty(cx, ret, "src", nameVal, JSPROP_PERMANENT | JSPROP_READONLY);

    return ret;
}

NativeJSImage::NativeJSImage(JS::HandleObject obj, JSContext *cx) :
    JSExposer<NativeJSImage>(obj, cx),
    m_Image(NULL), m_Stream(NULL)
{

}

NativeJSImage::~NativeJSImage()
{
    if (m_Image != NULL) {
        delete m_Image;
    }
    if (m_Stream) {
        delete m_Stream;
    }
}
// }}}

// {{{ Registration
void NativeJSImage::registerObject(JSContext *cx)
{
    JS::RootedObject global(cx, JS::CurrentGlobalOrNull(cx));
    JS_InitClass(cx, global, JS::NullPtr(), &Image_class,
        native_Image_constructor,
        0, nullptr, Image_funcs, nullptr, nullptr);
}

//NATIVE_OBJECT_EXPOSE(Image)

// }}}
} // namespace Nidium
} // namespace Binding
