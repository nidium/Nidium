#ifndef nativejsnative_h__
#define nativejsnative_h__

#include "NativeJSExposer.h"

class NativeSkia;
class NativeCanvasHandler;

class NativeJSNative : public NativeJSExposer<NativeJSNative>
{
  public:
     NativeJSNative(){};
    ~NativeJSNative(){};

    static bool showFPS;
    static void registerObject(JSContext *cx, int width, int height);

    static const char *getJSObjectName() {
        return "Native";
    }

    static JSClass *jsclass;

    NativeCanvasHandler *getCanvasHandler() const {
    	return this->handler;
    }

  private:
  	NativeCanvasHandler *handler;
};

#endif
