#ifndef nidium_jsdocument_h__
#define nidium_jsdocument_h__

#include <Binding/JSExposer.h>

class SkTypeface;

namespace Nidium {
namespace Binding {

class nativefont
{
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


class NativeJSdocument : public JSExposer<NativeJSdocument>
{
  public:
    NativeJSdocument(JS::HandleObject obj, JSContext *cx) :
    JSExposer<NativeJSdocument>(obj, cx, false),
    m_Fonts(256000) {};
    ~NativeJSdocument() {};

    static bool m_ShowFPS;
    bool populateStyle(JSContext *cx, const char *data,
        size_t len, const char *filename);
    static JSObject *registerObject(JSContext *cx);
    static const char *getJSObjectName() {
        return "document";
    }

    static JSClass *jsclass;

    JS::Heap<JSObject *> m_Stylesheet;

    Core::Hash<nativefont *>m_Fonts;

    bool loadFont(const char *path, const char *name, int weight = 400,
        nativefont::Style = nativefont::kNativeFontNormal);

    SkTypeface *getFont(char *name);
};

} // namespace Nidium
} // namespace Binding

#endif

