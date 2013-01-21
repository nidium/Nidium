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
        * ::destroy() (JS_RemoveObjectRoot)
*/

struct NativeRect
{
    double fLeft, fTop, fBottom, fRight;
    bool isEmpty() const { return fLeft >= fRight || fTop >= fBottom; }
    bool intersect(double left, double top, double right, double bottom) {
        if (left < right && top < bottom && !this->isEmpty() &&
            fLeft < right && left < fRight && fTop < bottom && top < fBottom)
        {
            if (fLeft < left) fLeft = left;
            if (fTop < top) fTop = top;
            if (fRight > right) fRight = right;
            if (fBottom > bottom) fBottom = bottom;
            return true;
        }
        return false;
    }
};

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
        struct JSContext *jscx;

        int width, height;
        /*
            left and top are relative to parent
            a_left and a_top are relative to the root layer
        */
        double left, top, a_left, a_top;

        struct {
            double top;
            double bottom;
            double left;
            double right;
            int global;
        } padding;

        struct {
            int width;
            int height;
        } content;

        double opacity;
        bool overflow;
        
        NativeCanvasHandler(int width, int height);
        ~NativeCanvasHandler();

        void unrootHierarchy();
        void setWidth(int width);
        void setHeight(int height);
        void setSize(int width, int height);
        void setPadding(int padding);
        void setPositioning(NativeCanvasHandler::COORD_POSITION mode);
        void computeAbsolutePosition();
        void computeContentSize(int *cWidth, int *cHeight);

        void bringToFront();
        void sendToBack();
        void addChild(NativeCanvasHandler *insert,
            NativeCanvasHandler::Position position = POSITION_FRONT);

        int getContentWidth();
        int getContentHeight();
        void setHidden(bool val);
        bool isDisplayed() const;
        bool isHidden() const;
        void setOpacity(double val);
        void removeFromParent();
        NativeCanvasHandler *getParent();
        void getChildren(NativeCanvasHandler **out) const;
        int32_t countChildren() const;
        bool containsPoint(double x, double y) const;
        void layerize(NativeCanvasHandler *layer, double pleft,
            double ptop, double aopacity, NativeRect *clip);
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
