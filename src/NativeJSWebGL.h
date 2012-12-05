#ifndef nativejswebgl_h__
#define nativejswebgl_h__

#include "NativeJSExposer.h"

#define NEW_CLASS(name)\
class NativeJS ## name: public NativeJSExposer\
{\
    public :\
        NativeJS ## name ();\
        ~NativeJS ## name ();\
        JSObject *jsobj;\
        static void registerObject(JSContext *cx);\
};

class NativeJSNativeGL: public NativeJSExposer
{
    public :
        NativeJSNativeGL() 
            : jsobj(NULL), unpackFlipY(false), unpackPremultiplyAlpha(false) {};
        ~NativeJSNativeGL();

        JSObject *jsobj;
        bool unpackFlipY;
        bool unpackPremultiplyAlpha;

        static void registerObject(JSContext *cx);
};

NEW_CLASS(WebGLRenderingContext)
NEW_CLASS(WebGLObject)
NEW_CLASS(WebGLBuffer)
NEW_CLASS(WebGLFrameBuffer)
NEW_CLASS(WebGLProgram)
NEW_CLASS(WebGLRenderbuffer)
NEW_CLASS(WebGLShader)
NEW_CLASS(WebGLTexture)
NEW_CLASS(WebGLUniformLocation)

#endif
