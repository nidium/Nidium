#include "NativeJSImage.h"
#include "NativeSkImage.h"
#include "NativeJS.h"

#include <string.h>

JSObject *NativeJSImage::classe = NULL;

#define NATIVE_IMAGE_GETTER(obj) ((class NativeJSImage *)JS_GetPrivate(obj))

static void Image_Finalize(JSFreeOp *fop, JSObject *obj);

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

static JSBool native_image_prop_set(JSContext *cx, JSHandleObject obj,
    JSHandleId id, JSBool strict, JSMutableHandleValue vp)
{
    NativeJSImage *nimg = NATIVE_IMAGE_GETTER(obj.get());

    switch(JSID_TO_INT(id)) {
        case IMAGE_PROP_SRC:
        {
            if (JSVAL_IS_STRING(vp)) {
                NativeSkImage *ImageObject;
                jsval rval, onload_callback;

                JSAutoByteString imgPath(cx, JSVAL_TO_STRING(vp));

                if (strncasecmp(imgPath.ptr(), "http://", 7) == 0) {
                    NativeJSObj(cx)->rootObjectUntilShutdown(obj.get());
                    
                    NativeHTTP *http = new NativeHTTP(imgPath.ptr(),
                        (ape_global *)JS_GetContextPrivate(cx));
                    http->request(NATIVE_IMAGE_GETTER(obj.get()));
                } else {

                    ImageObject = new NativeSkImage(imgPath.ptr());

                    nimg->img = ImageObject;

                    JS_DefineProperty(cx, obj, "width",
                        INT_TO_JSVAL(ImageObject->getWidth()), NULL, NULL,
                        JSPROP_PERMANENT | JSPROP_READONLY);

                    JS_DefineProperty(cx, obj, "height",
                        INT_TO_JSVAL(ImageObject->getHeight()), NULL, NULL,
                        JSPROP_PERMANENT | JSPROP_READONLY);

                    if (JS_GetProperty(cx, obj, "onload", &onload_callback) &&
                        JS_TypeOfValue(cx, onload_callback) == JSTYPE_FUNCTION) {

                        JS_CallFunctionValue(cx, obj, onload_callback,
                            0, NULL, &rval);
                    }
                }
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
        printf("Image finalized\n");
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

void NativeJSImage::onRequest(NativeHTTP::HTTPData *h,
    NativeHTTP::DataType type)
{
    if (type == NativeHTTP::DATA_IMAGE) {
        jsval rval, onload_callback;
        NativeSkImage *ImageObject = new NativeSkImage(h->data->data,
                                            h->data->used);
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

    } /* TODO : onError || onError is not defined by the spec. */
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
    httpref = NULL;
}

NativeJSImage::~NativeJSImage()
{
    if (img != NULL) {
        delete img;
    }
    if (httpref) {
        delete httpref;
    }
}

NATIVE_OBJECT_EXPOSE(Image)
