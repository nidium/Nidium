#include "NativeREPL.h"
#include "NativeMacros.h"

#include <JS/NativeJS.h>
#include <stdio.h>

#include "external/linenoise.h"

enum {
    NATIVE_MESSAGE_READLINE
};

static void *native_repl_thread(void *arg)
{
    NativeREPL *repl = (NativeREPL *)arg;
    char *line;

    while ((line = linenoise("nidium> ")) != NULL) {
        linenoiseHistoryAdd(line);
        repl->postMessage(line, NATIVE_MESSAGE_READLINE);

        sem_wait(repl->getReadLineLock());
    }

    return NULL;
}

NativeREPL::NativeREPL(NativeJS *js)
    : m_JS(js)
{
    m_Buffer = buffer_new(512);

    sem_init(&m_ReadLineLock, 0, 0);

    pthread_create(&m_ThreadHandle, NULL, native_repl_thread, this);
}


void NativeREPL::onMessage(const NativeSharedMessages::Message &msg)
{
    buffer_append_string(m_Buffer, (char *)msg.dataPtr());

    JS::RootedObject rgbl(m_JS->cx, JS::CurrentGlobalOrNull(m_JS->cx));

    if (JS_BufferIsCompilableUnit(m_JS->cx, rgbl,
        (char *)m_Buffer->data, m_Buffer->used)) {

        char *ret = m_JS->LoadScriptContentAndGetResult((char *)m_Buffer->data,
            m_Buffer->used, "commandline");

        if (ret) {
            printf("%s\n", ret);
            free(ret);
        }

        m_Buffer->used = 0;
    }

    free(msg.dataPtr());

    sem_post(&m_ReadLineLock);
}

void NativeREPL::onMessageLost(const NativeSharedMessages::Message &msg)
{
    free(msg.dataPtr());
}

NativeREPL::~NativeREPL()
{
    /*
        TODO : stop thread and pthread_join()
    */
    buffer_destroy(m_Buffer);
}

