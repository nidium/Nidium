#include "Frontend/InputHandler.h"
#include "Graphics/Geometry.h"

namespace Nidium {
namespace Frontend {

void InputHandler::pushEvent(InputEvent &ev)
{
    std::lock_guard<std::mutex> lock(m_PushEventLock);

    m_PendingInputEvents->push_back(std::move(ev));
}

void InputHandler::clear()
{
    std::lock_guard<std::mutex> lock(m_PushEventLock);

    m_InputEvents->clear();
    m_ChangedTouches.clear();

    /*
        Switch pending events & processed events
    */
    std::vector<InputEvent> *tmp = m_InputEvents;
    m_InputEvents = m_PendingInputEvents;
    m_PendingInputEvents = tmp;
}

void InputHandler::addTouch(std::shared_ptr<InputTouch> touch)
{
    for(std::vector<int>::size_type i = 0; i < m_Touches.size(); i++) {
        if (m_Touches[i] == nullptr) {
            touch->m_Identifier = i;
            m_Touches[i] = touch;
            return;
        }
    }

    touch->m_Identifier = m_Touches.size();
    m_Touches.push_back(touch);
}

void InputHandler::rmTouch(unsigned int id)
{
    if (m_Touches[id]) {
        m_Touches[id] = nullptr;
    }
}

std::shared_ptr<InputTouch> InputHandler::getTouch(InputTouch::TouchID id)
{
    for(auto const& touch : m_Touches) {
        if (touch && touch->m_TouchID == id) {
            return touch;
        }
    }

    return nullptr;
}

std::shared_ptr<InputTouch> InputHandler::getKnownTouch(InputTouch::TouchID id)
{
    for(auto const& touch : m_KnownTouch) {
        if (touch && touch->m_TouchID == id) {
            return touch;
        }
    }

    return nullptr;
}

void InputHandler::rmKnownTouch(InputTouch::TouchID id)
{
    for(std::vector<int>::size_type i = 0; i < m_KnownTouch.size(); i++) {
        if (m_KnownTouch[i] && m_KnownTouch[i]->getTouchID() == id) {
            m_KnownTouch[i] = nullptr;
            return;
        }
    }
}

void InputHandler::setCurrentTouchedHandler(unsigned int id, Graphics::CanvasHandler *handler)
{
    if (m_CurrentTouchedHandler.capacity() <= id) {
        m_CurrentTouchedHandler.reserve(id <= 8 ? 8 : id);
    }
    m_CurrentTouchedHandler[id] = handler;
}

Graphics::CanvasHandler *InputHandler::getCurrentTouchHandler(unsigned int id)
{
    if (m_CurrentTouchedHandler.capacity() < id) {
        return nullptr;
    }

    return m_CurrentTouchedHandler[id];
}

}
}
