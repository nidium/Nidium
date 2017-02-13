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
}

}
}