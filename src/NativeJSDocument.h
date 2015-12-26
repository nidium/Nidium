#ifndef nativejsdocument_h__
#define nativejsdocument_h__

#include <NativeJSExposer.h>

class SkTypeface;

class nativefont {
public:
    SkTypeface *m_Typeface;

    enum Style {
        kNativeFontBold,
        kNativeFontNormal,
        kNativeFontItalic
    } m_Style;

    int m_Weight;

    nativefont *m_Next;
};


class NativeJSdocument : public NativeJSExposer<NativeJSdocument>
{
  public:
    NativeJSdocument(JS::HandleObject obj, JSContext *cx) :
    NativeJSExposer<NativeJSdocument>(obj, cx, false),
        m_Stylesheet(NULL), m_Fonts(256000) {};
    ~NativeJSdocument() {};

    static bool m_ShowFPS;
    bool populateStyle(JSContext *cx, const char *data,
        size_t len, const char *filename);
    static JSObject *registerObject(JSContext *cx);
    static const char *getJSObjectName() {
        return "document";
    }

    static JSClass *m_JsClass;

    JS::Heap<JSObject *> m_Stylesheet;
    NativeHash<nativefont *>m_Fonts;

    bool loadFont(const char *path, const char *name, int weight = 400,
        nativefont::Style = nativefont::kNativeFontNormal);

    SkTypeface *getFont(char *name);
};

#endif

