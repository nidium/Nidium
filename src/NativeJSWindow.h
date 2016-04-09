#ifndef nativejswindow_h__
#define nativejswindow_h__

#include <JS/NativeJSExposer.h>

#include "NativeTypes.h"

class NativeCanvasHandler;
class NativeDB;

class NativeJSwindow : public NativeJSExposer<NativeJSwindow>
{
  public:
    NativeJSwindow(JS::HandleObject jsobj, JSContext *cx) :
        NativeJSExposer<NativeJSwindow>(jsobj, cx),
        m_RequestedFrame(NULL), m_Handler(NULL), m_Db(NULL),
        m_Dragging(false)
    {
    };

    ~NativeJSwindow();

    void onReady(JS::HandleObject layout);
    bool onClose();
    void assetReady(const NMLTag &tag);
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

    NativeCanvasHandler *getCanvasHandler() const {
        return m_Handler;
    }

    NativeDB *getDataBase() const {
        return m_Db;
    }

    static NativeJSwindow *registerObject(JSContext *cx, int width,
        int height, JS::HandleObject doc);

    static const char *getJSObjectName() {
        return "Window";
    }

    static NativeJSwindow* getNativeClass(JSContext *cx);
    static NativeJSwindow* getNativeClass(NativeJS *njs);

    static JSClass *jsclass;

  private:

    bool dragEvent(const char *name, int x, int y);

    void createMainCanvas(int width, int height, JS::HandleObject docObj);
    void createStorage();

    struct _requestedFrame {
        JS::PersistentRootedValue cb;
        struct _requestedFrame *next;
        _requestedFrame(JSContext *cx): cb(cx) {
            next = NULL;
        }
    } *m_RequestedFrame;

    NativeCanvasHandler *m_Handler;
    NativeDB *m_Db;

    bool m_Dragging;
    JS::Heap<JSObject *> m_DraggedFiles;
};

#endif

