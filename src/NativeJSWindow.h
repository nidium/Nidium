#ifndef nativejswindow_h__
#define nativejswindow_h__

#include "NativeJSExposer.h"
#include "NativeTypes.h"

class NativeJSwindow : public NativeJSExposer<NativeJSwindow>
{
  public:
    NativeJSwindow(){};
    ~NativeJSwindow(){};

    void onReady();
    void assetReady(const NMLTag &tag);
    void windowFocus();
    void windowBlur();
    void mouseWheel(int xrel, int yrel, int x, int y);
    void mouseMove(int x, int y, int xrel, int yrel);
    void mouseClick(int x, int y, int state, int button);
    void textInput(const char *data);
    void keyupdown(int keycode, int mod, int state, int repeat);

    static void registerObject(JSContext *cx);
    static const char *getJSObjectName() {
        return "window";
    }

    static JSClass *jsclass;
};

#endif
