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
namespace Graphics {

class CanvasHandler;
class SkiaContext;
class CanvasContext;

struct ComposeContext
{
    CanvasHandler *handler;
    float left;
    float top;
    double opacity;
    double zoom;
    bool   needClip;
    Rect   clip;
};

struct LayerizeContext
{
    CanvasHandler *m_Layer;
    float m_pLeft;
    float m_pTop;
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

#define CANVAS_DEF_CLASS_CARDINAL_PROPERTY(name, type, default_value, state) \
    CANVAS_DEF_CLASS_PROPERTY(name##Top, type, default_value, state) \
    CANVAS_DEF_CLASS_PROPERTY(name##Right, type, default_value, state) \
    CANVAS_DEF_CLASS_PROPERTY(name##Bottom, type, default_value, state) \
    CANVAS_DEF_CLASS_PROPERTY(name##Left, type, default_value, state)

#define CANVAS_DEF_PROP_YOGA_SETTER(name, position) \
    void setProp##name##position(float val) override \
    { \
        p_##name##position.set(val); \
        if (p_##name##position.isPercentageValue()) { \
            YGNodeStyleSet##name##Percent(m_YogaRef, YGEdge##position, isnan(val) ? YGUndefined : val); \
        } else { \
            YGNodeStyleSet##name(m_YogaRef, YGEdge##position, isnan(val) ? YGUndefined : val); \
        } \
    }

#define CANVAS_DEF_PROP_YOGA_SETTER_POSITION(position) \
    void setProp##position(float val) override \
    { \
        p_##position.set(val); \
        if (p_##position.isPercentageValue()) { \
            YGNodeStyleSetPositionPercent(m_YogaRef, YGEdge##position, isnan(val) ? YGUndefined : val); \
        } else { \
            YGNodeStyleSetPosition(m_YogaRef, YGEdge##position, isnan(val) ? YGUndefined : val); \
        } \
    }

#define CANVAS_DEF_PROP_CARDINAL_YOGA_SETTER(name) \
    CANVAS_DEF_PROP_YOGA_SETTER(name, Top) \
    CANVAS_DEF_PROP_YOGA_SETTER(name, Right) \
    CANVAS_DEF_PROP_YOGA_SETTER(name, Bottom) \
    CANVAS_DEF_PROP_YOGA_SETTER(name, Left)


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
            m_Name(name), m_Canvas(h), m_Value(val), m_CachedValue(val) {
#if 0
                position = m_Canvas->m_PropertyList.size();
                m_Canvas->m_PropertyList.push_back((void *)this);
#endif
            };

        inline T get() const {
#if 0
            if (m_State == State::kInherit) {
                if (m_Canvas->getParentBase()) {
                    CanvasProperty<T> *ref =
                        static_cast<CanvasProperty<T> *>
                            (m_Canvas->getParentBase()->m_PropertyList.at(position));

                    return ref->get();
                }
            }
#endif
            return m_Value;
        }

        inline T getCachedValue() const {
            return m_CachedValue;
        }


        inline operator T() const {
            return get();
        }
        
        /* Change the computed value */
        inline void set(T val) {
            m_Value = val;
        }

        inline void setCachedValue(T val) {
            m_CachedValue = val;
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

        void setIsPercentageValue(bool val) {
            m_IsPercentage = val;
        }

        bool isPercentageValue() const {
            return m_IsPercentage;
        }

    private:
        /* Used for debug purpose
         * TODO: ifdef DEBUG */
        const char *m_Name;

        CanvasHandlerBase *m_Canvas;

        /* Actual value used for computation */
        T m_Value;
        T m_CachedValue;

        bool m_IsPercentage = false;
        State m_State = State::kDefault;

        /* Position of the property in the Canvas properyList
         * This is used in order to lookup for parent same property */
        int position;
    };

    CANVAS_DEF_CLASS_PROPERTY(Right,        float, NAN, State::kDefault);
    CANVAS_DEF_CLASS_PROPERTY(Bottom,       float, NAN, State::kDefault);
    CANVAS_DEF_CLASS_PROPERTY(Top,          float, NAN, State::kDefault);
    CANVAS_DEF_CLASS_PROPERTY(Left,         float, NAN, State::kDefault);
    CANVAS_DEF_CLASS_PROPERTY(Width,        float, NAN, State::kDefault);
    CANVAS_DEF_CLASS_PROPERTY(Height,       float, NAN, State::kDefault);
    CANVAS_DEF_CLASS_PROPERTY(MinWidth,     float, -1,  State::kDefault);
    CANVAS_DEF_CLASS_PROPERTY(MinHeight,    float, -1,  State::kDefault);
    CANVAS_DEF_CLASS_PROPERTY(MaxWidth,     float, 0,   State::kDefault);
    CANVAS_DEF_CLASS_PROPERTY(MaxHeight,    float, 0,   State::kDefault);
    CANVAS_DEF_CLASS_PROPERTY(Coating,      float, 0,   State::kDefault);
    CANVAS_DEF_CLASS_PROPERTY(EventReceiver,bool, true, State::kDefault);
    CANVAS_DEF_CLASS_PROPERTY(Display,      bool, true, State::kDefault);
    CANVAS_DEF_CLASS_PROPERTY(Opacity,      float, 1.0, State::kDefault);
    CANVAS_DEF_CLASS_CARDINAL_PROPERTY(Margin,   float, NAN, State::kDefault);
    CANVAS_DEF_CLASS_CARDINAL_PROPERTY(Padding,  float, NAN, State::kDefault);

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
        kEvents_Resize = 1,
        kEvents_Loaded,
        kEvents_Change,
        kEvents_Mouse,
        kEvents_Drag,
	kEvents_Scroll,
        kEvents_Paint,
        kEvents_Mount,
        kEvents_Unmount
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

    CanvasContext *getContext() const
    {
        return m_Context;
    }

    double getZoom() const
    {
        return m_Zoom;
    }

    inline float getPropLeftAbsolute()
    {
        return p_Left.getCachedValue();
    }

    inline float getPropTopAbsolute()
    {
        return p_Top.getCachedValue();
    }

    float getTopScrolled() 
    {
        float top = getPropTop();
        if (m_CoordPosition == COORD_RELATIVE && m_Parent != NULL) {
            top -= m_Parent->m_Content.scrollTop;
        }

        return top;
    }

    float getLeftScrolled()
    {
        float left = getPropLeft();
        if (m_CoordPosition == COORD_RELATIVE && m_Parent != NULL) {
            left -= m_Parent->m_Content.scrollLeft;
        }
        return left;
    }

    /*
        Get the real dimensions computed by Yoga
    */
    bool getDimensions(float *width, float *height,
        float *left = nullptr, float *top = nullptr)
    {
        *width = YGNodeLayoutGetWidth(m_YogaRef);
        *height = YGNodeLayoutGetHeight(m_YogaRef);

        if (isnan(*width) || isnan(*height)) {
            return false;
        }

        if (left) {
            *left = YGNodeLayoutGetLeft(m_YogaRef);
            if (isnan(*left)) {
                return false;
            }
        }

        if (top) {
            *top = YGNodeLayoutGetTop(m_YogaRef);
            if (isnan(*top)) {
                return false;
            }
        }

        return true;
    }

    inline float getComputedTop() const {
        return YGNodeLayoutGetTop(m_YogaRef);
    }

    inline float getComputedLeft() const {
        return YGNodeLayoutGetLeft(m_YogaRef);
    }

    inline float getComputedRight() const {
        return YGNodeLayoutGetRight(m_YogaRef);
    }

    inline float getComputedBottom() const {
        return YGNodeLayoutGetBottom(m_YogaRef);
    }

    inline float getComputedWidth() const {
        return YGNodeLayoutGetWidth(m_YogaRef);
    }

    inline float getComputedHeight() const {
        return YGNodeLayoutGetHeight(m_YogaRef);
    }

    inline float getComputedAbsoluteLeft() const {
        return p_Left.getCachedValue();
    }

    inline float getComputedAbsoluteTop() const {
        return p_Top.getCachedValue();
    }

    Frontend::Context *getNidiumContext() const
    {
        return m_NidiumContext;
    }

    CANVAS_DEF_PROP_CARDINAL_YOGA_SETTER(Padding);
    CANVAS_DEF_PROP_CARDINAL_YOGA_SETTER(Margin);
    CANVAS_DEF_PROP_YOGA_SETTER_POSITION(Top);
    CANVAS_DEF_PROP_YOGA_SETTER_POSITION(Right);
    CANVAS_DEF_PROP_YOGA_SETTER_POSITION(Bottom);
    CANVAS_DEF_PROP_YOGA_SETTER_POSITION(Left);

    void setPropDisplay(bool state) override
    {
        p_Display.set(state);

        YGNodeStyleSetDisplay(m_YogaRef, state ? YGDisplayFlex : YGDisplayNone);
    }

    void setOverflow(bool state) {
        m_Overflow = state;
        /* TODO: We should set YGOverflowScroll only if the view is scrollable */
        YGNodeStyleSetOverflow(m_YogaRef, state ? YGOverflowScroll : YGOverflowVisible);
    }

    bool canOverflow() const {
        return m_Overflow;
    }

    void setPropCoating(float value) override;

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

    CanvasHandler(float width,
                  float height,
                  Frontend::Context *nctx,
                  bool lazyLoad = false);

    virtual ~CanvasHandler();

    void unrootHierarchy();

    void setContext(CanvasContext *context);

    void setPropWidth(float width) override;
    void setPropHeight(float height) override;

    void setPropMinWidth(float width) override;
    void setPropMinHeight(float height) override;

    void setPropMaxWidth(float width) override;
    void setPropMaxHeight(float height) override;

    void setSize(float width, float height, bool redraw = true);
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
    void setPropOpacity(float val) override;
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
    bool containsPoint(float x, float y);
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
    void deviceSetSize(float width, float height);
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


    bool m_Overflow;
    bool m_ScrollableX = false;
    bool m_ScrollableY = false;
    bool m_Loaded;
    int m_Cursor;
    bool m_NeedPaint = true;


    /* Reference to the Yoga node */
    YGNodeRef m_YogaRef;
};


} // namespace Graphics
} // namespace Nidium

#endif
