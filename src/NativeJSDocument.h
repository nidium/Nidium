#ifndef nativejsdocument_h__
#define nativejsdocument_h__

#include <NativeJSExposer.h>

class SkTypeface;

class nativefont {
public:
    SkTypeface *typeface;

    enum Style {
        kNativeFontBold,
        kNativeFontNormal,
        kNativeFontItalic
    } style;

    int weight;

    nativefont *next;
};


class NativeJSdocument : public NativeJSExposer<NativeJSdocument>
{
  public:
    NativeJSdocument(JSObject *obj, JSContext *cx) :
    NativeJSExposer<NativeJSdocument>(obj, cx, false),
        m_Fonts(256000) {};
    ~NativeJSdocument() {};

    static bool showFPS;
    bool populateStyle(JSContext *cx, const char *data,
        size_t len, const char *filename);
    static JSObject *registerObject(JSContext *cx);
    static const char *getJSObjectName() {
        return "document";
    }

    static JSClass *jsclass;

    JSObject *stylesheet;
    NativeHash<nativefont *>m_Fonts;

    bool loadFont(const char *path, const char *name, int weight = 400,
        nativefont::Style = nativefont::kNativeFontNormal);

    SkTypeface *getFont(char *name);
};

#endif

