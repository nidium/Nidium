#ifndef nativecanvashandler_h__
#define nativecanvashandler_h__

#include <stdint.h>

class NativeSkia;

struct handler_children
{
    NativeSkia *child;
    struct handler_children *next;
    struct handler_children *prev;
};

class NativeCanvasHandler
{
    private:
        NativeSkia *parent;
        NativeSkia *children;
        NativeSkia *next;
        NativeSkia *prev;
        NativeSkia *self;

        double left, top;
        
        void addChild(NativeSkia *child);
        void removeFromParent();
    public:
        friend class NativeSkia;
        NativeCanvasHandler();
};

#endif
