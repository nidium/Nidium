#ifndef nativecanvashandler_h__
#define nativecanvashandler_h__

#include <stdint.h>

class NativeSkia;
class NativeCanvas2DContext;

/*
    Handle a canvas layer.
    Agnostic to any renderer.
    A NativeCanvasContext must be attached to it

    TODO:
        * NativeCanvasContext interface instead of NativeCanvas2DContext;
        * this->jsobj (JS_AddObjectRoot)
        * ::destroy() (JS_RemoveObjectRoot)
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
        struct JSObject *jsobj;

        int width, height;
        /*
            left and top are relative to parent
            a_left and a_top are relative to the root layer
        */
        double left, top, a_left, a_top;

        double opacity;
        
        NativeCanvasHandler(int width, int height);
        ~NativeCanvasHandler();

        void setWidth(int width);
        void setHeight(int height);
        void setPosition(double left, double top);
        void setPositioning(NativeCanvasHandler::COORD_POSITION mode);
        void computeAbsolutePosition();

        void bringToFront();
        void sendToBack();
        void addChild(NativeCanvasHandler *insert,
            NativeCanvasHandler::Position position = POSITION_FRONT);

        void setHidden(bool val);
        bool isDisplayed();
        bool isHidden();
        void setOpacity(double val);
        void removeFromParent();
        NativeCanvasHandler *getParent();
        void getChildren(NativeCanvasHandler **out);
        int32_t countChildren();
        void layerize(NativeCanvasHandler *layer, double pleft, double ptop);
    private:
        NativeCanvasHandler *parent;
        NativeCanvasHandler *children;
        NativeCanvasHandler *next;
        NativeCanvasHandler *prev;
        NativeCanvasHandler *last;
        int32_t nchildren;

        COORD_POSITION coordPosition;
        Visibility visibility;

};

#endif
