#ifndef nativecanvashandler_h__
#define nativecanvashandler_h__

#include <stdint.h>

class NativeSkia;
class NativeCanvas2DContext;

/*
    Handle a canvas layer.
    Agnostic to any renderer.
    A NativeCanvasContext must be attached to it

    TODO: NativeCanvasContext interface instead of NativeCanvas2DContext;
*/

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

        NativeCanvas2DContext *context;
        int width, height;

        friend class NativeSkia;
        NativeCanvasHandler(int width, int height);
        ~NativeCanvasHandler();

        void setPosition(double left, double top);
        void setPositioning(NativeCanvasHandler::COORD_POSITION mode);

        void addChild(NativeCanvasHandler *insert,
            NativeCanvasHandler::Position position = POSITION_FRONT);
        void removeFromParent();
        void layerize(NativeCanvasHandler *layer, double pleft, double ptop);
    private:
        NativeCanvasHandler *parent;
        NativeCanvasHandler *children;
        NativeCanvasHandler *next;
        NativeCanvasHandler *prev;
        NativeCanvasHandler *last;

        double left, top;


        COORD_POSITION coordPosition;

};

#endif
