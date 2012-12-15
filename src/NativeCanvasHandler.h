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
        friend class NativeSkia;

        enum Position {
            POSITION_FRONT,
            POSITION_BACK
        };
        enum COORD_POSITION {
            COORD_RELATIVE,
            COORD_ABSOLUTE
        };

        enum Visibility {
            CANVAS_VISIBILITY_VISIBLE,
            CANVAS_VISIBILITY_HIDDEN
        };

        NativeCanvas2DContext *context;

        int width, height;
        /*
            left and top are relative to parent
            a_left and a_top are relative to the root layer
        */
        double left, top, a_left, a_top;
        double opacity;
        
        NativeCanvasHandler(int width, int height);
        ~NativeCanvasHandler();

        void setPosition(double left, double top);
        void setPositioning(NativeCanvasHandler::COORD_POSITION mode);

        void addChild(NativeCanvasHandler *insert,
            NativeCanvasHandler::Position position = POSITION_FRONT);

        void setHidden(bool val);
        bool isDisplayed();
        bool isHidden();
        void setOpacity(double val);
        void removeFromParent();
        void layerize(NativeCanvasHandler *layer, double pleft, double ptop);
    private:
        NativeCanvasHandler *parent;
        NativeCanvasHandler *children;
        NativeCanvasHandler *next;
        NativeCanvasHandler *prev;
        NativeCanvasHandler *last;

        COORD_POSITION coordPosition;
        Visibility visibility;

};

#endif
