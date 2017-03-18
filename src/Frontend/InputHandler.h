/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef frontend_inputhandler_h__
#define frontend_inputhandler_h__

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <set>
#include <memory>

#include "Graphics/Geometry.h"

typedef int64_t SDL_FingerID;

namespace Nidium {
namespace Graphics {
    class CanvasHandler;
}
}

namespace Nidium {
namespace Frontend {

static const char *InputEvent_Names[]
    = { "mousemove", "mousedown", "mouseup", "dblclick", "dragstart",
        "dragend",   "dragover",  "drop",    "drag",     "mousewheel",
        "touchstart", "touchend", "touchmove" };

class InputTouch;

class InputEvent
{
public:
    enum Type
    {
        kMouseMove_Type = 0,
        kMouseClick_Type,
        kMouseClickRelease_Type,
        kMouseDoubleClick_Type,
        kMouseDragStart_Type,
        kMouseDragEnd_Type,
        kMouseDragOver_Type,
        kMouseDrop_Type,
        kMouseDrag_Type,
        kMouseWheel_Type,
        kTouchStart_Type,
        kTouchEnd_Type,
        kTouchMove_Type
    };

    InputEvent(Type type,
               int ix,
               int iy,
               uint32_t *idata   = NULL,
               uint8_t idata_len = 0)
        : m_x(ix), m_y(iy), m_Next(NULL), m_PassThroughEvent(NULL),
          m_Handler(NULL), m_Origin(NULL), m_depthAffectedCanvas(0),
          m_Type(type)
    {

        if (idata && idata_len <= 8) {
            memcpy(m_data, idata, sizeof(uint32_t) * idata_len);
        }
    }

    InputEvent *dupWithHandler(Graphics::CanvasHandler *handler)
    {
        InputEvent *dup = new InputEvent(*this);
        dup->m_Handler  = handler;
        dup->m_Origin   = this;

        m_PassThroughEvent = dup;

        return dup;
    }

    bool isInRect(Graphics::Rect rect)
    {
        return rect.contains(m_x, m_y);
    }

    Type getType() const
    {
        return m_Type;
    }

    void inc()
    {
        m_depthAffectedCanvas++;
    }

    unsigned getDepth() const
    {
        return m_depthAffectedCanvas;
    }

    static const char *GetName(int type)
    {
        return InputEvent_Names[type];
    }

    InputEvent *getEventForNextCanvas() const
    {
        return m_PassThroughEvent;
    }

    void setData(int index, uint32_t data)
    {
        m_data[index] = data;
    }

    void setTouch(std::shared_ptr<InputTouch> touch) {
        m_Touch = touch;
    }

    std::shared_ptr<InputTouch> getTouch() {
        return m_Touch;
    }

    int m_x, m_y;
    uint32_t m_data[8];
    InputEvent *m_Next;
    InputEvent *m_PassThroughEvent;
    Graphics::CanvasHandler *m_Handler;
    InputEvent *m_Origin;
    unsigned m_depthAffectedCanvas;

private:
    Type m_Type;
    std::shared_ptr<InputTouch> m_Touch;
};

class InputTouch
{
public:
    friend class InputHandler;
    typedef SDL_FingerID TouchID;

    InputTouch(unsigned int x, unsigned int y, TouchID id)
        : x(x), y(y), m_TouchID(id) { }

    unsigned int x = 0;
    unsigned int y = 0;

    unsigned int getIdentifier() {
        return m_Identifier;
    }

    void addOrigin(Graphics::CanvasHandler *handler)
    {
        if (m_Origins.empty()) {
            m_Target = handler;
        }
        m_Origins.insert(handler);
    }

    bool hasOrigin(Graphics::CanvasHandler *handler)
    {
        return m_Origins.find(handler) == m_Origins.end() ? false : true;
    }

    Graphics::CanvasHandler *getTarget()
    {
        return m_Target;
    }

    TouchID getTouchID() {
        return m_TouchID;
    }

private:
    std::set<Graphics::CanvasHandler *> m_Origins {};
    Graphics::CanvasHandler *m_Target = nullptr;
    unsigned int m_Identifier         = -1; /* UINT_MAX */
    TouchID m_TouchID;
};

class InputHandler
{
public:
    void pushEvent(InputEvent *ev);

    void clear();

    InputEvent *getEvents() const
    {
        return m_InputEvents.head;
    }

    void addTouch(std::shared_ptr<InputTouch> touch);
    void rmTouch(unsigned int id);
    void rmChangedTouch(unsigned int id);
    std::shared_ptr<InputTouch> getTouch(InputTouch::TouchID id);
    std::shared_ptr<InputTouch> getTouch(unsigned int id)
    {
        return m_Touches[id];
    }

    std::vector<std::shared_ptr<InputTouch>> getTouches()
    {
        return m_Touches;
    }

    std::set<std::shared_ptr<InputTouch>> getChangedTouches()
    {
        return m_ChangedTouches;
    }

    void addChangedTouch(std::shared_ptr<InputTouch> touch)
    {
        m_ChangedTouches.insert(touch);
    }

private:
    struct
    {
        InputEvent *head = NULL;
        InputEvent *queue = NULL;
    } m_InputEvents;

    std::vector<std::shared_ptr<InputTouch>> m_Touches {};
    std::set<std::shared_ptr<InputTouch>> m_ChangedTouches {};
};

}
}

#endif
