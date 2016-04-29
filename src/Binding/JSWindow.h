#ifndef binding_jswindow_h__
#define binding_jswindow_h__

#include <Frontend/NML.h>
#include <Binding/JSExposer.h>
#include <Binding/JSDB.h>

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
        m_RequestedFrame(NULL), m_Handler(NULL), m_Db(NULL),
        m_Dragging(false)
    {
    };

    ~JSWindow();

    void onReady(JS::HandleObject layout);
    bool onClose();
    void assetReady(const Nidium::Frontend::NMLTag &tag);
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

    JSDB *getDataBase() const {
        return m_Db;
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
    void createStorage();

    struct _requestedFrame {
        JS::PersistentRootedValue cb;
        struct _requestedFrame *next;
        _requestedFrame(JSContext *cx): cb(cx) {
            next = NULL;
        }
    } *m_RequestedFrame;

    Graphics::CanvasHandler *m_Handler;
    JSDB *m_Db;

    bool m_Dragging;
    JS::Heap<JSObject *> m_DraggedFiles;
};

} // namespace Binding
} // namespace Nidium

#endif

