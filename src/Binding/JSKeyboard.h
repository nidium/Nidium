
/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef binding_jskeyboard_h__
#define binding_jskeyboard_h__

#include "Binding/ClassMapper.h"

using Nidium::Binding::ClassMapper;

namespace Nidium {
namespace Binding {

class JSKeyboard : public ClassMapper<JSKeyboard>
{
public:
    // Always keep in sync with platform implementation code
    enum KeyboardOptions {
        kOption_Normal      = 1 << 0,
        kOption_Numeric     = 1 << 1,
        kOption_TextOnly    = 1 << 2
    };

    JSKeyboard();
    static void RegisterObject(JSContext *cx);
    static JSObject *RegisterModule(JSContext *cx);
    static JSFunctionSpec *ListStaticMethods();
    static JSConstDoubleSpec *ListConstDoubles();

protected:
    NIDIUM_DECL_JSCALL_STATIC(show);
    NIDIUM_DECL_JSCALL_STATIC(hide);
};

} // namespace Binding
} // namespce Nidium

#endif
