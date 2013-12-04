#ifndef nativecanvashandler_h__
#define nativecanvashandler_h__

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

class NativeSkia;
class NativeCanvasContext;

/*
    - Handle a canvas layer.
    - Agnostic to any renderer.
    - All size are in logical pixels (device ratio is handled by NativeCanvasContext)
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

    bool checkIntersect(double left, double top, double right, double bottom) const {
        if (left < right && top < bottom && !this->isEmpty() &&
            fLeft < right && left < fRight && fTop < bottom && top < fBottom)
        {
            return true;
        }
        return false;
    }

    NativeRect scaled(float scale) const {
        NativeRect r = {
            this->fLeft*scale,
            this->fTop*scale,
            this->fBottom*scale,
            this->fRight*scale
        };

        return r;
    }
    bool contains(double x, double y) const {
        return !this->isEmpty() &&
               fLeft <= x && x < fRight &&
               fTop <= y && y < fBottom;
    }
};

class NativeCanvasHandler
{
    public:
        friend class NativeSkia;

        enum COORD_MODE {
            kLeft_Coord   = 1 << 0,
            kRight_Coord  = 1 << 1,
            kTop_Coord    = 1 << 2,
            kBottom_Coord = 1 << 3
        };

        enum Position {
            POSITION_FRONT,
            POSITION_BACK
        };
        enum COORD_POSITION {
            COORD_RELATIVE,
            COORD_ABSOLUTE,
            COORD_FIXED
        };

        enum Visibility {
            CANVAS_VISIBILITY_VISIBLE,
            CANVAS_VISIBILITY_HIDDEN
        };

        NativeCanvasContext *m_Context;
        class JSObject *jsobj;
        struct JSContext *jscx;

        int width, height;
        /*
            left and top are relative to parent
            a_left and a_top are relative to the root layer
        */
        double left, top, a_left, a_top, right, bottom;

        struct {
            double top;
            double bottom;
            double left;
            double right;
            int global;
        } padding;

        struct {
            double x;
            double y;
        } translate_s;

        struct {
            int width;
            int height;
            int scrollTop;
            int scrollLeft;
        } content;

        struct {
            int x, y, xrel, yrel;
            bool consumed;
        } mousePosition;

        bool overflow;

        NativeCanvasContext *getContext() const {
            return this->m_Context;
        }

        double getOpacity() const {
            return this->opacity;
        }

        double getZoom() const {
            return this->zoom;
        }

        double getLeft(bool absolute = false) const {
            if (absolute) return a_left;

            if (!(coordMode & kLeft_Coord)) {
                return this->parent->getWidth() - (this->width + this->right);
            }

            return this->left;
        }
        double getTop(bool absolute = false) const {
            if (absolute) return a_top;

            if (!(coordMode & kTop_Coord)) {
                return this->parent->getHeight() - (this->height + this->bottom);
            }

            return this->top;
        }

        double getTopScrolled() const {
            double top = getTop();
            if (coordPosition == COORD_RELATIVE && parent != NULL) {
                top -= parent->content.scrollTop;
            }

            return top;
        }

        double getLeftScrolled() const {
            double left = getLeft();
            if (coordPosition == COORD_RELATIVE && parent != NULL) {
                left -= parent->content.scrollLeft;
            }
            return left;
        }

        double getRight() const {
            return this->right;
        }
        double getBottom() const {
            return this->bottom;
        }

        /*
            Get the width in logical pixels
        */
        double getWidth() const {
            if (hasFixedWidth()) {
                return this->width;
            }
            if (parent == NULL) return 0.;

            double pwidth = parent->getWidth();

            if (pwidth == 0) return 0.;

            return pwidth - this->getLeft() - this->getRight();
        }

        /*
            Get the height in logical pixels
        */
        double getHeight() const {
            if (hasFixedHeight()) {
                return this->height;
            }
            if (parent == NULL) return 0.;

            double pheight = parent->getHeight();

            if (pheight == 0) return 0.;

            return pheight - this->getTop() - this->getBottom();
        }

        bool hasFixedWidth() const {
            return !((coordMode & (kLeft_Coord | kRight_Coord))
                    == (kLeft_Coord|kRight_Coord));
        }

        bool hasFixedHeight() const {
            return !((coordMode & (kTop_Coord | kBottom_Coord))
                    == (kTop_Coord|kBottom_Coord));
        }        

        void unsetLeft() {
            coordMode &= ~kLeft_Coord;
        }

        void unsetRight() {
            coordMode &= ~kRight_Coord;
        }

        void unsetTop() {
            coordMode &= ~kTop_Coord;
        }

        void unsetBottom() {
            coordMode &= ~kBottom_Coord;
        }

        void setLeft(double val) {
            coordMode |= kLeft_Coord;
            this->left = val;
            if (!hasFixedWidth()) {
                setSize(this->getWidth(), this->height);
            }
        }
        void setRight(double val) {
            coordMode |= kRight_Coord;
            this->right = val; 
            if (!hasFixedWidth()) {
                setSize(this->getWidth(), this->height);
            }
        }

        void setTop(double val) {
            coordMode |= kTop_Coord;
            this->top = val;
            if (!hasFixedHeight()) {
                setSize(this->width, this->getHeight());
            }            
        }

        void setBottom(double val) {
            coordMode |= kBottom_Coord;
            this->bottom = val;
            if (!hasFixedHeight()) {
                setSize(this->width, this->getHeight());
            }            
        }

        void setScale(double x, double y);

        double getScaleX() const {
            return this->scaleX;
        }

        double getScaleY() const {
            return this->scaleY;
        }

        void setAllowNegativeScroll(bool val) {
            m_AllowNegativeScroll = val;
        }

        bool getAllowNegativeScroll() const {
            return m_AllowNegativeScroll;
        }
        
        NativeCanvasHandler(int width, int height);
        ~NativeCanvasHandler();

        void unrootHierarchy();

        void setContext(NativeCanvasContext *context);

        bool setWidth(int width);
        bool setHeight(int height);
        void updateChildrenSize(bool width, bool height);
        void setSize(int width, int height);
        void setPadding(int padding);
        void setPositioning(NativeCanvasHandler::COORD_POSITION mode);
        void setScrollTop(int value);
        void setScrollLeft(int value);
        void computeAbsolutePosition();
        void computeContentSize(int *cWidth, int *cHeight);
        void translate(double x, double y);
        bool isOutOfBound();
        NativeRect getViewport();
        NativeRect getVisibleRect();

        void bringToFront();
        void sendToBack();
        void addChild(NativeCanvasHandler *insert,
            NativeCanvasHandler::Position position = POSITION_FRONT);

        int getContentWidth();
        int getContentHeight();
        void setHidden(bool val);
        bool isDisplayed() const;
        bool isHidden() const;
        bool hasAFixedAncestor() const;
        void setOpacity(double val);
        void setZoom(double val);
        void removeFromParent();
        void getChildren(NativeCanvasHandler **out) const;
        NativeCanvasHandler *getParent() const { return this->parent; }
        NativeCanvasHandler *getFirstChild() const { return this->children; }
        NativeCanvasHandler *getLastChild() const { return this->last; }
        NativeCanvasHandler *getNextSibling() const { return this->next; }
        NativeCanvasHandler *getPrevSibling() const { return this->prev; }
        int32_t countChildren() const;
        bool containsPoint(double x, double y) const;
        void layerize(NativeCanvasHandler *layer, double pleft,
            double ptop, double aopacity, double zoom, NativeRect *clip);
        NativeCanvasHandler *parent;
        NativeCanvasHandler *children;
        NativeCanvasHandler *next;
        NativeCanvasHandler *prev;
        NativeCanvasHandler *last;
    private:
        int32_t nchildren;
        void dispatchMouseEvents(NativeCanvasHandler *layer);
        COORD_POSITION coordPosition;
        Visibility visibility;
        unsigned coordMode : 16;
        double opacity;
        double zoom;

        double scaleX, scaleY;
        bool m_AllowNegativeScroll;

        void recursiveScale(double x, double y, double oldX, double oldY);
};

#endif
