/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef binding_jsimage_h__
#define binding_jsimage_h__

#include "Core/Messages.h"
#include "IO/Stream.h"
#include "Binding/ClassMapperWithEvents.h"

using Nidium::Core::Path;

namespace Nidium {
namespace Graphics {
    class Image;
}
namespace Binding {

class JSImage : public ClassMapperWithEvents<JSImage>, public Core::Messages
{
public:
    JSImage();
    virtual ~JSImage();
    static JSPropertySpec *ListProperties();
    static JSImage *Constructor(JSContext *cx, JS::CallArgs &args,
        JS::HandleObject obj);

    static Graphics::Image *JSObjectToImage(JS::HandleObject obj);
    static void RegisterObject(JSContext *cx);
    static bool JSObjectIs(JSContext *cx, JS::HandleObject obj);
    static JSObject *BuildImageObject(JSContext *cx,
                                      Graphics::Image *image,
                                      const char name[] = NULL);

    void onMessage(const Core::SharedMessages::Message &msg);

    Graphics::Image *getImage() const {
        return m_Image;
    }

protected:
    NIDIUM_DECL_JSGETTERSETTER(src);
    NIDIUM_DECL_JSGETTER(width);
    NIDIUM_DECL_JSGETTER(height);
private:
    bool setupWithBuffer(buffer *buf);

    Graphics::Image *m_Image;
    IO::Stream *m_Stream;
    Path *m_Path;
};

} // namespace Binding
} // namespace Nidium

#endif
