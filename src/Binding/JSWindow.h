/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef binding_jswindow_h__
#define binding_jswindow_h__

#include "Frontend/NML.h"
#include "Binding/ClassMapperWithEvents.h"
#include "Frontend/InputHandler.h"
#include "Binding/JSGlobal.h"

namespace Nidium {
namespace Graphics {
  class CanvasHandler;
}

namespace Binding {

class JSWindow : public ClassMapperWithEvents<JSWindow>
{
public:

    JSWindow(NidiumJS *njs)
        : m_RequestedFrame(NULL),
          m_Dragging(false){};

    virtual ~JSWindow();

    static JSFunctionSpec *ListMethods();
    static JSPropertySpec *ListProperties();

    static JSClass *GetJSClass();

    static inline JSWindow *GetInstance(JSObject *obj,
        JSContext *cx = nullptr)
    {
        return ClassMapper<JSWindow>::GetInstanceSingleton();
    }

    static inline JSWindow *GetInstanceUnsafe(JS::HandleObject obj,
        JSContext *cx = nullptr)
    {
        return GetInstance(obj, cx);
    }

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
    bool onMediaKey(Frontend::InputEvent::Type evType, bool isUpKey);

    bool dragBegin(int x, int y, const char *const *files, size_t nfiles);
    bool dragUpdate(int x, int y);
    bool dragDroped(int x, int y);
    void dragLeave();
    void dragEnd();

    void textInput(const char *data);
    void textEdit(const char *data);
    void keyupdown(int keycode, int mod, int state, int repeat, int location);

    void onKeyUpDown(int keyCode, int location, int mod, bool repeat, bool isUpKey);
    void onKeyPress(const char *c);
    void onCompositionStart();
    void onCompositionUpdate(const char *data);
    void onCompositionEnd();

    void addFrameCallback(JS::MutableHandleValue cb);
    void callFrameCallbacks(double ts, bool garbage = false);

    void initDataBase();

    static JSWindow *
    RegisterObject(JSContext *cx, int width, int height, JS::HandleObject doc);

    static JSWindow *GetObject(JSContext *cx);
    static JSWindow *GetObject(NidiumJS *njs);

protected:

    NIDIUM_DECL_JSCALL(requestAnimationFrame);
    NIDIUM_DECL_JSCALL(openFileDialog);
    NIDIUM_DECL_JSCALL(openDirDialog);
    NIDIUM_DECL_JSCALL(setSize);
    NIDIUM_DECL_JSCALL(center);
    NIDIUM_DECL_JSCALL(setPosition);
    NIDIUM_DECL_JSCALL(setFrame);
    NIDIUM_DECL_JSCALL(notify);
    NIDIUM_DECL_JSCALL(quit);
    NIDIUM_DECL_JSCALL(close);
    NIDIUM_DECL_JSCALL(open);
    NIDIUM_DECL_JSCALL(setSystemTray);
    NIDIUM_DECL_JSCALL(openURL);
    NIDIUM_DECL_JSCALL(exec);
    NIDIUM_DECL_JSCALL(alert);
    NIDIUM_DECL_JSCALL(bridge);

    NIDIUM_DECL_JSGETTER(devicePixelRatio);

    NIDIUM_DECL_JSGETTERSETTER(left);
    NIDIUM_DECL_JSGETTERSETTER(top);
    NIDIUM_DECL_JSGETTERSETTER(innerWidth);
    NIDIUM_DECL_JSGETTERSETTER(innerHeight);
    NIDIUM_DECL_JSGETTERSETTER(title);

private:
    bool dragEvent(const char *name, int x, int y);

    struct _requestedFrame
    {
        JS::PersistentRootedValue m_Cb;
        struct _requestedFrame *m_Next;
        _requestedFrame(JSContext *cx) : m_Cb(cx)
        {
            m_Next = NULL;
        }
    } * m_RequestedFrame;

    bool m_Dragging;
    JS::Heap<JSObject *> m_DraggedFiles;
};

} // namespace Binding
} // namespace Nidium

#endif
