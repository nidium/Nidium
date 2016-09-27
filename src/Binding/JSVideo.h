/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/

#ifndef binding_jsvideo_h__
#define binding_jsvideo_h__

#include "AV/Video.h"
#include "Binding/ClassMapper.h"
#include "Binding/JSAV.h"

namespace Nidium {
namespace Binding {

class JSVideo : public JSExposer<JSVideo>, public Core::Messages
{
public:
    JSVideo(JS::HandleObject obj, Canvas2DContext *canvasCtx, JSContext *cx);

    AV::Video *m_Video;

    JS::Heap<JSObject *> m_AudioNode;
    void *m_ArrayContent;

    int m_Width;
    int m_Height;
    int m_Left;
    int m_Top;

    void stopAudio();
    void setSize(int width, int height);
    void close();

    static void RegisterObject(JSContext *cx);
    static void FrameCallback(uint8_t *data, void *custom);
    void onMessage(const Core::SharedMessages::Message &msg);
    static void onEvent(const struct AV::AVSourceEvent *cev);

    ~JSVideo();

private:
    void releaseAudioNode();
    bool m_IsDestructing;
    Canvas2DContext *m_CanvasCtx;
    JSContext *m_Cx;
};

} // namespace Binding
} // namespace Nidium

#endif

