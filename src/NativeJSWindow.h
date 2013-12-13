#ifndef nativejswindow_h__
#define nativejswindow_h__

#include "NativeJSExposer.h"
#include "NativeTypes.h"

class NativeCanvasHandler;
class NativeDB;

class NativeJSwindow : public NativeJSExposer<NativeJSwindow>
{
  public:
    NativeJSwindow() : 
        m_RequestedFrame(NULL), m_handler(NULL), m_Db(NULL),
        m_Dragging(false), m_DragedFiles(NULL)
    {
    };

    ~NativeJSwindow();

    void onReady(JSObject *layout);
    void assetReady(const NMLTag &tag);
    void windowFocus();
    void windowBlur();
    void resized(int width, int height);
    void mouseWheel(int xrel, int yrel, int x, int y);
    void mouseMove(int x, int y, int xrel, int yrel);
    void mouseClick(int x, int y, int state, int button);

    bool dragBegin(int x, int y, const char * const *files, size_t nfiles);
    bool dragUpdate(int x, int y);
    bool dragDroped(int x, int y);
    void dragLeave();
    void dragEnd();

    void textInput(const char *data);
    void keyupdown(int keycode, int mod, int state, int repeat, int location);
    void addFrameCallback(jsval &cb);
    void callFrameCallbacks(double ts, bool garbage = false);

    void initDataBase();

    NativeCanvasHandler *getCanvasHandler() const {
        return m_handler;
    }

    NativeDB *getDataBase() const {
        return m_Db;
    }

    static void registerObject(JSContext *cx, int width, int height);
    static const char *getJSObjectName() {
        return "window";
    }

    static JSClass *jsclass;

  private:

    bool dragEvent(const char *name, int x, int y);

    void createMainCanvas(int width, int height);
    void createStorage();

    struct _requestedFrame {
        jsval cb;
        struct _requestedFrame *next;
    } *m_RequestedFrame;

    NativeCanvasHandler *m_handler;
    NativeDB *m_Db;

    bool m_Dragging;
    JSObject *m_DragedFiles;
};

#endif
