#ifndef nativecanvashandler_h__
#define nativecanvashandler_h__

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include <NativeEvents.h>

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

struct NativeLayerSiblingContext {
    double maxLineHeight;
    double maxLineHeightPreviousLine;

    NativeLayerSiblingContext() :
        maxLineHeight(0.), maxLineHeightPreviousLine(0.){}
};

struct NativeLayerizeContext {
    class NativeCanvasHandler *layer;
    double pleft;
    double ptop;
    double aopacity;
    double azoom;
    NativeRect *clip;

    struct NativeLayerSiblingContext *siblingCtx;

    void reset() {
        layer = NULL;
        pleft = 0.;
        ptop = 0.;
        aopacity = 1.0;
        azoom = 1.0;
        clip = NULL;
        siblingCtx = NULL;
    }
};

class NativeCanvasHandler : public NativeEvents
{
    public:
        friend class NativeSkia;

        static const uint8_t EventID = 1;

        enum COORD_MODE {
            kLeft_Coord   = 1 << 0,
            kRight_Coord  = 1 << 1,
            kTop_Coord    = 1 << 2,
            kBottom_Coord = 1 << 3
        };

        enum Events {
            RESIZE_EVENT = 1,
        };

        enum Position {
            POSITION_FRONT,
            POSITION_BACK
        };

        enum COORD_POSITION {
            COORD_RELATIVE,
            COORD_ABSOLUTE,
            COORD_FIXED,
            COORD_INLINE
        };

        enum FLOW_MODE {
            kFlowDoesntInteract = 0,
            kFlowInlinePreviousSibling = 1 << 0
        };

        enum Visibility {
            CANVAS_VISIBILITY_VISIBLE,
            CANVAS_VISIBILITY_HIDDEN
        };

        NativeCanvasContext *m_Context;
        class JSObject *jsobj;
        struct JSContext *jscx;

        int m_Width, m_Height, m_MinWidth, m_MinHeight, m_MaxWidth, m_MaxHeight;
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

        bool m_Overflow;

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
                return m_Parent->getWidth() - (m_Width + this->right);
            }

            return this->left;
        }
        double getTop(bool absolute = false) const {
            if (absolute) return a_top;

            if (!(coordMode & kTop_Coord)) {
                return m_Parent->getHeight() - (m_Height + this->bottom);
            }

            return this->top;
        }

        double getTopScrolled() const {
            double top = getTop();
            if (coordPosition == COORD_RELATIVE && m_Parent != NULL) {
                top -= m_Parent->content.scrollTop;
            }

            return top;
        }

        double getLeftScrolled() const {
            double left = getLeft();
            if (coordPosition == COORD_RELATIVE && m_Parent != NULL) {
                left -= m_Parent->content.scrollLeft;
            }
            return left;
        }

        double getRight() const {
            if (hasStaticRight() || !m_Parent) {
                return this->right;
            }
            
            return m_Parent->getWidth() - (getLeftScrolled() + getWidth());
        }

        double getBottom() const {
            if (hasStaticBottom() || !m_Parent) {
                return this->bottom;
            }
            
            return m_Parent->getHeight() - (getTopScrolled() + getHeight());
        }

        /*
            Get the width in logical pixels
        */
        double getWidth() const {
            if (hasFixedWidth()) {
                return m_Width;
            }
            if (m_Parent == NULL) return 0.;

            double pwidth = m_Parent->getWidth();

            if (pwidth == 0) return 0.;

            return pwidth - this->getLeft() - this->getRight();
        }

        /*
            Get the height in logical pixels
        */
        double getHeight() const {
            if (hasFixedHeight()) {
                return m_Height;
            }
            if (m_Parent == NULL) return 0.;

            double pheight = m_Parent->getHeight();

            if (pheight == 0) return 0.;

            return pheight - this->getTop() - this->getBottom();
        }

        int getMinWidth() const {
            return m_MinWidth;
        }

        int getMaxWidth() const {
            return m_MaxWidth;
        }

        int getMinHeight() const {
            return m_MinHeight;
        }

        int getMaxHeight() const {
            return m_MaxHeight;
        }

        bool hasFixedWidth() const {
            return !((coordMode & (kLeft_Coord | kRight_Coord))
                    == (kLeft_Coord|kRight_Coord));
        }

        bool hasFixedHeight() const {
            return !((coordMode & (kTop_Coord | kBottom_Coord))
                    == (kTop_Coord|kBottom_Coord));
        }

        bool hasStaticLeft() const {
            return coordMode & kLeft_Coord;
        }

        bool hasStaticRight() const {
            return coordMode & kRight_Coord;
        }

        bool hasStaticTop() const {
            return coordMode & kTop_Coord;
        }

        bool hasStaticBottom() const {
            return coordMode & kBottom_Coord;
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
            if (m_FlowMode & kFlowInlinePreviousSibling) {
                return;
            }
            coordMode |= kLeft_Coord;
            this->left = val;
            if (!hasFixedWidth()) {
                setSize(this->getWidth(), m_Height);
            }
        }
        void setRight(double val) {
            coordMode |= kRight_Coord;
            this->right = val; 
            if (!hasFixedWidth()) {
                setSize(this->getWidth(), m_Height);
            }
        }

        void setTop(double val) {
            if (m_FlowMode & kFlowInlinePreviousSibling) {
                return;
            }
            coordMode |= kTop_Coord;
            this->top = val;
            if (!hasFixedHeight()) {
                setSize(m_Width, this->getHeight());
            }
        }

        void setBottom(double val) {
            coordMode |= kBottom_Coord;
            this->bottom = val;
            if (!hasFixedHeight()) {
                setSize(m_Width, this->getHeight());
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

        COORD_POSITION getPositioning() const {
            return coordPosition;
        }

        unsigned int getFlowMode() const {
            return m_FlowMode;
        }
        
        NativeCanvasHandler(int width, int height);
        virtual ~NativeCanvasHandler();

        void unrootHierarchy();

        void setContext(NativeCanvasContext *context);

        bool setWidth(int width);
        bool setHeight(int height);

        bool setMinWidth(int width);
        bool setMinHeight(int height);

        bool setMaxWidth(int width);
        bool setMaxHeight(int height);

        void updateChildrenSize(bool width, bool height);
        void setSize(int width, int height, bool redraw = true);
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

        NativeCanvasHandler *getParent() const { return m_Parent; }
        NativeCanvasHandler *getFirstChild() const { return m_Children; }
        NativeCanvasHandler *getLastChild() const { return m_Last; }
        NativeCanvasHandler *getNextSibling() const { return m_Next; }
        NativeCanvasHandler *getPrevSibling() const { return m_Prev; }
        int32_t countChildren() const;
        bool containsPoint(double x, double y) const;
        void layerize(NativeLayerizeContext &layerContext);

        NativeCanvasHandler *m_Parent;
        NativeCanvasHandler *m_Children;

        NativeCanvasHandler *m_Next;
        NativeCanvasHandler *m_Prev;
        NativeCanvasHandler *m_Last;
    protected:
        NativeCanvasHandler *getPrevInlineSibling() const {
            NativeCanvasHandler *prev;
            for (prev = m_Prev; prev != NULL; prev = prev->m_Prev) {
                if (prev->m_FlowMode & kFlowInlinePreviousSibling) {
                    return prev;
                }
            }

            return NULL;
        }
    private:
        int32_t nchildren;
        void dispatchMouseEvents(NativeCanvasHandler *layer);
        COORD_POSITION coordPosition;
        Visibility visibility;
        unsigned m_FlowMode;
        unsigned coordMode : 16;
        double opacity;
        double zoom;

        double scaleX, scaleY;
        bool m_AllowNegativeScroll;
        bool m_ExpandWidth, m_ExpandHeight;

        void recursiveScale(double x, double y, double oldX, double oldY);
};

#endif
