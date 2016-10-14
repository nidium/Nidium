/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/

#ifndef binding_jsaudio_h__
#define binding_jsaudio_h__

#include "AV/Audio.h"
#include "Binding/JSAudioContext.h"

using namespace Nidium::AV;

namespace Nidium {
namespace Binding {

class JSAudio : public ClassMapper<JSAudio>
{
public:
    static JSFunctionSpec *ListMethods();
    static void RegisterObject(JSContext *cx);
    static void RegisterAllObjects(JSContext *cx);

protected:
    NIDIUM_DECL_JSCALL(getContext);
};

} // namespace Binding
} // namespace Nidium


#endif
