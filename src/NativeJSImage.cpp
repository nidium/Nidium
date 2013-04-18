#include "NativeJSImage.h"
#include "NativeSkImage.h"
#include "NativeJS.h"
#include "NativeSkia.h"

#include <string.h>

JSObject *NativeJSImage::classe = NULL;

#define NATIVE_IMAGE_GETTER(obj) ((class NativeJSImage *)JS_GetPrivate(obj))
#define IMAGE_FROM_CALLEE ((class NativeJSImage *)JS_GetPrivate(JS_GetParent(JSVAL_TO_OBJECT(JS_CALLEE(cx, vp)))))

static void Image_Finalize(JSFreeOp *fop, JSObject *obj);
static JSBool native_image_shiftHue(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_image_markColorInAlpha(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_image_desaturate(JSContext *cx, unsigned argc, jsval *vp);

static JSBool native_image_prop_set(JSContext *cx, JSHandleObject obj,
    JSHandleId id, JSBool strict, JSMutableHandleValue vp);

static JSClass Image_class = {
    "Image", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, Image_Finalize,
    JSCLASS_NO_OPTIONAL_MEMBERS
};

static JSPropertySpec Image_props[] = {
    {"src", IMAGE_PROP_SRC, 0, JSOP_NULLWRAPPER,
    	JSOP_WRAPPER(native_image_prop_set)},
    {0, 0, 0, JSOP_NULLWRAPPER, JSOP_NULLWRAPPER}
};

static JSFunctionSpec Image_funcs[] = {
    JS_FN("shiftHue", native_image_shiftHue, 2, 0),
    JS_FN("markColorInAlpha", native_image_markColorInAlpha, 0, 0),
    JS_FN("desaturate", native_image_desaturate, 0, 0),
    JS_FS_END
};

static JSBool native_image_shiftHue(JSContext *cx, unsigned argc, jsval *vp)
{
    NativeJSImage *nimg = IMAGE_FROM_CALLEE;
    int val;
    int color;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "ii", &val, &color)) {
        return JS_TRUE;
    }

    if (nimg->img) {
        nimg->img->shiftHue(val, color);
    }
    return JS_TRUE;
}

static JSBool native_image_markColorInAlpha(JSContext *cx,
    unsigned argc, jsval *vp)
{
    NativeJSImage *nimg = IMAGE_FROM_CALLEE;
    if (nimg->img) {
        nimg->img->markColorsInAlpha();
    }

    return JS_TRUE;
}

static JSBool native_image_desaturate(JSContext *cx,
    unsigned argc, jsval *vp)
{
    NativeJSImage *nimg = IMAGE_FROM_CALLEE;
    if (nimg->img) {
        nimg->img->desaturate();
    }

    return JS_TRUE;
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
                
                NativeStream *stream = new NativeStream(
                        (ape_global *)JS_GetContextPrivate(cx),
                        imgPath.ptr());

                nimg->stream = stream;

                stream->setDelegate(nimg);
                stream->getContent();
            
            } else {
                vp.set(JSVAL_VOID);
                return JS_TRUE;
            }
        }
        break;
        default:
            break;
    }
    return JS_TRUE;
}

void Image_Finalize(JSFreeOp *fop, JSObject *obj)
{
    NativeJSImage *img = NATIVE_IMAGE_GETTER(obj);
    if (img != NULL) {
        if (img->stream) {
            img->stream->setDelegate(NULL);
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
        return JS_FALSE;
    }

    nimg = new NativeJSImage();
    nimg->cx = cx;
    nimg->jsobj = ret;

    JS_SetPrivate(ret, nimg);

    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(ret));

    JS_DefineProperties(cx, ret, Image_props);
    JS_DefineFunctions(cx, ret, Image_funcs);

    return JS_TRUE;
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
    NativeStream *stream = (NativeStream *)arg;

    delete stream;

    return 0;
}

void NativeJSImage::onGetContent(const char *data, size_t len)
{
    jsval rval, onload_callback;
    ape_global *ape = (ape_global *)JS_GetContextPrivate(cx);

    NativeSkImage *ImageObject = new NativeSkImage((void *)data, len);
    if (ImageObject->img == NULL) {
        timer_dispatch_async(delete_stream, stream);
        stream = NULL;
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

JSObject *NativeJSImage::buildImageObject(JSContext *cx, NativeSkImage *image,
	const char name[])
{	
	JSObject *ret = JS_NewObject(cx, &Image_class, NativeJSImage::classe, NULL);
    NativeJSImage *nimg = new NativeJSImage();

    nimg->img   = image;
    nimg->jsobj = ret;
    nimg->cx    = cx;

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

NativeJSImage::NativeJSImage() :
    img(NULL), jsobj(NULL)
{
    stream = NULL;
}

NativeJSImage::~NativeJSImage()
{
    if (img != NULL) {
        delete img;
    }
    if (stream) {
        delete stream;
    }
}

NATIVE_OBJECT_EXPOSE(Image)
