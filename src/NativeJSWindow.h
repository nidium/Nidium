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
        m_RequestedFrame(NULL), m_handler(NULL), m_Db(NULL)
    {
    };

    ~NativeJSwindow(){
        this->callFrameCallbacks(true);
    };

    void onReady();
    void assetReady(const NMLTag &tag);
    void windowFocus();
    void windowBlur();
    void resized(int width, int height);
    void mouseWheel(int xrel, int yrel, int x, int y);
    void mouseMove(int x, int y, int xrel, int yrel);
    void mouseClick(int x, int y, int state, int button);
    void textInput(const char *data);
    void keyupdown(int keycode, int mod, int state, int repeat);
    void addFrameCallback(jsval &cb);
    void callFrameCallbacks(double ts, bool garbage = false);

    void initDataBase();

    NativeCanvasHandler *getCanvasHandler() const {
        return m_handler;
    }

    static void registerObject(JSContext *cx, int width, int height);
    static const char *getJSObjectName() {
        return "window";
    }

    static JSClass *jsclass;

  private:

    void createMainCanvas(int width, int height);

    struct _requestedFrame {
        jsval cb;
        struct _requestedFrame *next;
    } *m_RequestedFrame;

    NativeCanvasHandler *m_handler;
    NativeDB *m_Db;
};

#endif
