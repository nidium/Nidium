/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef binding_jswindow_h__
#define binding_jswindow_h__

#include <Frontend/NML.h>
#include <Binding/JSExposer.h>

namespace Nidium {
namespace Graphics {
    class CanvasHandler;
}
namespace Binding {

class JSWindow : public JSExposer<JSWindow>
{
  public:
    JSWindow(JS::HandleObject jsobj, JSContext *cx) :
        JSExposer<JSWindow>(jsobj, cx),
        m_RequestedFrame(NULL), m_Handler(NULL),
        m_Dragging(false)
    {
    };

    ~JSWindow();

    void onReady(JS::HandleObject layout);
    bool onClose();
    void assetReady(const Frontend::NMLTag &tag);
    void windowFocus();
    void windowBlur();
    void resized(int width, int height);
    void mouseWheel(int xrel, int yrel, int x, int y);
    void mouseMove(int x, int y, int xrel, int yrel);
    void mouseClick(int x, int y, int state, int button, int clicks);
    void systemMenuClicked(const char *id);

    bool dragBegin(int x, int y, const char * const *files, size_t nfiles);
    bool dragUpdate(int x, int y);
    bool dragDroped(int x, int y);
    void dragLeave();
    void dragEnd();

    void textInput(const char *data);
    void keyupdown(int keycode, int mod, int state, int repeat, int location);
    void addFrameCallback(JS::MutableHandleValue cb);
    void callFrameCallbacks(double ts, bool garbage = false);

    void initDataBase();

    Graphics::CanvasHandler *getCanvasHandler() const {
        return m_Handler;
    }

    static JSWindow *RegisterObject(JSContext *cx, int width,
        int height, JS::HandleObject doc);

    static const char *GetJSObjectName() {
        return "Window";
    }

    static JSWindow* GetObject(JSContext *cx);
    static JSWindow* GetObject(NidiumJS *njs);

    static JSClass *jsclass;

  private:

    bool dragEvent(const char *name, int x, int y);

    void createMainCanvas(int width, int height, JS::HandleObject docObj);

    struct _requestedFrame {
        JS::PersistentRootedValue m_Cb;
        struct _requestedFrame *m_Next;
        _requestedFrame(JSContext *cx): m_Cb(cx) {
            m_Next = NULL;
        }
    } *m_RequestedFrame;

    Graphics::CanvasHandler *m_Handler;

    bool m_Dragging;
    JS::Heap<JSObject *> m_DraggedFiles;
};

} // namespace Binding
} // namespace Nidium

#endif

