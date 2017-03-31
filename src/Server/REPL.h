/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef server_repl_h__
#define server_repl_h__

#include <cstdint>
#include <semaphore.h>

#include <ape_buffer.h>
#include <ape_netlib.h>

#include "Core/Messages.h"

namespace Nidium {
namespace Binding {
    class NidiumJS;
}

namespace Server {

class REPL : public Core::Messages
{
public:
    REPL(Binding::NidiumJS *js);
    ~REPL();
    void onMessage(const Core::SharedMessages::Message &msg);
    void onMessageLost(const Core::SharedMessages::Message &msg);

    sem_t *getReadLineLock()
    {
        return &m_ReadLineLock;
    }

    bool isContinuing() const
    {
        return m_Continue;
    }

    int getExitCount() const
    {
        return m_ExitCount;
    }

    int setExitCount(int val)
    {
        m_ExitCount = val;

        return m_ExitCount;
    }

private:
    pthread_t m_ThreadHandle;
    sem_t m_ReadLineLock;

    Binding::NidiumJS *m_JS;

    buffer *m_Buffer;

    bool m_Continue;
    int m_ExitCount;
};

} // namespace Server
} // namespace Nidium

#endif
