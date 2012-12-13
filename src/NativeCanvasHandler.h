#ifndef nativecanvashandler_h__
#define nativecanvashandler_h__

#include <stdint.h>

class NativeSkia;

class NativeCanvasHandler
{
    private:
        NativeSkia *parent;
        NativeSkia *children;
        NativeSkia *next;
        NativeSkia *prev;
        NativeSkia *self;
        NativeSkia *last;

        double left, top;

        void addChild(NativeSkia *child);
        void removeFromParent();
    public:
        friend class NativeSkia;
        NativeCanvasHandler();
        ~NativeCanvasHandler();
};

#endif
