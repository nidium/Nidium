#ifndef serverrepl_h__
#define serverrepl_h__

#include <semaphore.h>

#include <ape_buffer.h>
#include <ape_netlib.h>

#include <Core/Messages.h>

namespace Nidium {
    namespace Binding {
        class NidiumJS;
   }

namespace Server {

class REPL : public Nidium::Core::Messages
{
public:
    REPL(Nidium::Binding::NidiumJS *js);
    ~REPL();
    void onMessage(const Nidium::Core::SharedMessages::Message &msg);
    void onMessageLost(const Nidium::Core::SharedMessages::Message &msg);

    sem_t *getReadLineLock() {
        return &m_ReadLineLock;
    }

    bool isContinuing() const {
        return m_Continue;
    }

    int getExitCount() const {
        return m_ExitCount;
    }

    int setExitCount(int val) {
        m_ExitCount = val;

        return m_ExitCount;
    }
private:
    pthread_t m_ThreadHandle;
    sem_t m_ReadLineLock;

    Nidium::Binding::NidiumJS *m_JS;

    buffer *m_Buffer;

    bool m_Continue;
    int m_ExitCount;
};

} // namespace Server
} // namespace Nidium

#endif

