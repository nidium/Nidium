#include "Frontend/InputHandler.h"
#include "Graphics/Geometry.h"

namespace Nidium {
namespace Frontend {

void InputHandler::pushEvent(InputEvent *ev)
{
    if (m_InputEvents.head == NULL) {
        m_InputEvents.head = ev;
    }

    if (m_InputEvents.queue) {
        m_InputEvents.queue->m_Next = ev;
    }

    m_InputEvents.queue = ev;
}

void InputHandler::clear()
{
    InputEvent *tmp;
    for (InputEvent *ev = m_InputEvents.head; ev != NULL; ev = tmp) {
        tmp = ev->m_Next;

        delete (ev);
    }
    m_InputEvents.head  = NULL;
    m_InputEvents.queue = NULL;

    m_ChangedTouches.clear();
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

}
}
