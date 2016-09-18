#ifndef binding_jsdocument_h__
#define binding_jsdocument_h__

#include "Binding/ClassMapper.h"

class SkTypeface;

namespace Nidium {
namespace Binding {

// {{{ NidiumFont
class NidiumFont
{
public:
    SkTypeface *m_Typeface;

    enum Style
    {
        kFontStyle_Bold,
        kFontStyle_Normal,
        kFontStyle_Italic
    } m_Style;

    int m_Weight;

    NidiumFont *m_Next;
};
// }}}

// {{{ JSDocument
class JSDocument : public ClassMapper<JSDocument>
{
public:
    virtual ~JSDocument(){};

    static JSFunctionSpec *ListMethods();

    static bool m_ShowFPS;
    bool populateStyle(JSContext *cx,
                       const char *data,
                       size_t len,
                       const char *filename);
    static JSObject *RegisterObject(JSContext *cx);

    JS::Heap<JSObject *> m_Stylesheet;

    Core::Hash<NidiumFont *> m_Fonts{64000};

    bool loadFont(const char *path,
                  const char *name,
                  int weight        = 400,
                  NidiumFont::Style = NidiumFont::kFontStyle_Normal);

    SkTypeface *getFont(char *name);
protected:

    NIDIUM_DECL_JSCALL(run);
    NIDIUM_DECL_JSCALL(showFPS);
    NIDIUM_DECL_JSCALL(setPasteBuffer);
    NIDIUM_DECL_JSCALL(getPasteBuffer);
    NIDIUM_DECL_JSCALL(loadFont);
    NIDIUM_DECL_JSCALL(getCanvasById);
    NIDIUM_DECL_JSCALL(getScreenData);
    NIDIUM_DECL_JSCALL(toDataArray);
    NIDIUM_DECL_JSCALL(parseNML);
};
// }}}

} // namespace Binding
} // namespace Nidium

#endif
