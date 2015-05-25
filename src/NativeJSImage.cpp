#include "NativeJSImage.h"
#include "NativeSkImage.h"
#include "NativeJS.h"
#include "NativeSkia.h"
#include <string.h>
#include <native_netlib.h>
#include <NativeFile.h>
#include <NativeJSFileIO.h>

JSObject *NativeJSImage::classe = NULL;

#define NATIVE_IMAGE_GETTER(obj) ((class NativeJSImage *)JS_GetPrivate(obj))
#define IMAGE_FROM_CALLEE ((class NativeJSImage *)JS_GetPrivate(JS_GetParent(JSVAL_TO_OBJECT(JS_CALLEE(cx, vp)))))

static void Image_Finalize(JSFreeOp *fop, JSObject *obj);
static JSBool native_image_shiftHue(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_image_markColorInAlpha(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_image_desaturate(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_image_print(JSContext *cx, unsigned argc, jsval *vp);

static JSBool native_image_prop_set(JSContext *cx, JSHandleObject obj,
    JSHandleId id, JSBool strict, JSMutableHandleValue vp);

extern JSClass File_class;

static JSClass Image_class = {
    "Image", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, Image_Finalize,
    JSCLASS_NO_OPTIONAL_MEMBERS
};

template<>
JSClass *NativeJSExposer<NativeJSImage>::jsclass = &Image_class;


static JSPropertySpec Image_props[] = {
    {"src", IMAGE_PROP_SRC, 0, JSOP_NULLWRAPPER,
    	JSOP_WRAPPER(native_image_prop_set)},
    {0, 0, 0, JSOP_NULLWRAPPER, JSOP_NULLWRAPPER}
};

static JSFunctionSpec Image_funcs[] = {
    JS_FN("shiftHue", native_image_shiftHue, 2, 0),
    JS_FN("markColorInAlpha", native_image_markColorInAlpha, 0, 0),
    JS_FN("desaturate", native_image_desaturate, 0, 0),
    JS_FN("print", native_image_print, 0, 0),
    JS_FS_END
};

static JSBool native_image_print(JSContext *cx, unsigned argc, jsval *vp)
{
    return true;
}

static JSBool native_image_shiftHue(JSContext *cx, unsigned argc, jsval *vp)
{
    JS::CallArgs args = JS::CallArgsFromVp(argc, vp);
    NativeJSImage *nimg = IMAGE_FROM_CALLEE;
    int val;
    int color;

    if (!JS_ConvertArguments(cx, argc, args.array(), "ii", &val, &color)) {
        return false;
    }

    if (nimg->img) {
        nimg->img->shiftHue(val, color);
    }
    return true;
}

static JSBool native_image_markColorInAlpha(JSContext *cx,
    unsigned argc, jsval *vp)
{
    NativeJSImage *nimg = IMAGE_FROM_CALLEE;
    if (nimg->img) {
        nimg->img->markColorsInAlpha();
    }

    return true;
}

static JSBool native_image_desaturate(JSContext *cx,
    unsigned argc, jsval *vp)
{
    NativeJSImage *nimg = IMAGE_FROM_CALLEE;
    if (nimg->img) {
        nimg->img->desaturate();
    }

    return true;
}

static JSBool native_image_prop_set(JSContext *cx, JSHandleObject obj,
    JSHandleId id, JSBool strict, JSMutableHandleValue vp)
{
    NativeJSImage *nimg = NATIVE_IMAGE_GETTER(obj.get());

    switch(JSID_TO_INT(id)) {
        case IMAGE_PROP_SRC:
        {
            if (JSVAL_IS_STRING(vp)) {
                JSAutoByteString imgPath(cx, JSVAL_TO_STRING(vp));

                NativeJSObj(cx)->rootObjectUntilShutdown(obj.get());
                
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
                NativeFile *file = NativeJSFileIO::GetFileFromJSObject(cx,
                    vp.toObjectOrNull());

                if (!file) {
                    vp.set(JSVAL_VOID);
                    return true;
                }

                NativeJSObj(cx)->rootObjectUntilShutdown(obj.get());

                NativeBaseStream *stream = NativeBaseStream::create(file->getFullPath());
                if (stream == NULL) {
                    break;
                }
                nimg->m_Stream = stream;

                stream->setListener(nimg);
                stream->getContent();
                    

            } else {
                vp.set(JSVAL_VOID);
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

static JSBool native_Image_constructor(JSContext *cx, unsigned argc, jsval *vp)
{
    JSObject *ret = JS_NewObjectForConstructor(cx, &Image_class, vp);
    NativeJSImage *nimg;

    if (!JS_IsConstructing(cx, vp)) {
        JS_ReportError(cx, "Bad constructor");
        return false;
    }

    nimg = new NativeJSImage(ret, cx);

    JS_SetPrivate(ret, nimg);

    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(ret));

    JS_DefineProperties(cx, ret, Image_props);
    //JS_DefineFunctions(cx, ret, Image_funcs);

    return true;
}

bool NativeJSImage::JSObjectIs(JSContext *cx, JSObject *obj)
{
	return JS_InstanceOf(cx, obj, &Image_class, NULL);
}

NativeSkImage *NativeJSImage::JSObjectToNativeSkImage(JSObject *obj)
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
            jsval rval, onload_callback;
            if (this->setupWithBuffer((buffer *)msg.args[0].toPtr())) {
                if (JS_GetProperty(m_Cx, m_JSObject, "onload", &onload_callback) &&
                    JS_TypeOfValue(m_Cx, onload_callback) == JSTYPE_FUNCTION) {

                    JS_CallFunctionValue(m_Cx, m_JSObject, onload_callback,
                        0, NULL, &rval);
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
    JS_DefineProperty(m_Cx, m_JSObject, "width",
        INT_TO_JSVAL(ImageObject->getWidth()), NULL, NULL,
        JSPROP_PERMANENT | JSPROP_READONLY);

    JS_DefineProperty(m_Cx, m_JSObject, "height",
        INT_TO_JSVAL(ImageObject->getHeight()), NULL, NULL,
        JSPROP_PERMANENT | JSPROP_READONLY);

    NativeJSObj(m_Cx)->unrootObject(m_JSObject);

    return true;
}

#if 0
void NativeJSImage::onGetContent(const char *data, size_t len)
{
    jsval rval, onload_callback;
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

    JS_DefineProperty(cx, jsobj, "width",
        INT_TO_JSVAL(ImageObject->getWidth()), NULL, NULL,
        JSPROP_PERMANENT | JSPROP_READONLY);

    JS_DefineProperty(cx, jsobj, "height",
        INT_TO_JSVAL(ImageObject->getHeight()), NULL, NULL,
        JSPROP_PERMANENT | JSPROP_READONLY);

    if (JS_GetProperty(cx, jsobj, "onload", &onload_callback) &&
        JS_TypeOfValue(cx, onload_callback) == JSTYPE_FUNCTION) {

        JS_CallFunctionValue(cx, jsobj, onload_callback,
            0, NULL, &rval);
    }

    NativeJSObj(cx)->unrootObject(jsobj);
    timer_dispatch_async(delete_stream, stream);
    stream = NULL;
}
#endif
JSObject *NativeJSImage::buildImageObject(JSContext *cx, NativeSkImage *image,
	const char name[])
{	
	JSObject *ret = JS_NewObject(cx, &Image_class, NativeJSImage::classe, NULL);
    NativeJSImage *nimg = new NativeJSImage(ret, cx);

    nimg->img   = image;

    JS_SetPrivate(ret, nimg);

    printf("Build image object\n");

    JS_DefineProperty(cx, ret, "width",
        INT_TO_JSVAL(image->getWidth()), NULL, NULL,
        JSPROP_PERMANENT | JSPROP_READONLY);

    JS_DefineProperty(cx, ret, "height",
        INT_TO_JSVAL(image->getHeight()), NULL, NULL,
        JSPROP_PERMANENT | JSPROP_READONLY);

    JS_DefineProperty(cx, ret, "src",
        STRING_TO_JSVAL(JS_NewStringCopyZ(cx, (name ? name : "unknown"))),
        NULL, NULL,
        JSPROP_PERMANENT | JSPROP_READONLY);    

	return ret;
}

NativeJSImage::NativeJSImage(JSObject *obj, JSContext *cx) :
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
    JS_InitClass(cx, JS_GetGlobalObject(cx), NULL, &Image_class, 
        native_Image_constructor,
        0, NULL, Image_funcs, NULL, NULL);
}

//NATIVE_OBJECT_EXPOSE(Image)
