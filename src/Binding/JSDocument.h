#ifndef binding_jsdocument_h__
#define binding_jsdocument_h__

#include <Binding/JSExposer.h>

class SkTypeface;

namespace Nidium {
namespace Binding {

// {{{ NidiumFont
class NidiumFont
{
  public:
    SkTypeface *m_Typeface;

    enum Style {
        kNativeFontBold,
        kNativeFontNormal,
        kNativeFontItalic
    } m_Style;

    int m_Weight;

    NidiumFont *m_Next;
};
// }}}

// {{{ JSDocument
class JSDocument : public JSExposer<JSDocument>
{
  public:
    JSDocument(JS::HandleObject obj, JSContext *cx) :
    JSExposer<JSDocument>(obj, cx, false),
    m_Fonts(256000) {};
    ~JSDocument() {};

    static bool m_ShowFPS;
    bool populateStyle(JSContext *cx, const char *data,
        size_t len, const char *filename);
    static JSObject *RegisterObject(JSContext *cx);
    static const char *GetJSObjectName() {
        return "document";
    }

    static JSClass *jsclass;

    JS::Heap<JSObject *> m_Stylesheet;

    Core::Hash<NidiumFont *>m_Fonts;

    bool loadFont(const char *path, const char *name, int weight = 400,
        NidiumFont::Style = NidiumFont::kNativeFontNormal);

    SkTypeface *getFont(char *name);
};
// }}}

} // namespace Binding
} // namespace Nidium

#endif

