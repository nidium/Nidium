/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef graphics_canvashandler_h__
#define graphics_canvashandler_h__

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>

#include <jsapi.h>

#include "Core/Events.h"
#include "Frontend/InputHandler.h"
#include "Graphics/Geometry.h"

#include <Yoga.h>
#include <YGStringEnums.h>

#ifndef NAN                                                                     
static const unsigned long __nan[2] = {0xffffffff, 0x7fffffff};                 
#define NAN (*(const float *) __nan)                                            
#endif                                                                          


/*
    - Handle a canvas layer.
    - Agnostic to any renderer.
    - All size are in logical pixels (device ratio is handled by CanvasContext)
*/

namespace Nidium {
namespace Interface {
class UIInterface;
}
namespace Binding {
class JSCanvas;
}
namespace Frontend {
class Context;
class InputEvent;
}
namespace Graphics {

class CanvasHandler;
class SkiaContext;
class CanvasContext;

struct ComposeContext
{
    CanvasHandler *handler;
    double left;
    double top;
    double opacity;
    double zoom;
    bool   needClip;
    Rect   clip;
};

struct LayerizeContext
{
    CanvasHandler *m_Layer;
    double m_pLeft;
    double m_pTop;
    double m_aOpacity;
    double m_aZoom;
    Rect *m_Clip;

    void reset()
    {
        m_Layer      = NULL;
        m_pLeft      = 0.;
        m_pTop       = 0.;
        m_aOpacity   = 1.0;
        m_aZoom      = 1.0;
        m_Clip       = NULL;
    }
};

#define CANVAS_DEF_CLASS_PROPERTY(name, type, default_value, state) \
    CanvasProperty<type> p_##name = {#name, default_value, CanvasProperty<type>::state, this}; \
    virtual void setProp##name(type value) { \
        this->p_##name.set(value); \
    } \
    virtual type getProp##name() { \
        return this->p_##name.get(); \
    }

class CanvasHandlerBase
{
private:
    /*
        This need to be initialized before properties
    */    
    std::vector<void *> m_PropertyList;

public:
    /*
     * This class holds two different value for the property
     * - The 'computed' value : the actual value used for the layout
     * - the 'user' value : the value set by the user */
    template <typename T>
    class CanvasProperty
    {
    public:
        enum class State {
            kDefault,
            kSet,
            kInherit,
            kUndefined
        };

        static constexpr float kUndefined_Value  = NAN;

        CanvasProperty(const char *name, T val, State state, CanvasHandlerBase *h) :
            m_Name(name), m_Canvas(h), m_Value(val), m_AlternativeValue(val) {
                
                position = m_Canvas->m_PropertyList.size();
                m_Canvas->m_PropertyList.push_back((void *)this);
            };

        inline T get() const {
            if (m_State == State::kInherit) {
                if (m_Canvas->getParentBase()) {
                    CanvasProperty<T> *ref =
                        static_cast<CanvasProperty<T> *>
                            (m_Canvas->getParentBase()->m_PropertyList.at(position));

                    return ref->get();
                }
            }

            return m_Value;
        }

        inline T getAlternativeValue() const {
            return m_AlternativeValue;
        }

        inline operator T() const {
            return get();
        }
        
        /* Change the computed value */
        inline void set(T val) {
            m_Value = val;
        }

        inline void setAlternativeValue(T val) {
            m_AlternativeValue = val;
        }
        
        /* Change the user value */
        inline void userSet(T val) {
            m_UserValue = val;
            m_State = State::kSet;
        }

        inline CanvasProperty<T> operator=(const T& val) {

            set(val);

            return *this;
        }

        void setInherit() {
            m_State = State::kInherit;
        }

        void reset() {
            m_State = State::kDefault;
        }

    private:
        /* Used for debug purpose
         * TODO: ifdef DEBUG */
        const char *m_Name;

        CanvasHandlerBase *m_Canvas;

        /* Actual value used for computation */
        T m_Value;
        /* Value set by the user */
        T m_UserValue;

        T m_AlternativeValue;
        
        State m_State = State::kDefault;

        /* Position of the property in the Canvas properyList
         * This is used in order to lookup for parent same property */
        int position;
    };

    CANVAS_DEF_CLASS_PROPERTY(Right,        double, NAN, State::kDefault);
    CANVAS_DEF_CLASS_PROPERTY(Bottom,       double, NAN, State::kDefault);
    CANVAS_DEF_CLASS_PROPERTY(Top,          double, 0, State::kDefault);
    CANVAS_DEF_CLASS_PROPERTY(Left,         double, 0, State::kDefault);
    CANVAS_DEF_CLASS_PROPERTY(Width,        int, 1, State::kDefault);
    CANVAS_DEF_CLASS_PROPERTY(Height,       int, 1, State::kDefault);
    CANVAS_DEF_CLASS_PROPERTY(MinWidth,     int, -1, State::kDefault);
    CANVAS_DEF_CLASS_PROPERTY(MinHeight,    int, -1, State::kDefault);
    CANVAS_DEF_CLASS_PROPERTY(MaxWidth,     int, 0, State::kDefault);
    CANVAS_DEF_CLASS_PROPERTY(MaxHeight,    int, 0, State::kDefault);
    CANVAS_DEF_CLASS_PROPERTY(Coating,      unsigned int, 0, State::kDefault);

    CANVAS_DEF_CLASS_PROPERTY(Flex,         bool, false, State::kDefault);

    CANVAS_DEF_CLASS_PROPERTY(Opacity,      double, 1.0, State::kDefault);

    virtual CanvasHandlerBase *getParentBase()=0;
};


// {{{ CanvasHandler
class CanvasHandler : public CanvasHandlerBase, public Core::Events 
{
private:
    /*
        This need to be initialized before properties
    */    
    std::vector<void *> m_PropertyList;

public:
    friend class SkiaContext;
    friend class Nidium::Frontend::Context;
    friend class Binding::JSCanvas;

    static const uint8_t EventID = 1;
    

    enum COORD_MODE
    {
        kLeft_Coord   = 1 << 0,
        kRight_Coord  = 1 << 1,
        kTop_Coord    = 1 << 2,
        kBottom_Coord = 1 << 3
    };

    enum Flags
    {
        kDrag_Flag = 1 << 0
    };

    enum EventsChangedProperty
    {
        kContentHeight_Changed,
        kContentWidth_Changed
    };

    enum Events
    {
        RESIZE_EVENT = 1,
        LOADED_EVENT,
        CHANGE_EVENT,
        MOUSE_EVENT,
        TOUCH_EVENT,
        DRAG_EVENT,
        SCROLL_EVENT,
        PAINT_EVENT,
        MOUNT_EVENT,
        UNMOUNT_EVENT
    };

    enum Position
    {
        POSITION_FRONT,
        POSITION_BACK
    };

    enum COORD_POSITION
    {
        COORD_RELATIVE,
        COORD_ABSOLUTE,
        COORD_FIXED,
        COORD_DEFAULT
    };

    enum Visibility
    {
        CANVAS_VISIBILITY_VISIBLE,
        CANVAS_VISIBILITY_HIDDEN
    };


    CanvasContext *m_Context;
    JS::TenuredHeap<JSObject *> m_JsObj;
    JSContext *m_JsCx;

    struct
    {
        double top;
        double right;
        double bottom;
        double left;
    } m_Margin;

    struct
    {
        double top;
        double right;
        double bottom;
        double left;
    } m_Padding;

    struct
    {
        int width;
        int height;
        int scrollTop;
        int scrollLeft;
        int _width, _height;
    } m_Content;

    struct
    {
        int x, y, xrel, yrel;
        bool consumed;
    } m_MousePosition;

    bool m_Overflow;
    bool m_ScrollableX = false;
    bool m_ScrollableY = false;
    struct CurrentScroll {
        int x;
        int y;
        bool active = false;
    } m_CurrentScroll;

    CanvasContext *getContext() const
    {
        return m_Context;
    }

    double getZoom() const
    {
        return m_Zoom;
    }

    inline double getPropLeftAbsolute()
    {
        return p_Left.getAlternativeValue();
    }

    inline double getPropTopAbsolute()
    {
        return p_Top.getAlternativeValue();
    }

    double getTopScrolled() 
    {
        double top = getPropTop();
        if (m_CoordPosition == COORD_RELATIVE && m_Parent != NULL) {
            top -= m_Parent->m_Content.scrollTop;
        }

        return top;
    }

    double getLeftScrolled()
    {
        double left = getPropLeft();
        if (m_CoordPosition == COORD_RELATIVE && m_Parent != NULL) {
            left -= m_Parent->m_Content.scrollLeft;
        }
        return left;
    }

    /*
        Get the real dimensions computed by Yoga
    */
    void getDimensions(int *width, int *height,
        int *left = nullptr, int *top = nullptr)
    {
        *width = ceilf(YGNodeLayoutGetWidth(m_YogaRef));
        *height = ceilf(YGNodeLayoutGetHeight(m_YogaRef));

        if (left) {
            *left = floorf(YGNodeLayoutGetLeft(m_YogaRef));
        }

        if (top) {
            *top = floorf(YGNodeLayoutGetTop(m_YogaRef));
        }
    }

    inline int getComputedTop() const {
        return floorf(YGNodeLayoutGetTop(m_YogaRef));
    }

    inline int getComputedLeft() const {
        return floorf(YGNodeLayoutGetLeft(m_YogaRef));
    }

    inline int getComputedRight() const {
        return floorf(YGNodeLayoutGetRight(m_YogaRef));
    }

    inline int getComputedBottom() const {
        return floorf(YGNodeLayoutGetBottom(m_YogaRef));
    }

    inline int getComputedWidth() const {
        return ceilf(YGNodeLayoutGetWidth(m_YogaRef));
    }

    inline int getComputedHeight() const {
        return ceilf(YGNodeLayoutGetHeight(m_YogaRef));
    }

    inline int getComputedAbsoluteLeft() const {
        return p_Left.getAlternativeValue();
    }

    inline int getComputedAbsoluteTop() const {
        return p_Top.getAlternativeValue();
    }

    Frontend::Context *getNidiumContext() const
    {
        return m_NidiumContext;
    }

    void setMargin(double top, double right, double bottom, double left)
    {
        YGNodeStyleSetMargin(m_YogaRef, YGEdgeTop, top);
        YGNodeStyleSetMargin(m_YogaRef, YGEdgeRight, right);
        YGNodeStyleSetMargin(m_YogaRef, YGEdgeBottom, bottom);
        YGNodeStyleSetMargin(m_YogaRef, YGEdgeLeft, left);

        m_Margin.top    = top;
        m_Margin.right  = right;
        m_Margin.bottom = bottom;
        m_Margin.left   = left;
    }

    void setPadding(double top, double right, double bottom, double left)
    {
        YGNodeStyleSetPadding(m_YogaRef, YGEdgeTop, top);
        YGNodeStyleSetPadding(m_YogaRef, YGEdgeRight, right);
        YGNodeStyleSetPadding(m_YogaRef, YGEdgeBottom, bottom);
        YGNodeStyleSetPadding(m_YogaRef, YGEdgeLeft, left);

        m_Padding.top    = top;
        m_Padding.right  = right;
        m_Padding.bottom = bottom;
        m_Padding.left   = left;
    }

    void setPropLeft(double val) override
    {
        p_Left.set(val);
        
        YGNodeStyleSetPosition(m_YogaRef, YGEdgeLeft, val);
    }

    void setPropTop(double val) override
    {
        p_Top.set(val);

        YGNodeStyleSetPosition(m_YogaRef, YGEdgeTop, val);
    }

    void setPropRight(double val) override
    {
        p_Left.set(val);
        
        YGNodeStyleSetPosition(m_YogaRef, YGEdgeRight, isnan(val) ? YGUndefined : val);
    }

    void setPropBottom(double val) override
    {
        p_Top.set(val);

        YGNodeStyleSetPosition(m_YogaRef, YGEdgeBottom, isnan(val) ? YGUndefined : val);
    }

    void setPropCoating(unsigned int value) override;

    void setScale(double x, double y);

    void setId(const char *str);

    double getScaleX() const
    {
        return m_ScaleX;
    }

    double getScaleY() const
    {
        return m_ScaleY;
    }

    uint64_t getIdentifier(char **str = NULL)
    {
        if (str != NULL) {
            *str = m_Identifier.str;
        }

        return m_Identifier.idx;
    }

    void setAllowNegativeScroll(bool val)
    {
        m_AllowNegativeScroll = val;
    }

    bool getAllowNegativeScroll() const
    {
        return m_AllowNegativeScroll;
    }

    COORD_POSITION getPositioning() const
    {
        return m_CoordPosition;
    }

    CanvasHandler(int width,
                  int height,
                  Frontend::Context *nctx,
                  bool lazyLoad = false);

    virtual ~CanvasHandler();


    void unrootHierarchy();

    void setContext(CanvasContext *context);

    bool setWidth(int width, bool force = false);
    bool setHeight(int height, bool force = false);

    void setPropMinWidth(int width) override;
    void setPropMinHeight(int height) override;

    void setPropMaxWidth(int width) override;
    void setPropMaxHeight(int height) override;

    void setSize(int width, int height, bool redraw = true);
    void setPositioning(CanvasHandler::COORD_POSITION mode);
    void setScrollTop(int value);
    void setScrollLeft(int value);
    void computeAbsolutePosition();
    bool isOutOfBound();
    Rect getViewport();
    Rect getVisibleRect();

    void bringToFront();
    void sendToBack();
    void addChild(CanvasHandler *insert,
                  CanvasHandler::Position position = POSITION_FRONT);

    void insertBefore(CanvasHandler *insert, CanvasHandler *ref);
    void insertAfter(CanvasHandler *insert, CanvasHandler *ref);

    int getContentWidth();
    int getContentHeight();
    void setHidden(bool val);
    bool isDisplayed() const;
    bool isHidden() const;
    bool hasAFixedAncestor() const;
    void setPropOpacity(double val) override;
    void setZoom(double val);
    void removeFromParent(bool willBeAdopted = false);
    void getChildren(CanvasHandler **out) const;

    bool checkLoaded(bool async = false);

    void setCursor(int cursor);
    int getCursor();

    bool isScrollable() {
        // XXX : Broken with yoga update, needs to be updated once ready
        /*
        return (m_ScrollableX && this->getWidth() < this->getContentWidth(true))
                || (m_ScrollableY && this->getHeight() < this->getContentHeight(true));
        */

        return (m_ScrollableX || m_ScrollableY);
    }

    void scroll(int x, int y);
    void onScroll(int x, int y,
                  int velocityX, int velocityY,
                  Frontend::InputEvent::ScrollState state);

    void invalidate()
    {
        m_NeedPaint = true;
    }

    CanvasHandler *getParent() const
    {
        return m_Parent;
    }

    CanvasHandlerBase *getParentBase() override
    {
        return m_Parent;
    }    

    CanvasHandler *getFirstChild() const
    {
        return m_Children;
    }
    CanvasHandler *getLastChild() const
    {
        return m_Last;
    }
    CanvasHandler *getNextSibling() const
    {
        return m_Next;
    }
    CanvasHandler *getPrevSibling() const
    {
        return m_Prev;
    }
    int32_t countChildren() const;
    bool containsPoint(double x, double y);
    void layerize(LayerizeContext &layerContext,
        std::vector<ComposeContext> &compList, bool draw);

    CanvasHandler *m_Parent;
    CanvasHandler *m_Children;

    CanvasHandler *m_Next;
    CanvasHandler *m_Prev;
    CanvasHandler *m_Last;

    bool _handleEvent(Frontend::InputEvent *ev);

    uint32_t m_Flags;

    void computeLayoutPositions();

protected:

    void paint();
    void propertyChanged(EventsChangedProperty property);

private:
    void deviceSetSize(int width, int height);
    void onTouch(Frontend::InputEvent *ev, Core::Args &args, CanvasHandler *handler);
    void onInputEvent(Frontend::InputEvent *ev);
    void onDrag(Frontend::InputEvent *ev, CanvasHandler *target, bool end = false);
    void onDrop(Frontend::InputEvent *ev, CanvasHandler *droped);

    void checkDrag(Frontend::InputEvent *ev,
                   Graphics::CanvasHandler *drag);
    void checkDrop(Frontend::InputEvent *ev,
                   Graphics::CanvasHandler *drag);

    int32_t m_nChildren;
    void dispatchMouseEvents(LayerizeContext &layerContext);
    COORD_POSITION m_CoordPosition;
    Visibility m_Visibility;

    double m_Zoom;

    double m_ScaleX, m_ScaleY;
    bool m_AllowNegativeScroll;

    Frontend::Context *m_NidiumContext;

    struct
    {
        uint64_t idx;
        char *str;
    } m_Identifier;

    void recursiveScale(double x, double y, double oldX, double oldY);

    bool m_Loaded;
    int m_Cursor;
    bool m_NeedPaint = true;

    /* Reference to the Yoga node */
    YGNodeRef m_YogaRef;
};


} // namespace Graphics
} // namespace Nidium

#endif
