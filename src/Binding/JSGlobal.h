/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef binding_jsglobal_h__
#define binding_jsglobal_h__

#include "Binding/ClassMapper.h"
#include "Binding/NidiumJS.h"
#include <jsapi.h>

namespace Nidium {
namespace Binding {

class JSGlobal : public ClassMapper<JSGlobal>
{
public:
    JSGlobal(NidiumJS *njs) : m_JS(njs) {}
    virtual ~JSGlobal(){}

    static JSFunctionSpec *ListMethods();
    static JSPropertySpec *ListProperties();
    static void RegisterObject(JSContext *cx, JS::HandleObject global,
        NidiumJS *njs);

    static inline JSGlobal *GetInstance(JSObject *obj, JSContext *cx)
    {
        if (cx == nullptr) {
            return nullptr;
        }

        return static_cast<JSGlobal *>(JS_GetPrivate(JS::CurrentGlobalOrNull(cx)));
    }

    /*
        Override ClassMapper<T>::GetJSClass,
        since the global class has some special flags
    */
    static JSClass *GetJSClass();
protected:
    NIDIUM_DECL_JSCALL(load);
    NIDIUM_DECL_JSCALL(setTimeout);
    NIDIUM_DECL_JSCALL(setImmediate);
    NIDIUM_DECL_JSCALL(setInterval);
    NIDIUM_DECL_JSCALL(clearTimeout);
    NIDIUM_DECL_JSCALL(btoa);

    NIDIUM_DECL_JSGETTER(__filename);
    NIDIUM_DECL_JSGETTER(__dirname);
    NIDIUM_DECL_JSGETTER(global);
private:
    NidiumJS *m_JS;
};


} // namespace Binding
} // namespace Nidium

#endif
