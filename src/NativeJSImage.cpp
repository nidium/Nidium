#include "NativeJSImage.h"
#include "NativeSkImage.h"
#include "NativeJS.h"
#include "NativeSkia.h"
#include <string.h>
#include <native_netlib.h>
#include <NativeFile.h>
#include <NativeJSFileIO.h>

JSObject *NativeJSImage::classe = nullptr;

#define NATIVE_IMAGE_GETTER(obj) ((class NativeJSImage *)JS_GetPrivate(obj))
#define IMAGE_FROM_CALLEE(nimg) \
    JS::RootedObject parent(cx, JS_GetParent(&args.callee())); \
    NativeJSImage *nimg = (NativeJSImage *) JS_GetPrivate(parent);

static void Image_Finalize(JSFreeOp *fop, JSObject *obj);
static bool native_image_shiftHue(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_image_markColorInAlpha(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_image_desaturate(JSContext *cx, unsigned argc, JS::Value *vp);
static bool native_image_print(JSContext *cx, unsigned argc, JS::Value *vp);

static bool native_image_prop_set(JSContext *cx, JS::HandleObject obj, uint8_t id,
    bool strict, JS::MutableHandleValue vp);

extern JSClass File_class;

static JSClass Image_class = {
    "Image", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_DeletePropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, Image_Finalize,
    nullptr, nullptr, nullptr, nullptr, JSCLASS_NO_INTERNAL_MEMBERS
};

template<>
JSClass *NativeJSExposer<NativeJSImage>::jsclass = &Image_class;

static JSPropertySpec Image_props[] = {
    {"src", JSPROP_PERMANENT | JSPROP_ENUMERATE | JSPROP_SHARED | JSPROP_NATIVE_ACCESSORS | JSPROP_READONLY,
        NATIVE_JS_STUBGETTER(),
        NATIVE_JS_SETTER(IMAGE_PROP_SRC, native_image_prop_set)},
    JS_PS_END
};

static JSFunctionSpec Image_funcs[] = {
    JS_FN("shiftHue", native_image_shiftHue, 2, 0),
    JS_FN("markColorInAlpha", native_image_markColorInAlpha, 0, 0),
    JS_FN("desaturate", native_image_desaturate, 0, 0),
    JS_FN("print", native_image_print, 0, 0),
    JS_FS_END
};

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

    if (nimg->img) {
        nimg->img->shiftHue(val, color);
    }
    return true;
}

static bool native_image_markColorInAlpha(JSContext *cx,
    unsigned argc, JS::Value *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    IMAGE_FROM_CALLEE(nimg);
    if (nimg->img) {
        nimg->img->markColorsInAlpha();
    }

    return true;
}

static bool native_image_desaturate(JSContext *cx,
    unsigned argc, JS::Value *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    IMAGE_FROM_CALLEE(nimg);
    if (nimg->img) {
        nimg->img->desaturate();
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
                JSAutoByteString imgPath(cx, vp.toString());

                NativeJSObj(cx)->rootObjectUntilShutdown(obj);

                NativeBaseStream *stream = NativeBaseStream::create(NativePath(imgPath.ptr()));

                if (stream == NULL) {
                    JS_ReportError(cx, "Invalid path");
                    return false;
                    break;
                }

                nimg->m_Stream = stream;

                stream->setListener(nimg);
                stream->getContent();
            } else if (vp.isObject()) {
                JS::RootedObject obj(cx, vp.toObjectOrNull());
                NativeFile *file = NativeJSFileIO::GetFileFromJSObject(cx, obj);
                if (!file) {
                    vp.set(JS::NullHandleValue);
                    return true;
                }

                NativeJSObj(cx)->rootObjectUntilShutdown(obj);

                NativeBaseStream *stream = NativeBaseStream::create(file->getFullPath());
                if (stream == NULL) {
                    break;
                }
                nimg->m_Stream = stream;
                stream->setListener(nimg);
                stream->getContent();

            } else {
                vp.set(JS::NullHandleValue);
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
    JS::RootedObject ret(cx, JS_NewObjectForConstructor(cx, &Image_class, args));
    NativeJSImage *nimg;

    if (!args.isConstructing()) {
        JS_ReportError(cx, "Bad constructor");
        return false;
    }

    nimg = new NativeJSImage(ret, cx);

    JS_SetPrivate(ret, nimg);

    args.rval().set(OBJECT_TO_JSVAL(ret));

    JS_DefineProperties(cx, ret, Image_props);
    JS_DefineFunctions(cx, ret, Image_funcs);

    return true;
}

bool NativeJSImage::JSObjectIs(JSContext *cx, JS::HandleObject obj)
{
    return JS_GetClass(obj) == &Image_class;
}

NativeSkImage *NativeJSImage::JSObjectToNativeSkImage(JS::HandleObject obj)
{
    return NATIVE_IMAGE_GETTER(obj)->img;
}

static int delete_stream(void *arg)
{
    NativeBaseStream *stream = (NativeBaseStream *)arg;

    delete stream;

    return 0;
}

void NativeJSImage::onMessage(const NativeSharedMessages::Message &msg)
{
    ape_global *ape = (ape_global *)JS_GetContextPrivate(m_Cx);

    switch (msg.event()) {
        case NATIVESTREAM_READ_BUFFER:
        {
            JS::RootedValue rval(m_Cx);
            JS::RootedValue onload_callback(m_Cx);
            JS::RootedObject obj(m_Cx, m_JSObject);
            if (this->setupWithBuffer((buffer *)msg.args[0].toPtr())) {
                if (JS_GetProperty(m_Cx, obj, "onload", &onload_callback) &&
                    JS_TypeOfValue(m_Cx, onload_callback) == JSTYPE_FUNCTION) {

                    JS_CallFunctionValue(m_Cx, obj, onload_callback,
                        JS::HandleValueArray::empty(), &rval);
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

        NativeJSObj(m_Cx)->unrootObject(m_JSObject);
        return false;
    }

    NativeSkImage *ImageObject = new NativeSkImage(buf->data, buf->used);
    if (ImageObject->img == NULL) {
        delete ImageObject;

        NativeJSObj(m_Cx)->unrootObject(m_JSObject);
        return false;
    }

    img = ImageObject;
    JS::RootedObject obj(m_Cx, m_JSObject);
    JS::RootedValue widthVal(m_Cx, INT_TO_JSVAL(ImageObject->getWidth()));
    JS::RootedValue heightVal(m_Cx, INT_TO_JSVAL(ImageObject->getHeight()));
    JS_DefineProperty(m_Cx, obj, "width", widthVal, JSPROP_PERMANENT | JSPROP_READONLY);
    JS_DefineProperty(m_Cx, obj, "height", heightVal, JSPROP_PERMANENT | JSPROP_READONLY);

    NativeJSObj(m_Cx)->unrootObject(m_JSObject);

    return true;
}

#if 0
void NativeJSImage::onGetContent(const char *data, size_t len)
{
    JS::RootedValue rval(cx);
    JS::RootedValue onload_callback(cx);
    ape_global *ape = (ape_global *)JS_GetContextPrivate(cx);

    if (data == NULL || len == 0) {
        timer_dispatch_async(delete_stream, stream);
        stream = NULL;
        return;
    }

    NativeSkImage *ImageObject = new NativeSkImage((void *)data, len);
    if (ImageObject->img == NULL) {
        timer_dispatch_async(delete_stream, stream);
        stream = NULL;
        delete ImageObject;
        return;
    }
    img = ImageObject;
    JS::RootedValue widthVal(cx, INT_TO_JSVAL(ImageObject->getWidth()));
    JS::RootedValue heightVal(cx, INT_TO_JSVAL(ImageObject->getHeight()));
    JS_DefineProperty(cx, jsobj, "width", widthVal, JSPROP_PERMANENT | JSPROP_READONLY);

    JS_DefineProperty(cx, jsobj, "height", heightVal, JSPROP_PERMANENT | JSPROP_READONLY);

    if (JS_GetProperty(cx, jsobj, "onload", &onload_callback) &&
        JS_TypeOfValue(cx, onload_callback) == JSTYPE_FUNCTION) {

        JS_CallFunctionValue(cx, jsobj, onload_callback,
            JS::HandleValueArray::empty(), &rval);
    }

    NativeJSObj(cx)->unrootObject(jsobj);
    timer_dispatch_async(delete_stream, stream);
    stream = NULL;
}
#endif

JSObject *NativeJSImage::buildImageObject(JSContext *cx, NativeSkImage *image,
    const char name[])
{
    JS::RootedValue proto(cx);
    JS::RootedObject global(cx, JS::CurrentGlobalOrNull(cx));
    JS_GetProperty(cx, global, "Image", &proto);
    JS::RootedObject protoObj(cx);
    protoObj.set(&proto.toObject());
    JS::RootedObject ret(cx, JS_NewObject(cx, &Image_class, protoObj, JS::NullPtr()));
    NativeJSImage *nimg = new NativeJSImage(ret, cx);

    nimg->img   = image;

    JS_SetPrivate(ret, nimg);

    printf("Build image object\n");
    JS::RootedValue widthVal(cx, INT_TO_JSVAL(image->getWidth()));
    JS::RootedValue heightVal(cx, INT_TO_JSVAL(image->getHeight()));
    JS::RootedValue nameStr(cx, STRING_TO_JSVAL(JS_NewStringCopyZ(cx, (name ? name : "unknown"))));
    JS_DefineProperty(cx, ret, "width", widthVal, JSPROP_PERMANENT | JSPROP_READONLY);
    JS_DefineProperty(cx, ret, "height", heightVal, JSPROP_PERMANENT | JSPROP_READONLY);
    JS_DefineProperty(cx, ret, "src", nameStr, JSPROP_PERMANENT | JSPROP_READONLY);

    return ret;
}

NativeJSImage::NativeJSImage(JS::HandleObject obj, JSContext *cx) :
    NativeJSExposer<NativeJSImage>(obj, cx),
    img(NULL), m_Stream(NULL)
{

}

NativeJSImage::~NativeJSImage()
{
    if (img != NULL) {
        delete img;
    }
    if (m_Stream) {
        delete m_Stream;
    }
}

void NativeJSImage::registerObject(JSContext *cx)
{
    JS::RootedObject global(cx, JS::CurrentGlobalOrNull(cx));
    JS_InitClass(cx, global, JS::NullPtr(), &Image_class,
        native_Image_constructor,
        0, nullptr, Image_funcs, nullptr, nullptr);
}

//NATIVE_OBJECT_EXPOSE(Image)

