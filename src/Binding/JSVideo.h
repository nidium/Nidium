/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef binding_jsvideo_h__
#define binding_jsvideo_h__

#include "AV/Video.h"
#include "Binding/ClassMapperWithEvents.h"
#include "Binding/JSAudioNode.h"

#include <SkBitmap.h>
#include <SkPaint.h>

namespace Nidium {
namespace Binding {

class JSAudioNodeSourceVideo;

// {{{ JSVideo
class JSVideo : public JSAVSourceBase,
                public JSAVSourceEventInterface,
                public ClassMapperWithEvents<JSVideo>
{
public:
    JSVideo(Binding::Canvas2DContext *canvasCtx, JSContext *cx);

    AV::Video *m_Video                  = nullptr;
    JSAudioNodeSourceVideo *m_AudioNode = nullptr;

    int m_Width  = -1;
    int m_Height = -1;
    int m_Left   = 0;
    int m_Top    = 0;
    SkBitmap m_Bitmap;
    SkPaint m_Paint;

    void releaseAudioNode();
    void setSize(int width, int height);

    static void RegisterObject(JSContext *cx);
    static void RegisterAllObjects(JSContext *cx);
    static void FrameCallback(uint8_t *data, void *custom);
    void frameCallback(uint8_t *data);
    void onSourceMessage(const Core::SharedMessages::Message &msg) override;
    static JSVideo *
    Constructor(JSContext *cx, JS::CallArgs &args, JS::HandleObject obj);
    static JSPropertySpec *ListProperties();
    static JSFunctionSpec *ListMethods();
    virtual ~JSVideo();

protected:
    NIDIUM_DECL_JSCALL(getAudioNode);
    NIDIUM_DECL_JSCALL(nextFrame);
    NIDIUM_DECL_JSCALL(prevFrame);
    NIDIUM_DECL_JSCALL(frameAt);
    NIDIUM_DECL_JSCALL(setSize);

    JSAV_PASSTHROUGH_CALL(JSAVSourceBase, open)
    JSAV_PASSTHROUGH_CALL(JSAVSourceBase, play)
    JSAV_PASSTHROUGH_CALL(JSAVSourceBase, pause)
    JSAV_PASSTHROUGH_CALL(JSAVSourceBase, stop)
    NIDIUM_DECL_JSCALL(close);

    JSAV_PASSTHROUGH_G(JSAVSourceBase, position)
    JSAV_PASSTHROUGH_S(JSAVSourceBase, position)
    JSAV_PASSTHROUGH_G(JSAVSourceBase, duration)
    JSAV_PASSTHROUGH_G(JSAVSourceBase, metadata)
    JSAV_PASSTHROUGH_G(JSAVSourceBase, bitrate)
    NIDIUM_DECL_JSGETTER(canvas);
    NIDIUM_DECL_JSGETTER(width);
    NIDIUM_DECL_JSGETTER(height);

    AV::AVSource *getSource() override
    {
        return m_Video;
    }

    bool isReleased() override
    {
        return m_IsReleased;
    }

    JSContext *getJSContext() override
    {
        return ClassMapperWithEvents<JSVideo>::getJSContext();
    }

    bool fireJSEvent(const char *name, JS::MutableHandleValue val) override
    {
        return ClassMapperWithEvents<JSVideo>::fireJSEvent(name, val);
    }

private:
    Canvas2DContext *m_CanvasCtx;
    bool m_IsReleased = false;
};
// }}}

// {{{ JSAudioNodeSourceVideo
class JSAudioNodeSourceVideo
    : public ClassMapperWithEvents<JSAudioNodeSourceVideo>,
      public JSAudioNode
{
public:
    static void RegisterObject(JSContext *cx)
    {
        JSAudioNodeSourceVideo::ExposeClass<0>(cx, "AudioNodeSourceVideo");
    }

    static JSPropertySpec *ListProperties()
    {
        return nullptr;
    };
    static JSFunctionSpec *ListMethods()
    {
        return nullptr;
    };

    JSAudioNodeSourceVideo(JSContext *cx,
                           JSVideo *video,
                           AV::VideoAudioSource *source)
        : m_Video(video)
    {
        m_Node = static_cast<AudioNode *>(source);
        this->createObject<JSAudioNodeSourceVideo>(cx);
    }

    virtual ~JSAudioNodeSourceVideo()
    {
        // First the video needs to release the audio node
        // (as we can't delete it while it's used by the video)
        m_Video->m_Video->releaseAudioNode(false /* do not delete the node */);

        // Then call releaseNode() which will take care of
        // deleting the node and other stuff
        this->releaseNode();

        m_Video->m_AudioNode = nullptr;
    }

private:
    JSVideo *m_Video;
};
// }}}

} // namespace Binding
} // namespace Nidium

#endif
