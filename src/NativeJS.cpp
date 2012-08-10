#include "NativeJS.h"
#include "NativeSkia.h"
#include "NativeSkGradient.h"
#include "NativeSkImage.h"
#include <stdio.h>
#include <jsapi.h>
#include <jsprf.h>

enum {
    CANVAS_PROP_FILLSTYLE = 1,
    CANVAS_PROP_STROKESTYLE,
    CANVAS_PROP_LINEWIDTH,
    CANVAS_PROP_GLOBALALPHA,
    CANVAS_PROP_LINECAP,
    CANVAS_PROP_LINEJOIN,
    CANVAS_PROP_WIDTH,
    CANVAS_PROP_HEIGHT,
    CANVAS_PROP_GLOBALCOMPOSITEOPERATION
};

#define NSKIA ((class NativeSkia *)JS_GetContextPrivate(cx))
#define NJS ((class NativeJS *)JS_GetRuntimePrivate(JS_GetRuntime(cx)))

static JSClass global_class = {
    "_GLOBAL", JSCLASS_GLOBAL_FLAGS | JSCLASS_IS_GLOBAL,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, NULL,
    JSCLASS_NO_OPTIONAL_MEMBERS
};

static JSClass canvas_class = {
    "Canvas", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, NULL,
    JSCLASS_NO_OPTIONAL_MEMBERS
};

static JSClass canvasGradient_class = {
    "CanvasGradient", JSCLASS_HAS_PRIVATE,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, NULL,
    JSCLASS_NO_OPTIONAL_MEMBERS
};

static JSClass mouseEvent_class = {
    "MouseEvent", 0,
    JS_PropertyStub, JS_PropertyStub, JS_PropertyStub, JS_StrictPropertyStub,
    JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, NULL,
    JSCLASS_NO_OPTIONAL_MEMBERS
};

jsval gfunc  = JSVAL_VOID;
jsval gmove  = JSVAL_VOID;
jsval gclick = JSVAL_VOID;

/******** Natives ********/
static JSBool Print(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_load(JSContext *cx, unsigned argc, jsval *vp);
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
static JSBool native_canvas_clip(JSContext *cx, unsigned argc, jsval *vp);
static JSBool native_canvas_createLinearGradient(JSContext *cx,
    unsigned argc, jsval *vp);
static JSBool native_canvas_createRadialGradient(JSContext *cx,
    unsigned argc, jsval *vp);
static JSBool native_canvasGradient_addColorStop(JSContext *cx,
    unsigned argc, jsval *vp);
static JSBool native_canvas_requestAnimationFrame(JSContext *cx,
    unsigned argc, jsval *vp);
static JSBool native_canvas_mouseMove(JSContext *cx,
    unsigned argc, jsval *vp);
static JSBool native_canvas_mouseClick(JSContext *cx,
    unsigned argc, jsval *vp);
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

static JSPropertySpec canvas_props[] = {
    {"fillStyle", CANVAS_PROP_FILLSTYLE, JSPROP_PERMANENT, NULL,
        native_canvas_prop_set},
    {"strokeStyle", CANVAS_PROP_STROKESTYLE, JSPROP_PERMANENT, NULL,
        native_canvas_prop_set},
    {"lineWidth", CANVAS_PROP_LINEWIDTH, JSPROP_PERMANENT, NULL,
        native_canvas_prop_set},
    {"globalAlpha", CANVAS_PROP_GLOBALALPHA, JSPROP_PERMANENT, NULL,
        native_canvas_prop_set},
    {"globalCompositeOperation", CANVAS_PROP_GLOBALCOMPOSITEOPERATION, JSPROP_PERMANENT, NULL,
        native_canvas_prop_set},
    {"lineCap", CANVAS_PROP_LINECAP, JSPROP_PERMANENT, NULL,
        native_canvas_prop_set},
    {"lineJoin", CANVAS_PROP_LINEJOIN, JSPROP_PERMANENT, NULL,
        native_canvas_prop_set},
    {"width", CANVAS_PROP_WIDTH, JSPROP_PERMANENT, native_canvas_prop_get,
        NULL},
    {"height", CANVAS_PROP_HEIGHT, JSPROP_PERMANENT, native_canvas_prop_get,
        NULL},
    {NULL}
};
/*************************/



static JSFunctionSpec glob_funcs[] = {
    
    JS_FN("echo", Print, 0, 0),
    JS_FN("load", native_load, 1, 0),

    JS_FS_END
};

static JSFunctionSpec gradient_funcs[] = {
    
    JS_FN("addColorStop", native_canvasGradient_addColorStop, 2, 0),

    JS_FS_END
};

static JSFunctionSpec canvas_funcs[] = {
    
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
    JS_FN("requestAnimationFrame", native_canvas_requestAnimationFrame, 1, 0),
    JS_FN("mouseMove", native_canvas_mouseMove, 1, 0),
    JS_FN("mouseClick", native_canvas_mouseClick, 1, 0),
    JS_FN("drawImage", native_canvas_drawImage, 0, 0),
    JS_FN("measureText", native_canvas_measureText, 1, 0),
    JS_FS_END
};

static JSBool native_canvas_stub(JSContext *cx, unsigned argc, jsval *vp)
{
    return JS_TRUE;
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
    switch(JSID_TO_INT(id)) {
        case CANVAS_PROP_WIDTH:
        {
            *vp = INT_TO_JSVAL(NSKIA->getWidth());
        }
        break;
        case CANVAS_PROP_HEIGHT:
        {
            *vp = INT_TO_JSVAL(NSKIA->getHeight());
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

    switch(JSID_TO_INT(id)) {
        case CANVAS_PROP_FILLSTYLE:
        {
            if (JSVAL_IS_STRING(*vp)) {
                JSAutoByteString colorName(cx, JSVAL_TO_STRING(*vp));
                NSKIA->setFillColor(colorName.ptr());
            } else if (!JSVAL_IS_PRIMITIVE(*vp) && 
                JS_InstanceOf(cx, JSVAL_TO_OBJECT(*vp),
                    &canvasGradient_class, NULL)) {

                NativeSkGradient *gradient = (class NativeSkGradient *)
                                            JS_GetPrivate(JSVAL_TO_OBJECT(*vp));

                NSKIA->setFillColor(gradient);

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
                NSKIA->setStrokeColor(colorName.ptr());
            } else if (!JSVAL_IS_PRIMITIVE(*vp) && 
                JS_InstanceOf(cx, JSVAL_TO_OBJECT(*vp),
                    &canvasGradient_class, NULL)) {

                NativeSkGradient *gradient = (class NativeSkGradient *)
                                            JS_GetPrivate(JSVAL_TO_OBJECT(*vp));

                NSKIA->setStrokeColor(gradient);

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
            NSKIA->setLineWidth(ret);
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
            NSKIA->setGlobalAlpha(ret);
        }
        break;
        case CANVAS_PROP_GLOBALCOMPOSITEOPERATION:
        {
            if (!JSVAL_IS_STRING(*vp)) {
                *vp = JSVAL_VOID;

                return JS_TRUE;
            }
            JSAutoByteString composite(cx, JSVAL_TO_STRING(*vp));
            NSKIA->setGlobalComposite(composite.ptr());            
        }
        break;
        case CANVAS_PROP_LINECAP:
        {
            if (!JSVAL_IS_STRING(*vp)) {
                *vp = JSVAL_VOID;

                return JS_TRUE;
            }
            JSAutoByteString lineCap(cx, JSVAL_TO_STRING(*vp));
            NSKIA->setLineCap(lineCap.ptr());                
        }
        break;
        case CANVAS_PROP_LINEJOIN:
        {
            if (!JSVAL_IS_STRING(*vp)) {
                *vp = JSVAL_VOID;

                return JS_TRUE;
            }
            JSAutoByteString lineJoin(cx, JSVAL_TO_STRING(*vp));
            NSKIA->setLineJoin(lineJoin.ptr());                
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

    NSKIA->drawRect(x, y, width+x, height+y, 0);

    return JS_TRUE;
}

static JSBool native_canvas_strokeRect(JSContext *cx, unsigned argc, jsval *vp)
{
    double x, y, width, height;
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "dddd", &x, &y, &width, &height)) {
        return JS_TRUE;
    }

    NSKIA->drawRect(x, y, width+x, height+y, 1);

    return JS_TRUE;
}

static JSBool native_canvas_clearRect(JSContext *cx, unsigned argc, jsval *vp)
{
    int x, y, width, height;
    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "iiii", &x, &y, &width, &height)) {
        return JS_TRUE;
    }

    NSKIA->clearRect(x, y, width+x, height+y);

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

    NSKIA->drawText(text.ptr(), x, y);

    return JS_TRUE;
}


static JSBool native_canvas_beginPath(JSContext *cx, unsigned argc, jsval *vp)
{
    NSKIA->beginPath();

    return JS_TRUE;
}

static JSBool native_canvas_moveTo(JSContext *cx, unsigned argc, jsval *vp)
{
    double x, y;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "dd", &x, &y)) {
        return JS_TRUE;
    }

    NSKIA->moveTo(x, y);

    return JS_TRUE;
}

static JSBool native_canvas_lineTo(JSContext *cx, unsigned argc, jsval *vp)
{
    double x, y;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "dd", &x, &y)) {
        return JS_TRUE;
    }

    NSKIA->lineTo(x, y);

    return JS_TRUE;
}

static JSBool native_canvas_fill(JSContext *cx, unsigned argc, jsval *vp)
{
    NSKIA->fill();

    return JS_TRUE;
}

static JSBool native_canvas_stroke(JSContext *cx, unsigned argc, jsval *vp)
{
    NSKIA->stroke();

    return JS_TRUE;
}
static JSBool native_canvas_closePath(JSContext *cx, unsigned argc, jsval *vp)
{
    NSKIA->closePath();

    return JS_TRUE;
}

static JSBool native_canvas_clip(JSContext *cx, unsigned argc, jsval *vp)
{
    NSKIA->clip();

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

    NSKIA->arc(x, y, radius, startAngle, endAngle, CCW);

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

    NSKIA->quadraticCurveTo(cpx, cpy, x, y);

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

    NSKIA->bezierCurveTo(cpx, cpy, cpx2, cpy2, x, y);

    return JS_TRUE;
}

static JSBool native_canvas_rotate(JSContext *cx, unsigned argc, jsval *vp)
{
    double angle;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "d", &angle)) {
        return JS_TRUE;
    }

    NSKIA->rotate(angle);

    return JS_TRUE;
}

static JSBool native_canvas_scale(JSContext *cx, unsigned argc, jsval *vp)
{
    double x, y;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "dd", &x, &y)) {
        return JS_TRUE;
    }

    NSKIA->scale(x, y);

    return JS_TRUE;
}

static JSBool native_canvas_translate(JSContext *cx, unsigned argc, jsval *vp)
{
    double x, y;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "dd", &x, &y)) {
        return JS_TRUE;
    }

    NSKIA->translate(x, y);

    return JS_TRUE;
}

static JSBool native_canvas_transform(JSContext *cx, unsigned argc, jsval *vp)
{
    double scalex, skewx, skewy, scaley, translatex, translatey;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "dddddd",
        &scalex, &skewx, &skewy, &scaley, &translatex, &translatey)) {
        return JS_TRUE;
    }

    NSKIA->transform(scalex, skewx, skewy, scaley,
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

    NSKIA->transform(scalex, skewx, skewy, scaley,
        translatex, translatey, 1);

    return JS_TRUE;
}

static JSBool native_canvas_save(JSContext *cx, unsigned argc, jsval *vp)
{
    NSKIA->save();

    return JS_TRUE;
}

static JSBool native_canvas_restore(JSContext *cx, unsigned argc, jsval *vp)
{
    NSKIA->restore();

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

static JSBool native_canvas_requestAnimationFrame(JSContext *cx,
    unsigned argc, jsval *vp)
{

    if (!JS_ConvertValue(cx, JS_ARGV(cx, vp)[0], JSTYPE_FUNCTION, &gfunc)) {
        return JS_TRUE;
    }
    JS_AddValueRoot(cx, &gfunc);
    return JS_TRUE;
}

static JSBool native_canvas_mouseMove(JSContext *cx, unsigned argc, jsval *vp)
{

    if (!JS_ConvertValue(cx, JS_ARGV(cx, vp)[0], JSTYPE_FUNCTION, &gmove)) {
        return JS_TRUE;
    }

    JS_AddValueRoot(cx, &gmove);
    return JS_TRUE;
}

static JSBool native_canvas_mouseClick(JSContext *cx, unsigned argc, jsval *vp)
{

    if (!JS_ConvertValue(cx, JS_ARGV(cx, vp)[0], JSTYPE_FUNCTION, &gclick)) {
        return JS_TRUE;
    }

    JS_AddValueRoot(cx, &gclick);

    return JS_TRUE;
}

static JSBool native_canvas_drawImage(JSContext *cx, unsigned argc, jsval *vp)
{
    NSKIA->drawImage();

    return JS_TRUE;
}

static JSBool native_canvas_measureText(JSContext *cx, unsigned argc,
    jsval *vp)
{
    JSString *text;

    if (!JS_ConvertArguments(cx, argc, JS_ARGV(cx, vp), "S",
        &text)) {
        JS_SET_RVAL(cx, vp, JSVAL_ZERO);
        return JS_TRUE;
    }

    JSAutoByteString ctext(cx, text);

    JS_SET_RVAL(cx, vp, DOUBLE_TO_JSVAL(NSKIA->measureText(ctext.ptr(),
        strlen(ctext.ptr()))));


    return JS_TRUE;
}

void NativeJS::callFrame()
{
    jsval rval;
    char fps[16];
    //JS_MaybeGC(cx);
    //JS_GC(JS_GetRuntime(cx));
    if (gfunc != JSVAL_VOID) {
        JS_CallFunctionValue(cx, JS_GetGlobalObject(cx), gfunc, 0, NULL, &rval);
    }
    sprintf(fps, "%d fps", currentFPS);
    NSKIA->system(fps, 5, 15);
}

void NativeJS::mouseClick(int x, int y, int state, int button)
{
#define EVENT_PROP(name, val) JS_DefineProperty(cx, event, name, \
    val, NULL, NULL, JSPROP_PERMANENT | JSPROP_READONLY)

    jsval rval, jevent;
    JSObject *event;
    char evname[16];

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

    if (button == 4 || button == 5) {
        strcpy(evname, "onmousewheel");
    } else if (state) {
        strcpy(evname, "onmousedown");
    } else {
        strcpy(evname, "onmouseup");
    }

    if (JS_GetProperty(cx, JSVAL_TO_OBJECT(canvas), evname, &onclick) &&
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

    /* TODO: BUG */
    //JS_SetCStringsAreUTF8();

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

    JS_SetNativeStackQuota(rt, 500000);

    if ((cx = JS_NewContext(rt, 8192)) == NULL) {
        printf("Failed to init JS context\n");
        return;     
    }

    JS_SetOptions(cx, JSOPTION_VAROBJFIX | JSOPTION_METHODJIT | JSOPTION_TYPE_INFERENCE | JSOPTION_METHODJIT_ALWAYS);
    JS_SetVersion(cx, JSVERSION_LATEST);

    JS_SetErrorReporter(cx, reportError);

    gbl = JS_NewGlobalObject(cx, &global_class, NULL);

    if (!JS_InitStandardClasses(cx, gbl))
        return;

    JS_SetGlobalObject(cx, gbl);
    JS_DefineFunctions(cx, gbl, glob_funcs);

    LoadCanvasObject();

    nskia = new NativeSkia();

    JS_SetContextPrivate(cx, nskia);
    JS_SetRuntimePrivate(rt, this);

    //animationframeCallbacks = ape_new_pool(sizeof(ape_pool_t), 8);
}

NativeJS::~NativeJS()
{
    JSRuntime *rt;
    rt = JS_GetRuntime(cx);
    JS_DestroyContext(cx);
    JS_DestroyRuntime(rt);
    delete nskia;
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

void NativeJS::LoadCanvasObject()
{
    JSObject *gbl = JS_GetGlobalObject(cx);
    JSObject *canvasObj;

    canvasObj = JS_DefineObject(cx, gbl, "canvas", &canvas_class, NULL, 0);
    JS_DefineFunctions(cx, canvasObj, canvas_funcs);

    JS_DefineProperties(cx, canvasObj, canvas_props);

}

void NativeJS::gc()
{
    JS_GC(JS_GetRuntime(cx));
}


