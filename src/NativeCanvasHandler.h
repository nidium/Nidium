#ifndef nativecanvashandler_h__
#define nativecanvashandler_h__

#include <stdint.h>

class NativeSkia;

class NativeCanvasHandler
{
    public:
        
        enum Position {
            POSITION_FRONT,
            POSITION_BACK
        };
        enum COORD_POSITION {
            COORD_RELATIVE,
            COORD_ABSOLUTE
        };

        friend class NativeSkia;
        NativeCanvasHandler();
        ~NativeCanvasHandler();    
    private:
        NativeSkia *parent;
        NativeSkia *children;
        NativeSkia *next;
        NativeSkia *prev;
        NativeSkia *self;
        NativeSkia *last;

        double left, top;

        void addChild(NativeSkia *child,
            NativeCanvasHandler::Position position = POSITION_FRONT);
        void removeFromParent();

        COORD_POSITION coordPosition;

};

#endif
