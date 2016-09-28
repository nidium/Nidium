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

class JSVideo : public ClassMapperWithEvents<JSVideo>, public Core::Messages
{
public:
    JSVideo(Canvas2DContext *canvasCtx, JSContext *cx);

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
    static JSVideo *Constructor(JSContext *cx, JS::CallArgs &args,
        JS::HandleObject obj);
    static JSPropertySpec *ListProperties();
    static JSFunctionSpec *ListMethods();
    ~JSVideo();
protected:
    NIDIUM_DECL_JSCALL(play);
    NIDIUM_DECL_JSCALL(pause);
    NIDIUM_DECL_JSCALL(stop);
    NIDIUM_DECL_JSCALL(close);
    NIDIUM_DECL_JSCALL(open);
    NIDIUM_DECL_JSCALL(getAudioNode);
    NIDIUM_DECL_JSCALL(nextFrame);
    NIDIUM_DECL_JSCALL(prevFrame);
    NIDIUM_DECL_JSCALL(frameAt);
    NIDIUM_DECL_JSCALL(setSize);

    NIDIUM_DECL_JSGETTER(canvas);
    NIDIUM_DECL_JSGETTER(width);
    NIDIUM_DECL_JSGETTER(height);
    NIDIUM_DECL_JSGETTER(duration);
    NIDIUM_DECL_JSGETTER(bitrate);
    NIDIUM_DECL_JSGETTER(metadata);
    NIDIUM_DECL_JSGETTERSETTER(position);
private:
    void releaseAudioNode();
    bool m_IsDestructing;
    Canvas2DContext *m_CanvasCtx;
    //JSContext *m_Cx;
};

} // namespace Binding
} // namespace Nidium

#endif

