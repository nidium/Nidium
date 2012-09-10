#include "NativeJS.h"
#include "NativeSkia.h"
#include "NativeSkGradient.h"
#include "NativeSkImage.h"
#include "NativeSharedMessages.h"
#include <native_netlib.h>
#include <stdio.h>
#include <jsapi.h>
#include <jsprf.h>
#include <jsfriendapi.h>
#include <math.h>

#include <pthread.h>

enum {
    CANVAS_PROP_FILLSTYLE = 1,
    CANVAS_PROP_STROKESTYLE,
    CANVAS_PROP_LINEWIDTH,
    CANVAS_PROP_GLOBALALPHA,
    CANVAS_PROP_LINECAP,
    CANVAS_PROP_LINEJOIN,
    CANVAS_PROP_WIDTH,
    CANVAS_PROP_HEIGHT,
    CANVAS_PROP_GLOBALCOMPOSITEOPERATION,
    CANVAS_PROP_FONTSIZE,
    CANVAS_PROP_FONTTYPE,
    CANVAS_PROP_SHADOWOFFSETX,
    CANVAS_PROP_SHADOWOFFSETY,
    CANVAS_PROP_SHADOWBLUR,
    CANVAS_PROP_SHADOWCOLOR,
    IMAGE_PROP_SRC,
    SOCKET_PROP_BINARY
};

struct _native_sm_timer
{
    JSContext *cx;
    JSObject *global;
    jsval func;

    unsigned argc;
    jsval *argv;
    int ms;
    int cleared;
    struct _ticks_callback *timer;
    ape_timer *timerng;
};

enum {
    NATIVE_SOCKET_ISBINARY = 1 << 0
};

typedef struct _native_socket
{
    const char *host;
    unsigned short port;
    ape_socket *socket;
    int flags;

} native_socket;

#define NSKIA_NATIVE ((class NativeSkia *)JS_GetPrivate(JS_GetParent(JSVAL_TO_OBJECT(JS_CALLEE(cx, vp)))))
#define NSKIA_NATIVE_GETTER(obj) ((class NativeSkia *)JS_GetPrivate(obj))
#define NSKIA_OBJ(obj) ((class NativeSkia *)JS_GetPrivate(obj))

#define NJS ((class NativeJS *)JS_GetRuntimePrivate(JS_GetRuntime(cx)))

static void native_timer_wrapper(struct _native_sm_timer *params, int *last);
static int native_timerng_wrapper(void *arg);

static void CanvasGradient_Finalize(JSFreeOp *fop, JSObject *obj);
static void Canvas_Finalize(JSFreeOp *fop, JSObject *obj);
static void Image_Finalize(JSFreeOp *fop, JSObject *obj);
static void Socket_Finalize(JSFreeOp *fop, JSObject *obj);

static void native_socket_wrapper_onconnect(ape_socket *s, ape_global *ape);
static void native_socket_wrapper_read(ape_socket *s, ape_global *ape);
static void native_socket_wrapper_disconnect(ape_socket *s, ape_global *ape);

static JSClass global_class = {
    "_GLOBAL", JSCLASS_GLOBAL_FLAGS | JSCLASS_IS_GLOBAL,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, NULL,
    JSCLASS_NO_OPTIONAL_MEMBERS
};

static JSClass canvas_class = {
    "Canvas", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, Canvas_Finalize,
    JSCLASS_NO_OPTIONAL_MEMBERS
};

static JSClass socket_class = {
    "Socket", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, Socket_Finalize,
    JSCLASS_NO_OPTIONAL_MEMBERS
};

static JSClass image_class = {
    "Image", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, Image_Finalize,
    JSCLASS_NO_OPTIONAL_MEMBERS
};

static JSClass imageData_class = {
    "ImageData", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, NULL,
    JSCLASS_NO_OPTIONAL_MEMBERS
};

static JSClass thread_class = {
    "Thread", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, NULL,
    JSCLASS_NO_OPTIONAL_MEMBERS
};

static JSClass canvasGradient_class = {
    "CanvasGradient", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, CanvasGradient_Finalize,
    JSCLASS_NO_OPTIONAL_MEMBERS
};

static JSClass mouseEvent_class = {
    "MouseEvent", 0,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, NULL,
    JSCLASS_NO_OPTIONAL_MEMBERS
};

static JSClass messageEvent_class = {
    "MessageEvent", 0,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, NULL,
    JSCLASS_NO_OPTIONAL_MEMBERS
};

static JSClass textEvent_class = {
    "TextInputEvent", 0,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, NULL,
    JSCLASS_NO_OPTIONAL_MEMBERS
};

static JSClass keyEvent_class = {
    "keyEvent", 0,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, NULL,
    JSCLASS_NO_OPTIONAL_MEMBERS
};

jsval gfunc  = JSVAL_VOID;

/******** Natives ********/
static JSBool Print(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_load(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_canvas_shadow(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_canvas_fillRect(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_canvas_strokeRect(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_canvas_clearRect(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_canvas_fillText(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_canvas_beginPath(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_canvas_moveTo(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_canvas_lineTo(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_canvas_fill(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_canvas_stroke(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_canvas_closePath(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_canvas_arc(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_canvas_rect(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_canvas_quadraticCurveTo(JSContext *cx, unsigned argc,
    jsval *vp);
static JSBool native_canvas_bezierCurveTo(JSContext *cx, unsigned argc,
    jsval *vp);
static JSBool native_canvas_rotate(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_canvas_scale(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_canvas_save(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_canvas_restore(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_canvas_translate(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_canvas_transform(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_canvas_setTransform(JSContext *cx, unsigned argc,
    jsval *vp);
static JSBool native_set_timeout(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_post_message(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_set_interval(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_clear_timeout(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_canvas_clip(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_canvas_createImageData(JSContext *cx,
    unsigned argc, jsval *vp);
static JSBool native_canvas_putImageData(JSContext *cx,
    unsigned argc, jsval *vp);
static JSBool native_canvas_getImageData(JSContext *cx,
    unsigned argc, jsval *vp);
static JSBool native_canvas_createLinearGradient(JSContext *cx,
    unsigned argc, jsval *vp);
static JSBool native_canvas_createRadialGradient(JSContext *cx,
    unsigned argc, jsval *vp);
static JSBool native_canvasGradient_addColorStop(JSContext *cx,
    unsigned argc, jsval *vp);
static JSBool native_canvas_requestAnimationFrame(JSContext *cx,
    unsigned argc, jsval *vp);

static JSBool native_socket_connect(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_socket_write(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_socket_close(JSContext *cx, unsigned argc, jsval *vp);

static JSBool native_canvas_stub(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_canvas_drawImage(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_canvas_measureText(JSContext *cx, unsigned argc,
    jsval *vp);
/*************************/

/******** Setters ********/

static JSBool native_canvas_prop_set(JSContext *cx, JSHandleObject obj,
    JSHandleId id, JSBool strict, jsval *vp);
static JSBool native_canvas_prop_get(JSContext *cx, JSHandleObject obj,
    JSHandleId id, jsval *vp);
static JSBool native_image_prop_set(JSContext *cx, JSHandleObject obj,
    JSHandleId id, JSBool strict, jsval *vp);
static JSBool native_socket_prop_set(JSContext *cx, JSHandleObject obj,
    JSHandleId id, JSBool strict, jsval *vp);

static JSPropertySpec canvas_props[] = {
    {"fillStyle", CANVAS_PROP_FILLSTYLE, JSPROP_PERMANENT, NULL,
        native_canvas_prop_set},
    {"strokeStyle", CANVAS_PROP_STROKESTYLE, JSPROP_PERMANENT, NULL,
        native_canvas_prop_set},
    {"lineWidth", CANVAS_PROP_LINEWIDTH, JSPROP_PERMANENT, NULL,
        native_canvas_prop_set},
    {"globalAlpha", CANVAS_PROP_GLOBALALPHA, JSPROP_PERMANENT, NULL,
        native_canvas_prop_set},
    {"globalCompositeOperation", CANVAS_PROP_GLOBALCOMPOSITEOPERATION,
    JSPROP_PERMANENT, NULL,
        native_canvas_prop_set},
    {"fontSize", CANVAS_PROP_FONTSIZE, JSPROP_PERMANENT, NULL,
    native_canvas_prop_set},
    {"fontType", CANVAS_PROP_FONTTYPE, JSPROP_PERMANENT, NULL,
    native_canvas_prop_set},
    {"lineCap", CANVAS_PROP_LINECAP, JSPROP_PERMANENT, NULL,
        native_canvas_prop_set},
    {"lineJoin", CANVAS_PROP_LINEJOIN, JSPROP_PERMANENT, NULL,
        native_canvas_prop_set},
    {"shadowOffsetX", CANVAS_PROP_SHADOWOFFSETX, JSPROP_PERMANENT, NULL,
        native_canvas_prop_set},
    {"shadowOffsetY", CANVAS_PROP_SHADOWOFFSETY, JSPROP_PERMANENT, NULL,
        native_canvas_prop_set},
    {"shadowBlur", CANVAS_PROP_SHADOWBLUR, JSPROP_PERMANENT, NULL,
        native_canvas_prop_set},
    {"shadowColor", CANVAS_PROP_SHADOWCOLOR, JSPROP_PERMANENT, NULL,
        native_canvas_prop_set},
    {"width", CANVAS_PROP_WIDTH, JSPROP_PERMANENT, native_canvas_prop_get,
        NULL},
    {"height", CANVAS_PROP_HEIGHT, JSPROP_PERMANENT, native_canvas_prop_get,
        NULL},
    {0, 0, 0, 0, 0}
};

static JSPropertySpec Image_props[] = {
    {"src", IMAGE_PROP_SRC, 0, NULL, native_image_prop_set},
    {0, 0, 0, 0, 0}
};

static JSPropertySpec Socket_props[] = {
    {"isBinary", SOCKET_PROP_BINARY, 0, NULL, native_socket_prop_set},
    {0, 0, 0, 0, 0}
};

/*************************/

static JSFunctionSpec glob_funcs_threaded[] = {
    JS_FN("send", native_post_message, 1, 0),
    JS_FS_END
};

static JSFunctionSpec glob_funcs[] = {
    
    JS_FN("echo", Print, 0, 0),
    JS_FN("load", native_load, 1, 0),
    JS_FN("setTimeout", native_set_timeout, 2, 0),
    JS_FN("setInterval", native_set_interval, 2, 0),
    JS_FN("clearTimeout", native_clear_timeout, 1, 0),
    JS_FN("clearInterval", native_clear_timeout, 1, 0),
    JS_FS_END
};

static JSFunctionSpec gradient_funcs[] = {
    
    JS_FN("addColorStop", native_canvasGradient_addColorStop, 2, 0),

    JS_FS_END
};

static JSFunctionSpec socket_funcs[] = {
    JS_FN("connect", native_socket_connect, 0, 0),
    JS_FN("write", native_socket_write, 1, 0),
    JS_FN("disconnect", native_socket_close, 0, 0),
    JS_FS_END
};

static JSFunctionSpec canvas_funcs[] = {
    JS_FN("shadow", native_canvas_shadow, 0, 0),
    JS_FN("onerror", native_canvas_stub, 0, 0),
    JS_FN("fillRect", native_canvas_fillRect, 4, 0),
    JS_FN("fillText", native_canvas_fillText, 3, 0),
    JS_FN("strokeRect", native_canvas_strokeRect, 4, 0),
    JS_FN("clearRect", native_canvas_clearRect, 4, 0),
    JS_FN("beginPath", native_canvas_beginPath, 0, 0),
    JS_FN("moveTo", native_canvas_moveTo, 2, 0),
    JS_FN("lineTo", native_canvas_lineTo, 2, 0),
    JS_FN("fill", native_canvas_fill, 0, 0),
    JS_FN("stroke", native_canvas_stroke, 0, 0),
    JS_FN("closePath", native_canvas_closePath, 0, 0),
    JS_FN("clip", native_canvas_clip, 0, 0),
    JS_FN("arc", native_canvas_arc, 5, 0),
    JS_FN("rect", native_canvas_rect, 4, 0),
    JS_FN("quadraticCurveTo", native_canvas_quadraticCurveTo, 4, 0),
    JS_FN("bezierCurveTo", native_canvas_bezierCurveTo, 4, 0),
    JS_FN("rotate", native_canvas_rotate, 1, 0),
    JS_FN("scale", native_canvas_scale, 2, 0),
    JS_FN("save", native_canvas_save, 0, 0),
    JS_FN("restore", native_canvas_restore, 0, 0),
    JS_FN("translate", native_canvas_translate, 2, 0),
    JS_FN("transform", native_canvas_transform, 6, 0),
    JS_FN("setTransform", native_canvas_setTransform, 6, 0),
    JS_FN("createLinearGradient", native_canvas_createLinearGradient, 4, 0),
    JS_FN("createRadialGradient", native_canvas_createRadialGradient, 6, 0),
    JS_FN("createImageData", native_canvas_createImageData, 2, 0),
    JS_FN("putImageData", native_canvas_putImageData, 3, 0),
    JS_FN("getImageData", native_canvas_getImageData, 4, 0),
    JS_FN("requestAnimationFrame", native_canvas_requestAnimationFrame, 1, 0),
    JS_FN("drawImage", native_canvas_drawImage, 3, 0),
    JS_FN("measureText", native_canvas_measureText, 1, 0),
    JS_FS_END
};

static JSBool native_canvas_stub(JSContext *cx, unsigned argc, jsval *vp)
{
    return JS_TRUE;
}

void CanvasGradient_Finalize(JSFreeOp *fop, JSObject *obj)
{
    NativeSkGradient *gradient = (class NativeSkGradient *)JS_GetPrivate(obj);
    if (gradient != NULL) {
        delete gradient;
    }
}

void Socket_Finalize(JSFreeOp *fop, JSObject *obj)
{
    native_socket *nsocket = (native_socket *)JS_GetPrivate(obj);
    if (nsocket == NULL) {
        return;
    }
    if (nsocket->socket != NULL) {
        nsocket->socket->ctx[0] = NULL;
        nsocket->socket->ctx[1] = NULL;
        nsocket->socket->ctx[2] = NULL;
        APE_socket_destroy(nsocket->socket);
    }
    free(nsocket);
}

void Canvas_Finalize(JSFreeOp *fop, JSObject *obj)
{
    NativeSkia *currSkia = NSKIA_NATIVE_GETTER(obj);
    if (currSkia != NULL) {
        delete currSkia;
    }
}

void Image_Finalize(JSFreeOp *fop, JSObject *obj)
{
    NativeSkImage *img = (class NativeSkImage *)JS_GetPrivate(obj);
    if (img != NULL) {
        delete img;
    }
}

static void
reportError(JSContext *cx, const char *message, JSErrorReport *report)
{
    int i, j, k, n;
    char *prefix, *tmp;
    const char *ctmp;

    if (!report) {
        fprintf(stdout, "%s\n", message);
        fflush(stdout);
        return;
    }

    /* Conditionally ignore reported warnings. */
    /*if (JSREPORT_IS_WARNING(report->flags) && !reportWarnings)
        return;*/

    prefix = NULL;
    if (report->filename)
        prefix = JS_smprintf("%s:", report->filename);
    if (report->lineno) {
        tmp = prefix;
        prefix = JS_smprintf("%s%u: ", tmp ? tmp : "", report->lineno);
        JS_free(cx, tmp);
    }
    if (JSREPORT_IS_WARNING(report->flags)) {
        tmp = prefix;
        prefix = JS_smprintf("%s%swarning: ",
                             tmp ? tmp : "",
                             JSREPORT_IS_STRICT(report->flags) ? "strict " : "");
        JS_free(cx, tmp);
    }

    /* embedded newlines -- argh! */
    while ((ctmp = strchr(message, '\n')) != 0) {
        ctmp++;
        if (prefix)
            fputs(prefix, stdout);
        fwrite(message, 1, ctmp - message, stdout);
        message = ctmp;
    }

    /* If there were no filename or lineno, the prefix might be empty */
    if (prefix)
        fputs(prefix, stdout);
    fputs(message, stdout);

    if (!report->linebuf) {
        fputc('\n', stdout);
        goto out;
    }

    /* report->linebuf usually ends with a newline. */
    n = strlen(report->linebuf);
    fprintf(stdout, ":\n%s%s%s%s",
            prefix,
            report->linebuf,
            (n > 0 && report->linebuf[n-1] == '\n') ? "" : "\n",
            prefix);
    n = report->tokenptr - report->linebuf;
    for (i = j = 0; i < n; i++) {
        if (report->linebuf[i] == '\t') {
            for (k = (j + 8) & ~7; j < k; j++) {
                fputc('.', stdout);
            }
            continue;
        }
        fputc('.', stdout);
        j++;
    }
    fputs("^\n", stdout);
 out:
    fflush(stdout);
    if (!JSREPORT_IS_WARNING(report->flags)) {
    /*
        if (report->errorNumber == JSMSG_OUT_OF_MEMORY) {
            gExitCode = EXITCODE_OUT_OF_MEMORY;
        } else {
            gExitCode = EXITCODE_RUNTIME_ERROR;
        }*/
    }
    JS_free(cx, prefix);
}

static JSBool
PrintInternal(JSContext *cx, unsigned argc, jsval *vp, FILE *file)
{
    jsval *argv;
    unsigned i;
    JSString *str;
    char *bytes;

    argv = JS_ARGV(cx, vp);
    for (i = 0; i < argc; i++) {
        str = JS_ValueToString(cx, argv[i]);
        if (!str)
            return false;
        bytes = JS_EncodeString(cx, str);
        if (!bytes)
            return false;
        fprintf(file, "%s%s", i ? " " : "", bytes);
        JS_free(cx, bytes);
    }

    fputc('\n', file);
    fflush(file);

    JS_SET_RVAL(cx, vp, JSVAL_VOID);
    return true;
}

static JSBool
Print(JSContext *cx, unsigned argc, jsval *vp)
{
    return PrintInternal(cx, argc, vp, stdout);
}

static JSBool native_canvas_prop_get(JSContext *cx, JSHandleObject obj,
    JSHandleId id, jsval *vp)
{
    NativeSkia *curSkia = NSKIA_NATIVE_GETTER(obj.get());

    switch(JSID_TO_INT(id)) {
        case CANVAS_PROP_WIDTH:
        {
            *vp = INT_TO_JSVAL(curSkia->getWidth());
        }
        break;
        case CANVAS_PROP_HEIGHT:
        {
            *vp = INT_TO_JSVAL(curSkia->getHeight());
        }
        break;
        default:
            break;
    }

    return JS_TRUE;
}

static JSBool native_socket_prop_set(JSContext *cx, JSHandleObject obj,
    JSHandleId id, JSBool strict, jsval *vp)
{
    switch(JSID_TO_INT(id)) {
        case SOCKET_PROP_BINARY:
        {
            native_socket *nsocket;
            if (JSVAL_IS_BOOLEAN(*vp) &&
                (nsocket = (native_socket *)JS_GetPrivate(obj.get())) != NULL) {

                nsocket->flags = (JSVAL_TO_BOOLEAN(*vp) == JS_TRUE ?
                    nsocket->flags | NATIVE_SOCKET_ISBINARY :
                    nsocket->flags & ~NATIVE_SOCKET_ISBINARY);

            } else {
                *vp = JSVAL_FALSE;
                return JS_TRUE;
            }
        }
        break;
        default:
            break;
    }
    return JS_TRUE;
}

static JSBool native_image_prop_set(JSContext *cx, JSHandleObject obj,
    JSHandleId id, JSBool strict, jsval *vp)
{
    switch(JSID_TO_INT(id)) {
        case IMAGE_PROP_SRC:
        {
            if (JSVAL_IS_STRING(*vp)) {
                NativeSkImage *ImageObject;
                jsval rval, onload_callback;

                JSAutoByteString imgPath(cx, JSVAL_TO_STRING(*vp));
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
                *vp = JSVAL_VOID;
                return JS_TRUE;
            }
        }
        break;
        default:
            break;
    }
    return JS_TRUE;
}

/* TODO: do not change the value when a wrong type is set */
static JSBool native_canvas_prop_set(JSContext *cx, JSHandleObject obj,
    JSHandleId id, JSBool strict, jsval *vp)
{
    NativeSkia *curSkia = NSKIA_NATIVE_GETTER(obj.get());

    switch(JSID_TO_INT(id)) {
        case CANVAS_PROP_SHADOWOFFSETX:
        {
            double ret;
            if (!JSVAL_IS_NUMBER(*vp)) {
                *vp = JSVAL_VOID;
                return JS_TRUE;
            }
            JS_ValueToNumber(cx, *vp, &ret);

            curSkia->setShadowOffsetX(ret);  
        }
        break;
        case CANVAS_PROP_SHADOWOFFSETY:
        {
            double ret;
            if (!JSVAL_IS_NUMBER(*vp)) {
                *vp = JSVAL_VOID;
                return JS_TRUE;
            }
            JS_ValueToNumber(cx, *vp, &ret);

            curSkia->setShadowOffsetY(ret);  
        }
        break;
        case CANVAS_PROP_SHADOWBLUR:
        {
            double ret;
            if (!JSVAL_IS_NUMBER(*vp)) {
                *vp = JSVAL_VOID;
                return JS_TRUE;
            }
            JS_ValueToNumber(cx, *vp, &ret);

            curSkia->setShadowBlur(ret);  
        }
        break;
        case CANVAS_PROP_SHADOWCOLOR:
        {
            if (!JSVAL_IS_STRING(*vp)) {
                *vp = JSVAL_VOID;

                return JS_TRUE;
            }
            JSAutoByteString color(cx, JSVAL_TO_STRING(*vp));
            curSkia->setShadowColor(color.ptr());          
        }
        break;
        case CANVAS_PROP_FONTSIZE:
        {
            double ret;
            if (!JSVAL_IS_NUMBER(*vp)) {
                *vp = JSVAL_VOID;
                return JS_TRUE;
            }
            JS_ValueToNumber(cx, *vp, &ret);
            curSkia->setFontSize(ret);

        }
        break;
        case CANVAS_PROP_FONTTYPE:
        {
            if (!JSVAL_IS_STRING(*vp)) {
                *vp = JSVAL_VOID;

                return JS_TRUE;
            }
            JSAutoByteString font(cx, JSVAL_TO_STRING(*vp));
            curSkia->setFontType(font.ptr());          
        }
        break;
        case CANVAS_PROP_FILLSTYLE:
        {
            if (JSVAL_IS_STRING(*vp)) {
                JSAutoByteString colorName(cx, JSVAL_TO_STRING(*vp));
                curSkia->setFillColor(colorName.ptr());
            } else if (!JSVAL_IS_PRIMITIVE(*vp) && 
                JS_InstanceOf(cx, JSVAL_TO_OBJECT(*vp),
                    &canvasGradient_class, NULL)) {

                NativeSkGradient *gradient = (class NativeSkGradient *)
                                            JS_GetPrivate(JSVAL_TO_OBJECT(*vp));

                curSkia->setFillColor(gradient);

            } else {
                *vp = JSVAL_VOID;

                return JS_TRUE;                
            }
        }
        break;
        case CANVAS_PROP_STROKESTYLE:
        {
            if (JSVAL_IS_STRING(*vp)) {
                JSAutoByteString colorName(cx, JSVAL_TO_STRING(*vp));
                curSkia->setStrokeColor(colorName.ptr());
            } else if (!JSVAL_IS_PRIMITIVE(*vp) && 
                JS_InstanceOf(cx, JSVAL_TO_OBJECT(*vp),
                    &canvasGradient_class, NULL)) {

                NativeSkGradient *gradient = (class NativeSkGradient *)
                                            JS_GetPrivate(JSVAL_TO_OBJECT(*vp));

                curSkia->setStrokeColor(gradient);

            } else {
                *vp = JSVAL_VOID;

                return JS_TRUE;                
            }    
        }
        break;
        case CANVAS_PROP_LINEWIDTH:
        {
            double ret;
            if (!JSVAL_IS_NUMBER(*vp)) {
                *vp = JSVAL_VOID;
                return JS_TRUE;
            }
            JS_ValueToNumber(cx, *vp, &ret);
            curSkia->setLineWidth(ret);
        }
        break;
        case CANVAS_PROP_GLOBALALPHA:
        {
            double ret;
            if (!JSVAL_IS_NUMBER(*vp)) {
                *vp = JSVAL_VOID;
                return JS_TRUE;
            }
            JS_ValueToNumber(cx, *vp, &ret);
            curSkia->setGlobalAlpha(ret);
        }
        break;
        case CANVAS_PROP_GLOBALCOMPOSITEOPERATION:
        {
            if (!JSVAL_IS_STRING(*vp)) {
                *vp = JSVAL_VOID;

                return JS_TRUE;
            }
            JSAutoByteString composite(cx, JSVAL_TO_STRING(*vp));
            curSkia->setGlobalComposite(composite.ptr());            
        }
        break;
        case CANVAS_PROP_LINECAP:
        {
            if (!JSVAL_IS_STRING(*vp)) {
                *vp = JSVAL_VOID;

                return JS_TRUE;
            }
            JSAutoByteString lineCap(cx, JSVAL_TO_STRING(*vp));
            curSkia->setLineCap(lineCap.ptr());                
        }
        break;
        case CANVAS_PROP_LINEJOIN:
        {
            if (!JSVAL_IS_STRING(*vp)) {
                *vp = JSVAL_VOID;

                return JS_TRUE;
            }
            JSAutoByteString lineJoin(cx, JSVAL_TO_STRING(*vp));
            curSkia->setLineJoin(lineJoin.ptr());                
        }
        break;        
        default:
            break;
    }


    return JS_TRUE;
}


static JSBool native_canvas_fillRect(JSContext *cx, unsigned argc, jsval *vp)
{
    double x, y, width, height;
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "dddd", &x, &y, &width, &height)) {
        return JS_TRUE;
    }

    NSKIA_NATIVE->drawRect(x, y, width+x, height+y, 0);

    return JS_TRUE;
}

static JSBool native_canvas_strokeRect(JSContext *cx, unsigned argc, jsval *vp)
{
    double x, y, width, height;
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "dddd", &x, &y,
        &width, &height)) {
        return JS_TRUE;
    }

    NSKIA_NATIVE->drawRect(x, y, width+x, height+y, 1);

    return JS_TRUE;
}

static JSBool native_canvas_clearRect(JSContext *cx, unsigned argc, jsval *vp)
{
    int x, y, width, height;
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "iiii", &x, &y,
        &width, &height)) {
        return JS_TRUE;
    }

    NSKIA_NATIVE->clearRect(x, y, width+x, height+y);

    return JS_TRUE;
}

static JSBool native_canvas_fillText(JSContext *cx, unsigned argc, jsval *vp)
{
    int x, y, maxwidth;
    JSString *str;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "Sii/i",
            &str, &x, &y, &maxwidth)) {
        return JS_TRUE;
    }

    JSAutoByteString text(cx, str);

    NSKIA_NATIVE->drawText(text.ptr(), x, y);

    return JS_TRUE;
}

static JSBool native_canvas_shadow(JSContext *cx, unsigned argc, jsval *vp)
{
    NSKIA_NATIVE->setShadow();
    return JS_TRUE;
}

static JSBool native_canvas_beginPath(JSContext *cx, unsigned argc, jsval *vp)
{
    NSKIA_NATIVE->beginPath();

    return JS_TRUE;
}

static JSBool native_canvas_moveTo(JSContext *cx, unsigned argc, jsval *vp)
{
    double x, y;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "dd", &x, &y)) {
        return JS_TRUE;
    }

    NSKIA_NATIVE->moveTo(x, y);

    return JS_TRUE;
}

static JSBool native_canvas_lineTo(JSContext *cx, unsigned argc, jsval *vp)
{
    double x, y;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "dd", &x, &y)) {
        return JS_TRUE;
    }

    NSKIA_NATIVE->lineTo(x, y);

    return JS_TRUE;
}

static JSBool native_canvas_fill(JSContext *cx, unsigned argc, jsval *vp)
{
    NSKIA_NATIVE->fill();

    return JS_TRUE;
}

static JSBool native_canvas_stroke(JSContext *cx, unsigned argc, jsval *vp)
{
    NSKIA_NATIVE->stroke();

    return JS_TRUE;
}
static JSBool native_canvas_closePath(JSContext *cx, unsigned argc, jsval *vp)
{
    NSKIA_NATIVE->closePath();

    return JS_TRUE;
}

static JSBool native_canvas_clip(JSContext *cx, unsigned argc, jsval *vp)
{
    NSKIA_NATIVE->clip();

    return JS_TRUE;
}

static JSBool native_canvas_rect(JSContext *cx, unsigned argc, jsval *vp)
{
    double x, y, width, height;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "dddd", &x, &y,
        &width, &height)) {
        return JS_TRUE;
    }

    NSKIA_NATIVE->rect(x, y, width, height);

    return JS_TRUE;
}

static JSBool native_canvas_arc(JSContext *cx, unsigned argc, jsval *vp)
{
    int x, y, radius;
    double startAngle, endAngle;
    JSBool CCW = JS_FALSE;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "iiidd/b", &x, &y,
        &radius, &startAngle, &endAngle, &CCW)) {
        return JS_TRUE;
    }

    NSKIA_NATIVE->arc(x, y, radius, startAngle, endAngle, CCW);

    return JS_TRUE;
}

static JSBool native_canvas_quadraticCurveTo(JSContext *cx, unsigned argc,
    jsval *vp)
{
    int x, y, cpx, cpy;
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "iiii", &cpx, &cpy,
        &x, &y)) {
        return JS_TRUE;
    }

    NSKIA_NATIVE->quadraticCurveTo(cpx, cpy, x, y);

    return JS_TRUE;
}

static JSBool native_canvas_bezierCurveTo(JSContext *cx, unsigned argc,
    jsval *vp)
{
    double x, y, cpx, cpy, cpx2, cpy2;
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "dddddd", &cpx, &cpy,
        &cpx2, &cpy2, &x, &y)) {
        return JS_TRUE;
    }

    NSKIA_NATIVE->bezierCurveTo(cpx, cpy, cpx2, cpy2, x, y);

    return JS_TRUE;
}

static JSBool native_canvas_rotate(JSContext *cx, unsigned argc, jsval *vp)
{
    double angle;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "d", &angle)) {
        return JS_TRUE;
    }

    NSKIA_NATIVE->rotate(angle);

    return JS_TRUE;
}

static JSBool native_canvas_scale(JSContext *cx, unsigned argc, jsval *vp)
{
    double x, y;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "dd", &x, &y)) {
        return JS_TRUE;
    }

    NSKIA_NATIVE->scale(x, y);

    return JS_TRUE;
}

static JSBool native_canvas_translate(JSContext *cx, unsigned argc, jsval *vp)
{
    double x, y;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "dd", &x, &y)) {
        return JS_TRUE;
    }

    NSKIA_NATIVE->translate(x, y);

    return JS_TRUE;
}

static JSBool native_canvas_transform(JSContext *cx, unsigned argc, jsval *vp)
{
    double scalex, skewx, skewy, scaley, translatex, translatey;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "dddddd",
        &scalex, &skewx, &skewy, &scaley, &translatex, &translatey)) {
        return JS_TRUE;
    }

    NSKIA_NATIVE->transform(scalex, skewx, skewy, scaley,
        translatex, translatey, 0);

    return JS_TRUE;
}

static JSBool native_canvas_setTransform(JSContext *cx, unsigned argc, jsval *vp)
{
    double scalex, skewx, skewy, scaley, translatex, translatey;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "dddddd",
        &scalex, &skewx, &skewy, &scaley, &translatex, &translatey)) {
        return JS_TRUE;
    }

    NSKIA_NATIVE->transform(scalex, skewx, skewy, scaley,
        translatex, translatey, 1);

    return JS_TRUE;
}

static JSBool native_canvas_save(JSContext *cx, unsigned argc, jsval *vp)
{
    NSKIA_NATIVE->save();

    return JS_TRUE;
}

static JSBool native_canvas_restore(JSContext *cx, unsigned argc, jsval *vp)
{
    NSKIA_NATIVE->restore();

    return JS_TRUE;
}

static JSBool native_canvas_createLinearGradient(JSContext *cx,
    unsigned argc, jsval *vp)
{
    JSObject *linearObject;
    double x1, y1, x2, y2;
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "dddd",
        &x1, &y1, &x2, &y2)) {
        return JS_TRUE;
    }

    linearObject = JS_NewObject(cx, &canvasGradient_class, NULL, NULL);

    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(linearObject));

    JS_SetPrivate(linearObject,
        new NativeSkGradient(x1, y1, x2, y2));

    JS_DefineFunctions(cx, linearObject, gradient_funcs);

    return JS_TRUE;
}

static JSBool native_canvas_getImageData(JSContext *cx,
    unsigned argc, jsval *vp)
{
    int left, top, width, height;
    JSObject *dataObject;
    JSObject *arrBuffer;
    uint8_t *data;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "iiii",
        &left, &top, &width, &height)) {
        return JS_TRUE;
    }

    dataObject = JS_NewObject(cx, &imageData_class, NULL, NULL);

    arrBuffer = JS_NewUint8ClampedArray(cx, width*height * 4);
    data = JS_GetUint8ClampedArrayData(arrBuffer, cx);

    NSKIA_NATIVE->readPixels(top, left, width, height, data);

    JS_DefineProperty(cx, dataObject, "width", UINT_TO_JSVAL(width),
        NULL, NULL, JSPROP_PERMANENT | JSPROP_READONLY);
    JS_DefineProperty(cx, dataObject, "height", UINT_TO_JSVAL(height),
        NULL, NULL, JSPROP_PERMANENT | JSPROP_READONLY);
    JS_DefineProperty(cx, dataObject, "data", OBJECT_TO_JSVAL(arrBuffer),
        NULL, NULL, JSPROP_PERMANENT | JSPROP_READONLY);

    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(dataObject));

    return JS_TRUE;
}

/* TODO: Huge memory leak? */
static JSBool native_canvas_putImageData(JSContext *cx,
    unsigned argc, jsval *vp)
{
    JSObject *dataObject;
    int x, y;
    uint8_t *pixels;
    jsval jdata, jwidth, jheight;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "oii",
        &dataObject, &x, &y)) {
        return JS_TRUE;
    }

    if (!JS_InstanceOf(cx, dataObject, &imageData_class, NULL)) {
        return JS_TRUE;
    }

    JS_GetProperty(cx, dataObject, "data", &jdata);
    JS_GetProperty(cx, dataObject, "width", &jwidth);
    JS_GetProperty(cx, dataObject, "height", &jheight);

    pixels = JS_GetUint8ClampedArrayData(JSVAL_TO_OBJECT(jdata), cx);

    NSKIA_NATIVE->drawPixels(pixels, JSVAL_TO_INT(jwidth), JSVAL_TO_INT(jheight),
        x, y);

    return JS_TRUE;
}

static JSBool native_canvas_createImageData(JSContext *cx,
    unsigned argc, jsval *vp)
{
    unsigned long x, y;
    JSObject *dataObject;
    JSObject *arrBuffer;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "uu",
        &x, &y)) {
        return JS_TRUE;
    }

    dataObject = JS_NewObject(cx, &imageData_class, NULL, NULL);

    JS_DefineProperty(cx, dataObject, "width", UINT_TO_JSVAL(x), NULL, NULL,
        JSPROP_PERMANENT | JSPROP_READONLY);

    JS_DefineProperty(cx, dataObject, "height", UINT_TO_JSVAL(y), NULL, NULL,
        JSPROP_PERMANENT | JSPROP_READONLY);

    arrBuffer = JS_NewUint8ClampedArray(cx, x*y * 4);
    if (arrBuffer == NULL) {
        JS_ReportOutOfMemory(cx);
        return JS_TRUE;
    }

    JS_DefineProperty(cx, dataObject, "data", OBJECT_TO_JSVAL(arrBuffer), NULL,
        NULL, JSPROP_PERMANENT | JSPROP_READONLY);

    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(dataObject));

    return JS_TRUE;
}

static JSBool native_canvas_createRadialGradient(JSContext *cx,
    unsigned argc, jsval *vp)
{
    JSObject *linearObject;
    double x1, y1, x2, y2, r1, r2;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "dddddd",
        &x1, &y1, &r1, &x2, &y2, &r2)) {
        return JS_TRUE;
    }

    linearObject = JS_NewObject(cx, &canvasGradient_class, NULL, NULL);

    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(linearObject));

    JS_SetPrivate(linearObject,
        new NativeSkGradient(x1, y1, r1, x2, y2, r2));

    JS_DefineFunctions(cx, linearObject, gradient_funcs);

    return JS_TRUE;
}

static JSBool native_canvasGradient_addColorStop(JSContext *cx,
    unsigned argc, jsval *vp)
{
    double position;
    JSString *color;
    JSObject *caller = JS_THIS_OBJECT(cx, vp);
    NativeSkGradient *gradient;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "dS",
        &position, &color)) {
        return JS_TRUE;
    }

    if ((gradient = (NativeSkGradient *)JS_GetPrivate(caller)) != NULL) {
        JSAutoByteString colorstr(cx, color);

        gradient->addColorStop(position, colorstr.ptr());
    }

    return JS_TRUE;
}

static JSBool native_socket_connect(JSContext *cx, unsigned argc, jsval *vp)
{
    native_socket *nsocket;
    ape_socket *socket;
    ape_global *net = (ape_global *)JS_GetContextPrivate(cx);
    JSObject *caller = JS_THIS_OBJECT(cx, vp);

    nsocket = (native_socket *)JS_GetPrivate(caller);

    if (nsocket == NULL || nsocket->socket != NULL) {
        return JS_TRUE;
    }

    if ((socket = APE_socket_new(APE_SOCKET_PT_TCP, 0, net)) == NULL) {
        printf("[Socket] Cant load socket (new)\n");
        /* TODO: add exception */
        return JS_TRUE;
    }


    socket->callbacks.on_connected  = native_socket_wrapper_onconnect;
    socket->callbacks.on_read       = native_socket_wrapper_read;
    socket->callbacks.on_disconnect = native_socket_wrapper_disconnect;

    socket->ctx[0] = caller;
    socket->ctx[1] = cx;
    socket->ctx[2] = nsocket;

    if (APE_socket_connect(socket, nsocket->port, nsocket->host) == -1) {
        printf("[Socket] Cant connect (0)\n");
        return JS_TRUE;
    }

    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(caller));

    return JS_TRUE;
}

static JSBool native_socket_write(JSContext *cx, unsigned argc, jsval *vp)
{
    JSString *data;
    JSObject *caller = JS_THIS_OBJECT(cx, vp);
    native_socket *nsocket = (native_socket *)JS_GetPrivate(caller);

    if (nsocket == NULL || nsocket->socket == NULL ||
        !JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "S", &data)) {
        return JS_TRUE;
    }

    /* TODO: Use APE_DATA_AUTORELEASE and specicy a free func (JS_free)) */
    JSAutoByteString cdata(cx, data);

    APE_socket_write(nsocket->socket, (unsigned char*)cdata.ptr(),
        strlen(cdata.ptr()), APE_DATA_COPY);

    return JS_TRUE;
}

static JSBool native_socket_close(JSContext *cx, unsigned argc, jsval *vp)
{
    JSObject *caller = JS_THIS_OBJECT(cx, vp);
    native_socket *nsocket = (native_socket *)JS_GetPrivate(caller);

    if (nsocket == NULL || nsocket->socket == NULL) {
        return JS_TRUE;
    }

    APE_socket_shutdown(nsocket->socket);

    return JS_TRUE;
}


static JSBool native_canvas_requestAnimationFrame(JSContext *cx,
    unsigned argc, jsval *vp)
{

    if (!JS_ConvertValue(cx, JS_ARGV(cx, vp)[0], JSTYPE_FUNCTION, &gfunc)) {
        return JS_TRUE;
    }
    JS_AddValueRoot(cx, &gfunc);
    return JS_TRUE;
}


static JSBool native_canvas_drawImage(JSContext *cx, unsigned argc, jsval *vp)
{
    JSObject *jsimage;
    NativeSkImage *image;
    double x, y, width, height;
    int sx, sy, swidth, sheight;
    int need_free = 0;

    if (argc == 9) {
         if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "oiiiidddd",
            &jsimage, &sx, &sy, &swidth, &sheight, &x, &y, &width, &height)) {
            return JS_TRUE;
        }
    } else {

        if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "odd/dd",
            &jsimage, &x, &y, &width, &height)) {
            return JS_TRUE;
        }
    }

    if (JS_InstanceOf(cx, jsimage, &canvas_class, NULL)) {
        image = new NativeSkImage(NSKIA_NATIVE->canvas);
        need_free = 1;

    } else if (!JS_InstanceOf(cx, jsimage, &image_class, NULL) ||
        (image = (class NativeSkImage *)JS_GetPrivate(jsimage)) == NULL) {
        return JS_TRUE;
    }

    switch(argc) {
        case 3:
            NSKIA_NATIVE->drawImage(image, x, y);
            break;
        case 5:
            NSKIA_NATIVE->drawImage(image, x, y, width, height);
            break;
        case 9:
            NSKIA_NATIVE->drawImage(image, sx, sy, swidth, sheight,
                x, y, width, height);
            break;
        default:
            break;
    }

    /* TODO: add cache (keep an SkBitmap for the canvas) */
    if (need_free) {
        delete image;
    }

    return JS_TRUE;
}

static JSBool native_canvas_measureText(JSContext *cx, unsigned argc,
    jsval *vp)
{
    JSString *text;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "S",
        &text)) {
        return JS_TRUE;
    }

    JSAutoByteString ctext(cx, text);

    JS_SET_RVAL(cx, vp, DOUBLE_TO_JSVAL(NSKIA_NATIVE->measureText(ctext.ptr(),
        strlen(ctext.ptr()))));

    return JS_TRUE;
}

static void native_socket_wrapper_onconnect(ape_socket *s, ape_global *ape)
{
    JSObject *this_socket = (JSObject *)s->ctx[0];
    JSContext *cx = (JSContext *)s->ctx[1];
    jsval onconnect, rval;
    native_socket *nsocket;

    if (this_socket == NULL || cx == NULL) {
        return;
    }

    nsocket = (native_socket *)JS_GetPrivate(this_socket);
    nsocket->socket = s;

    if (JS_GetProperty(cx, this_socket, "onconnect", &onconnect) &&
        JS_TypeOfValue(cx, onconnect) == JSTYPE_FUNCTION) {

        JS_CallFunctionValue(cx, this_socket, onconnect,
            0, NULL, &rval);
    }
}

static void native_socket_wrapper_read(ape_socket *s, ape_global *ape)
{
    JSObject *this_socket = (JSObject *)s->ctx[0];
    JSContext *cx = (JSContext *)s->ctx[1];
    jsval onread, rval, jdata;

    native_socket *nsocket;

    if (this_socket == NULL || cx == NULL) {
        return;
    }

    nsocket = (native_socket *)JS_GetPrivate(this_socket);

    if (nsocket->flags & NATIVE_SOCKET_ISBINARY) {
        JSObject *arrayBuffer = JS_NewArrayBuffer(cx, s->data_in.used);
        uint8_t *data = JS_GetArrayBufferData(arrayBuffer, cx);
        memcpy(data, s->data_in.data, s->data_in.used);

        jdata = OBJECT_TO_JSVAL(arrayBuffer);

    } else {
        jdata = STRING_TO_JSVAL(JS_NewStringCopyN(cx, (char *)s->data_in.data,
            s->data_in.used));        
    }

    if (JS_GetProperty(cx, this_socket, "onread", &onread) &&
        JS_TypeOfValue(cx, onread) == JSTYPE_FUNCTION) {

        JS_CallFunctionValue(cx, this_socket, onread,
            1, &jdata, &rval);
    }
}

static void native_socket_wrapper_disconnect(ape_socket *s, ape_global *ape)
{
    JSObject *this_socket = (JSObject *)s->ctx[0];
    JSContext *cx = (JSContext *)s->ctx[1];
    native_socket *nsocket = (native_socket *)s->ctx[2];
    jsval ondisconnect, rval;

    if (this_socket == NULL || cx == NULL) {
        return;
    }

    nsocket->socket = NULL;

    s->ctx[0] = NULL;
    s->ctx[1] = NULL;
    s->ctx[2] = NULL;

    if (JS_GetProperty(cx, this_socket, "ondisconnect", &ondisconnect) &&
        JS_TypeOfValue(cx, ondisconnect) == JSTYPE_FUNCTION) {

        JS_CallFunctionValue(cx, this_socket, ondisconnect,
            0, NULL, &rval);
    }
}

static JSBool native_Socket_constructor(JSContext *cx, unsigned argc, jsval *vp)
{
    JSString *host;
    unsigned int port;
    native_socket *nsocket;
    jsval isBinary = JSVAL_FALSE;

    JSObject *ret = JS_NewObjectForConstructor(cx, &socket_class, vp);

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "Su",
        &host, &port)) {
        return JS_TRUE;
    }
    JSAutoByteString chost(cx, host);

    nsocket = (native_socket *)malloc(sizeof(native_socket));

    nsocket->host   = JS_EncodeString(cx, host);
    nsocket->port   = port;
    nsocket->socket = NULL;
    nsocket->flags  = 0;
    JS_SetPrivate(ret, nsocket);

    /* TODO: JS_IsConstructing() */
    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(ret));

    JS_DefineFunctions(cx, ret, socket_funcs);
    JS_DefineProperties(cx, ret, Socket_props);

    JS_SetProperty(cx, ret, "isBinary", &isBinary);
    //JS_SetPropertyById(cx, ret, SOCKET_PROP_BINARY, &isBinary);

    

    return JS_TRUE;
}

static JSBool native_Image_constructor(JSContext *cx, unsigned argc, jsval *vp)
{
    JSObject *ret = JS_NewObjectForConstructor(cx, &image_class, vp);

    /* TODO: JS_IsConstructing() */
    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(ret));

    JS_DefineProperties(cx, ret, Image_props);
    return JS_TRUE;
}

static JSBool native_Canvas_constructor(JSContext *cx, unsigned argc, jsval *vp)
{
    int width, height;
    NativeSkia *CanvasObject;

    JSObject *ret = JS_NewObjectForConstructor(cx, &canvas_class, vp);

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "ii",
        &width, &height)) {
        return JS_TRUE;
    }

    CanvasObject = new NativeSkia();
    CanvasObject->bindOffScreen(width, height);

    JS_SetPrivate(ret, CanvasObject);

    JS_DefineFunctions(cx, ret, canvas_funcs);
    JS_DefineProperties(cx, ret, canvas_props);

    /* TODO: JS_IsConstructing() */
    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(ret));

    return JS_TRUE;
}

struct tpass
{
    JSContext *cx;
    jsval fn;
    JSFunction *nfn;
    JSString *str;
    NativeJS *njs;
    JSObject *obj;
};

struct thread_msg
{
    uint64_t *data;
    size_t nbytes;
    JSObject *callee;
};

static JSBool native_post_message(JSContext *cx, unsigned argc, jsval *vp)
{
    uint64_t *datap;
    size_t nbytes;
    struct tpass *pass = (struct tpass*)JS_GetContextPrivate(cx);
    NativeJS *njs = pass->njs;

    struct thread_msg *msg;

    if (!JS_WriteStructuredClone(cx, JS_ARGV(cx, vp)[0], &datap, &nbytes,
        NULL, NULL)) {
        printf("Failed to write strclone\n");
        /* TODO: exception */
        return JS_TRUE;
    }

    msg = new struct thread_msg;

    //JS_WriteStructuredClone(JSContext *cx, jsval v, uint64_t **datap,
    //size_t *nbytesp, const JSStructuredCloneCallbacks *optionalCallbacks, void *closure)

    msg->data   = datap;
    msg->nbytes = nbytes;
    msg->callee = pass->obj;

    njs->messages->postMessage(msg);

    return JS_TRUE;
}

static void *native_thread(void *arg)
{
    struct tpass *pass = (struct tpass *)arg;

    JSRuntime *rt;
    JSContext *tcx;
    jsval rval;
    JSObject *gbl;


    if ((rt = JS_NewRuntime(128L * 1024L * 1024L)) == NULL) {
        printf("Failed to init JS runtime\n");
        return NULL;
    }

    if ((tcx = JS_NewContext(rt, 8192)) == NULL) {
        printf("Failed to init JS context\n");
        return NULL;     
    }

    gbl = JS_NewGlobalObject(tcx, &global_class, NULL);


    JS_SetOptions(tcx, JSOPTION_VAROBJFIX | JSOPTION_METHODJIT |
        JSOPTION_TYPE_INFERENCE);
    JS_SetVersion(tcx, JSVERSION_LATEST);

    if (!JS_InitStandardClasses(tcx, gbl))
        return NULL;
    
    JS_SetErrorReporter(tcx, reportError);

    JS_SetGlobalObject(tcx, gbl);

    JS_DefineFunctions(tcx, gbl, glob_funcs_threaded);

    JSAutoByteString str(tcx, pass->str);

    /* Hold the parent cx */
    JS_SetContextPrivate(tcx, pass);

#if 0
    JSObject *ff = (JSObject *)pass->nfn;
    JSCrossCompartmentCall *call;
    if ((call = JS_EnterCrossCompartmentCall(tcx, ff)) == NULL) {
        printf("Failed to cross compart\n");
    }

    //JSAutoEnterCompartment ac;
    //ac.enter(tcx, (JSObject *)fn);
    if (JS_WrapObject(tcx, &ff) == JS_FALSE) {
        printf("failed to wrap\n");
    }
    printf("calling\n");
    //JS_CallFunction(JSContext *cx, JSObject *obj, JSFunction *fun, unsigned int argc, jsval *argv, jsval *rval)
    JS_CallFunction(tcx, gbl, (JSFunction *)ff, 0, NULL, &rval);
    /*if (JS_CallFunctionValue(tcx, gbl, pass->fn, 0, NULL, &rval) == JS_FALSE) {
        printf("failed\n");
    }*/
    JS_LeaveCrossCompartmentCall(call);
#endif
    JSFunction *cf = JS_CompileFunction(tcx, gbl, NULL, 0, NULL, str.ptr(),
        strlen(str.ptr()), NULL, 0);
    if (cf == NULL) {
        printf("Cant compile function\n");
        return NULL;
    }
    
    if (JS_CallFunction(tcx, gbl, cf, 0, NULL, &rval) == JS_FALSE) {
        printf("Got an error?\n");
    }

    free(pass);

    /* TODO: destroy context, runtime, etc... */

    return NULL;
}

static JSBool native_Thread_constructor(JSContext *cx, unsigned argc, jsval *vp)
{
    JSObject *ret = JS_NewObjectForConstructor(cx, &thread_class, vp);

    pthread_t currentThread;
    jsval func;
    //uint64_t *data;
    //size_t nbytes;

    struct tpass *pass = (struct tpass *)malloc(sizeof(*pass));

    pass->nfn = JS_ValueToFunction(cx, JS_ARGV(cx, vp)[0]);

    //JSString *src = JS_ValueToSource(cx, JS_ARGV(cx, vp)[0]);
    JSString *src = JS_DecompileFunctionBody(cx, pass->nfn, 0);

    pass->cx  = cx;
    pass->fn  = func;
    pass->str = src;
    pass->njs = NJS;
    pass->obj = ret;

    pthread_create(&currentThread, NULL,
                            native_thread, pass);

    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(ret));

    return JS_TRUE;
}

void NativeJS::callFrame()
{
    jsval rval;
    char fps[16];
    //JS_MaybeGC(cx);
    //JS_GC(JS_GetRuntime(cx));
    //NSKIA->save();
    if (gfunc != JSVAL_VOID) {
        JS_CallFunctionValue(cx, JS_GetGlobalObject(cx), gfunc, 0, NULL, &rval);
    }
    sprintf(fps, "%d fps", currentFPS);
    //printf("Fps : %s\n", fps);
    nskia->system(fps, 5, 300);
    //NSKIA->restore();
}

void NativeJS::mouseWheel(int xrel, int yrel, int x, int y)
{
#define EVENT_PROP(name, val) JS_DefineProperty(cx, event, name, \
    val, NULL, NULL, JSPROP_PERMANENT | JSPROP_READONLY)
    
    jsval rval, jevent, canvas, onwheel;
    JSObject *event;

    event = JS_NewObject(cx, &mouseEvent_class, NULL, NULL);
    JS_AddObjectRoot(cx, &event);

    EVENT_PROP("xrel", INT_TO_JSVAL(xrel));
    EVENT_PROP("yrel", INT_TO_JSVAL(yrel));
    EVENT_PROP("x", INT_TO_JSVAL(x));
    EVENT_PROP("y", INT_TO_JSVAL(y));

    jevent = OBJECT_TO_JSVAL(event);

    JS_GetProperty(cx, JS_GetGlobalObject(cx), "canvas", &canvas);

    if (JS_GetProperty(cx, JSVAL_TO_OBJECT(canvas), "onmousewheel", &onwheel) &&
        !JSVAL_IS_PRIMITIVE(onwheel) && 
        JS_ObjectIsCallable(cx, JSVAL_TO_OBJECT(onwheel))) {

        JS_CallFunctionValue(cx, event, onwheel, 1, &jevent, &rval);
    }


    JS_RemoveObjectRoot(cx, &event);    
}

void NativeJS::keyupdown(int keycode, int mod, int state, int repeat)
{
#define EVENT_PROP(name, val) JS_DefineProperty(cx, event, name, \
    val, NULL, NULL, JSPROP_PERMANENT | JSPROP_READONLY)
    
    JSObject *event;
    jsval jevent, onkeyupdown, canvas, rval;

    event = JS_NewObject(cx, &keyEvent_class, NULL, NULL);
    JS_AddObjectRoot(cx, &event);

    EVENT_PROP("keyCode", INT_TO_JSVAL(keycode));
    EVENT_PROP("altKey", BOOLEAN_TO_JSVAL(mod & NATIVE_KEY_ALT));
    EVENT_PROP("ctrlKey", BOOLEAN_TO_JSVAL(mod & NATIVE_KEY_CTRL));
    EVENT_PROP("shiftKey", BOOLEAN_TO_JSVAL(mod & NATIVE_KEY_SHIFT));
    EVENT_PROP("repeat", BOOLEAN_TO_JSVAL(repeat));

    jevent = OBJECT_TO_JSVAL(event);

    JS_GetProperty(cx, JS_GetGlobalObject(cx), "canvas", &canvas);

    if (JS_GetProperty(cx, JSVAL_TO_OBJECT(canvas),
        (state ? "onkeydown" : "onkeyup"), &onkeyupdown) &&
        !JSVAL_IS_PRIMITIVE(onkeyupdown) && 
        JS_ObjectIsCallable(cx, JSVAL_TO_OBJECT(onkeyupdown))) {

        JS_CallFunctionValue(cx, event, onkeyupdown, 1, &jevent, &rval);
    }    


    JS_RemoveObjectRoot(cx, &event);
}

void NativeJS::textInput(const char *data)
{
#define EVENT_PROP(name, val) JS_DefineProperty(cx, event, name, \
    val, NULL, NULL, JSPROP_PERMANENT | JSPROP_READONLY)

    JSObject *event;
    jsval jevent, ontextinput, canvas, rval;

    event = JS_NewObject(cx, &textEvent_class, NULL, NULL);
    JS_AddObjectRoot(cx, &event);

    EVENT_PROP("val",
        STRING_TO_JSVAL(JS_NewStringCopyN(cx, data, strlen(data))));

    jevent = OBJECT_TO_JSVAL(event);

    JS_GetProperty(cx, JS_GetGlobalObject(cx), "canvas", &canvas);

    if (JS_GetProperty(cx, JSVAL_TO_OBJECT(canvas), "ontextinput", &ontextinput) &&
        !JSVAL_IS_PRIMITIVE(ontextinput) && 
        JS_ObjectIsCallable(cx, JSVAL_TO_OBJECT(ontextinput))) {

        JS_CallFunctionValue(cx, event, ontextinput, 1, &jevent, &rval);
    }

    JS_RemoveObjectRoot(cx, &event);
}

void NativeJS::mouseClick(int x, int y, int state, int button)
{
#define EVENT_PROP(name, val) JS_DefineProperty(cx, event, name, \
    val, NULL, NULL, JSPROP_PERMANENT | JSPROP_READONLY)

    jsval rval, jevent;
    JSObject *event;

    jsval canvas, onclick;

    event = JS_NewObject(cx, &mouseEvent_class, NULL, NULL);
    JS_AddObjectRoot(cx, &event);

    EVENT_PROP("x", INT_TO_JSVAL(x));
    EVENT_PROP("y", INT_TO_JSVAL(y));
    EVENT_PROP("clientX", INT_TO_JSVAL(x));
    EVENT_PROP("clientY", INT_TO_JSVAL(y));
    EVENT_PROP("which", INT_TO_JSVAL(button));

    jevent = OBJECT_TO_JSVAL(event);

    JS_GetProperty(cx, JS_GetGlobalObject(cx), "canvas", &canvas);

    if (JS_GetProperty(cx, JSVAL_TO_OBJECT(canvas),
        (state ? "onmousedown" : "onmouseup"), &onclick) &&
        !JSVAL_IS_PRIMITIVE(onclick) && 
        JS_ObjectIsCallable(cx, JSVAL_TO_OBJECT(onclick))) {

        JS_CallFunctionValue(cx, event, onclick, 1, &jevent, &rval);
    }

    JS_RemoveObjectRoot(cx, &event);

}

void NativeJS::mouseMove(int x, int y, int xrel, int yrel)
{
#define EVENT_PROP(name, val) JS_DefineProperty(cx, event, name, \
    val, NULL, NULL, JSPROP_PERMANENT | JSPROP_READONLY)
    
    jsval rval, jevent, canvas, onmove;
    JSObject *event;

    event = JS_NewObject(cx, &mouseEvent_class, NULL, NULL);
    JS_AddObjectRoot(cx, &event);

    EVENT_PROP("x", INT_TO_JSVAL(x));
    EVENT_PROP("y", INT_TO_JSVAL(y));
    EVENT_PROP("xrel", INT_TO_JSVAL(xrel));
    EVENT_PROP("yrel", INT_TO_JSVAL(yrel));
    EVENT_PROP("clientX", INT_TO_JSVAL(x));
    EVENT_PROP("clientY", INT_TO_JSVAL(y));

    jevent = OBJECT_TO_JSVAL(event);

    JS_GetProperty(cx, JS_GetGlobalObject(cx), "canvas", &canvas);

    if (JS_GetProperty(cx, JSVAL_TO_OBJECT(canvas), "onmousemove", &onmove) &&
        !JSVAL_IS_PRIMITIVE(onmove) && 
        JS_ObjectIsCallable(cx, JSVAL_TO_OBJECT(onmove))) {

        JS_CallFunctionValue(cx, event, onmove, 1, &jevent, &rval);
    }


    JS_RemoveObjectRoot(cx, &event);
}

static JSBool native_load(JSContext *cx, unsigned argc, jsval *vp)
{
    JSString *script;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "S", &script)) {
        return JS_TRUE;
    }

    JSAutoByteString scriptstr(cx, script);

    if (!NJS->LoadScript(scriptstr.ptr())) {
        return JS_TRUE;
    }

    return JS_TRUE;
}

NativeJS::NativeJS()
{
    JSRuntime *rt;
    JSObject *gbl;

    static int isUTF8 = 0;
    gfunc = JSVAL_VOID;
    /* TODO: BUG */
    if (!isUTF8) {
        JS_SetCStringsAreUTF8();
        isUTF8 = 1;
    }
    //printf("New JS runtime\n");

    currentFPS = 0;

    if ((rt = JS_NewRuntime(128L * 1024L * 1024L)) == NULL) {
        printf("Failed to init JS runtime\n");
        return;
    }

    JS_SetGCParameter(rt, JSGC_MAX_BYTES, 0xffffffff);
    JS_SetGCParameter(rt, JSGC_MODE, JSGC_MODE_INCREMENTAL);
    JS_SetGCParameter(rt, JSGC_SLICE_TIME_BUDGET, 15);
    JS_SetGCParameterForThread(cx, JSGC_MAX_CODE_CACHE_BYTES, 16 * 1024 * 1024);

    //JS_SetNativeStackQuota(rt, 500000);

    if ((cx = JS_NewContext(rt, 8192)) == NULL) {
        printf("Failed to init JS context\n");
        return;     
    }

    JS_SetOptions(cx, JSOPTION_VAROBJFIX | JSOPTION_METHODJIT |
        JSOPTION_TYPE_INFERENCE);
    JS_SetVersion(cx, JSVERSION_LATEST);

    JS_SetErrorReporter(cx, reportError);

    gbl = JS_NewGlobalObject(cx, &global_class, NULL);

    if (!JS_InitStandardClasses(cx, gbl))
        return;


    /* TODO: HAS_CTYPE in clang */
    //JS_InitCTypesClass(cx, gbl);

    JS_SetGlobalObject(cx, gbl);
    JS_DefineFunctions(cx, gbl, glob_funcs);

    nskia = new NativeSkia();

    //JS_SetContextPrivate(cx, nskia);
    JS_SetRuntimePrivate(rt, this);

    LoadCanvasObject(nskia);

    messages = new NativeSharedMessages();

    //animationframeCallbacks = ape_new_pool(sizeof(ape_pool_t), 8);
}

NativeJS::~NativeJS()
{
    JSRuntime *rt;
    rt = JS_GetRuntime(cx);

    ape_global *net = (ape_global *)JS_GetContextPrivate(cx);

    /* clear all non protected timers */
    del_timers_unprotected(&net->timersng);

    JS_DestroyContext(cx);
    JS_DestroyRuntime(rt);

    delete messages;
    //delete nskia; /* TODO: why is that commented out? */
    // is it covered by Canvas_Finalize()?
}

void NativeJS::bufferSound(int16_t *data, int len)
{
    jsval canvas, onwheel;

    if (JS_GetProperty(cx, JSVAL_TO_OBJECT(canvas), "onmousewheel", &onwheel) &&
        !JSVAL_IS_PRIMITIVE(onwheel) && 
        JS_ObjectIsCallable(cx, JSVAL_TO_OBJECT(onwheel))) {

       // JS_CallFunctionValue(cx, event, onwheel, 0, NULL, &rval);
    }
}

static int Native_handle_messages(void *arg)
{
#define EVENT_PROP(name, val) JS_DefineProperty(cx, event, name, \
    val, NULL, NULL, JSPROP_PERMANENT | JSPROP_READONLY)

    NativeJS *njs = (NativeJS *)arg;
    JSContext *cx = njs->cx;
    struct thread_msg *ptr;
    jsval onmessage, jevent, rval;

    JSObject *event;

    while ((ptr = (struct thread_msg *)njs->messages->readMessage()) != NULL) {

        if (JS_GetProperty(cx, ptr->callee, "onmessage", &onmessage) &&
            !JSVAL_IS_PRIMITIVE(onmessage) && 
            JS_ObjectIsCallable(cx, JSVAL_TO_OBJECT(onmessage))) {

            jsval inval;

            if (!JS_ReadStructuredClone(cx, ptr->data, ptr->nbytes,
                JS_STRUCTURED_CLONE_VERSION, &inval, NULL, NULL)) {

                printf("Failed to read input data (readMessage)\n");

                continue;
            }

            event = JS_NewObject(cx, &messageEvent_class, NULL, NULL);
            JS_AddObjectRoot(cx, &event);

            EVENT_PROP("message", inval);

            jevent = OBJECT_TO_JSVAL(event);
            JS_CallFunctionValue(cx, event, onmessage, 1, &jevent, &rval);
            JS_RemoveObjectRoot(cx, &event);            

        }

    }

    return 1;
}

void NativeJS::bindNetObject(ape_global *net)
{
    JS_SetContextPrivate(cx, net);

    ape_timer *timer = add_timer(&net->timersng, 1,
        Native_handle_messages, this);

    timer->flags &= ~APE_TIMER_IS_PROTECTED;
}

int NativeJS::LoadScript(const char *filename)
{
    uint32_t oldopts;

    JSObject *gbl = JS_GetGlobalObject(cx);
    oldopts = JS_GetOptions(cx);

    JS_SetOptions(cx, oldopts | JSOPTION_COMPILE_N_GO | JSOPTION_NO_SCRIPT_RVAL);

    JSScript *script = JS_CompileUTF8File(cx, gbl, filename);

    JS_SetOptions(cx, oldopts);

    if (script == NULL || !JS_ExecuteScript(cx, gbl, script, NULL)) {
        return 0;
    }
    
    return 1;
}

void NativeJS::LoadCanvasObject(NativeSkia *currentSkia)
{
    JSObject *gbl = JS_GetGlobalObject(cx);
    JSObject *canvasObj;

    canvasObj = JS_DefineObject(cx, gbl, "canvas", &canvas_class, NULL, 0);
    JS_DefineFunctions(cx, canvasObj, canvas_funcs);

    JS_DefineProperties(cx, canvasObj, canvas_props);

    JS_SetPrivate(canvasObj, currentSkia);

    /* Socket (client) object */
    JS_InitClass(cx, gbl, NULL, &socket_class, native_Socket_constructor,
        0, NULL, NULL, NULL, NULL);

    /* Image object */
    JS_InitClass(cx, gbl, NULL, &image_class, native_Image_constructor,
        0, NULL, NULL, NULL, NULL);

    /* Offscreen Canvas object */
    JS_InitClass(cx, gbl, NULL, &canvas_class, native_Canvas_constructor,
        2, NULL, NULL, NULL, NULL);   

    /* Threaded object */
    JS_InitClass(cx, gbl, NULL, &thread_class, native_Thread_constructor,
        0, NULL, NULL, NULL, NULL);
}

void NativeJS::gc()
{
    JS_GC(JS_GetRuntime(cx));
}

static JSBool native_set_timeout(JSContext *cx, unsigned argc, jsval *vp)
{
    struct _native_sm_timer *params;

    int ms, i;
    JSObject *obj = JS_THIS_OBJECT(cx, vp);

    params = (struct _native_sm_timer *)JS_malloc(cx, sizeof(*params));

    if (params == NULL) {
        return JS_FALSE;
    }

    params->cx = cx;
    params->global = obj;
    params->argc = argc-2;
    params->cleared = 0;
    params->timer = NULL;
    params->timerng = NULL;
    params->ms = 0;

    params->argv = (argc-2 ? (jsval *)JS_malloc(cx, sizeof(*params->argv) * argc-2) : NULL);

    if (!JS_ConvertValue(cx, JS_ARGV(cx, vp)[0], JSTYPE_FUNCTION, &params->func)) {
        return JS_TRUE;
    }

    if (!JS_ConvertArguments(cx, 1, &JS_ARGV(cx, vp)[1], "i", &ms)) {
        return JS_TRUE;
    }

    JS_AddValueRoot(cx, &params->func);

    for (i = 0; i < (int)argc-2; i++) {
        params->argv[i] = JS_ARGV(cx, vp)[i+2];
    }

    params->timerng = add_timer(&((ape_global *)JS_GetContextPrivate(cx))->timersng,
        ms, native_timerng_wrapper,
        (void *)params);

    params->timerng->flags &= ~APE_TIMER_IS_PROTECTED;

    JS_SET_RVAL(cx, vp, INT_TO_JSVAL(params->timerng->identifier));

    return JS_TRUE;
}

static JSBool native_set_interval(JSContext *cx, unsigned argc, jsval *vp)
{
    struct _native_sm_timer *params;
    int ms, i;
    JSObject *obj = JS_THIS_OBJECT(cx, vp);

    params = (struct _native_sm_timer *)JS_malloc(cx, sizeof(*params));

    if (params == NULL) {
        return JS_FALSE;
    }

    params->cx = cx;
    params->global = obj;
    params->argc = argc-2;
    params->cleared = 0;
    params->timer = NULL;

    params->argv = (argc-2 ? (jsval *)JS_malloc(cx, sizeof(*params->argv) * argc-2) : NULL);

    if (!JS_ConvertValue(cx, JS_ARGV(cx, vp)[0], JSTYPE_FUNCTION, &params->func)) {
        return JS_TRUE;
    }

    if (!JS_ConvertArguments(cx, 1, &JS_ARGV(cx, vp)[1], "i", &ms)) {
        return JS_TRUE;
    }

    params->ms = ms;

    JS_AddValueRoot(cx, &params->func);

    for (i = 0; i < (int)argc-2; i++) {
        params->argv[i] = JS_ARGV(cx, vp)[i+2];
    }

    params->timerng = add_timer(&((ape_global *)JS_GetContextPrivate(cx))->timersng,
        ms, native_timerng_wrapper,
        (void *)params);

    params->timerng->flags &= ~APE_TIMER_IS_PROTECTED;

    JS_SET_RVAL(cx, vp, INT_TO_JSVAL(params->timerng->identifier));

    return JS_TRUE; 
}

static JSBool native_clear_timeout(JSContext *cx, unsigned argc, jsval *vp)
{
    unsigned int identifier;

    if (!JS_ConvertArguments(cx, 1, JS_ARGV(cx, vp), "i", &identifier)) {
        return JS_TRUE;
    }

    clear_timer_by_id(&((ape_global *)JS_GetContextPrivate(cx))->timersng,
        identifier, 0);

    /* TODO: remove root / clear params */

    return JS_TRUE;    
}

static int native_timerng_wrapper(void *arg)
{
    jsval rval;
    struct _native_sm_timer *params = (struct _native_sm_timer *)arg;

    JS_CallFunctionValue(params->cx, params->global, params->func,
        params->argc, params->argv, &rval);

    //timers_stats_print(&((ape_global *)JS_GetContextPrivate(params->cx))->timersng);
    if (params->ms == 0) {
        JS_RemoveValueRoot(params->cx, &params->func);

        if (params->argv != NULL) {
            free(params->argv);
        }
        free(params);        
    }
    return params->ms;
}

static void native_timer_wrapper(struct _native_sm_timer *params, int *last)
{
    jsval rval;

    if (!params->cleared) {
        JS_CallFunctionValue(params->cx, params->global, params->func,
            params->argc, params->argv, &rval);
    }

    if (params->cleared && !*last) { /* JS_CallFunctionValue can set params->Cleared to true */
        *last = 1;
    }

    if (*last) {
        JS_RemoveValueRoot(params->cx, &params->func);

        if (params->argv != NULL) {
            free(params->argv);
        }
        free(params);
    }

}


