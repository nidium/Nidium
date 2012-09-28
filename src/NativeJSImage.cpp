#include "NativeJSImage.h"
#include "NativeSkImage.h"

JSObject *NativeJSImage::classe = NULL;

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
    switch(JSID_TO_INT(id)) {
        case IMAGE_PROP_SRC:
        {
            if (JSVAL_IS_STRING(vp)) {
                NativeSkImage *ImageObject;
                jsval rval, onload_callback;

                JSAutoByteString imgPath(cx, JSVAL_TO_STRING(vp));
                ImageObject = new NativeSkImage(imgPath.ptr());
                JS_SetPrivate(obj, ImageObject);

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
    NativeSkImage *img = (class NativeSkImage *)JS_GetPrivate(obj);
    if (img != NULL) {
        delete img;
    }
}

static JSBool native_Image_constructor(JSContext *cx, unsigned argc, jsval *vp)
{
    JSObject *ret = JS_NewObjectForConstructor(cx, &Image_class, vp);

    /* TODO: JS_IsConstructing() */
    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(ret));

    JS_DefineProperties(cx, ret, Image_props);

    return JS_TRUE;
}

bool NativeJSImage::JSObjectIs(JSContext *cx, JSObject *obj)
{
	return JS_InstanceOf(cx, obj, &Image_class, NULL);
}


JSObject *NativeJSImage::buildImageObject(JSContext *cx, NativeSkImage *image,
	const char name[])
{	
	JSObject *ret = JS_NewObject(cx, &Image_class, NativeJSImage::classe, NULL);

    JS_SetPrivate(ret, image);

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

NATIVE_OBJECT_EXPOSE(Image)
