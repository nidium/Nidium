#include "NativeREPL.h"
#include "NativeMacros.h"

#include <JS/NativeJS.h>
#include <stdio.h>

enum {
    NATIVE_MESSAGE_READLINE
};

static void *native_repl_thread(void *arg)
{
    NativeREPL *repl = (NativeREPL *)arg;
    char input[1024];
    while (1) {
        if (fgets(input, 1024, stdin)) {
            char *line = (char *)malloc((strlen(input) * sizeof(char)) + 1);
            memcpy(line, input, strlen(input)+1);
            repl->postMessage(line, NATIVE_MESSAGE_READLINE);
        }
    }

    return NULL;
}

NativeREPL::NativeREPL(NativeJS *js)
    : m_JS(js)
{
    m_Buffer = buffer_new(512);

    pthread_create(&m_ThreadHandle, NULL, native_repl_thread, this);

    fwrite("$ ", 1, 2, stdout);
    fflush(stdout);
}


void NativeREPL::onMessage(const NativeSharedMessages::Message &msg)
{
    buffer_append_string(m_Buffer, (char *)msg.dataPtr());

    JS::RootedObject rgbl(m_JS->cx, JS::CurrentGlobalOrNull(m_JS->cx));

    if (JS_BufferIsCompilableUnit(m_JS->cx, rgbl,
        (char *)m_Buffer->data, m_Buffer->used)) {

        m_JS->LoadScriptContent((char *)m_Buffer->data,
            m_Buffer->used, "commandline");

        m_Buffer->used = 0;

        fwrite("$ ", 1, 2, stdout);
        fflush(stdout);
    }

    free(msg.dataPtr());
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

