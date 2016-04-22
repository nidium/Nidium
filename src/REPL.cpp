#include "REPL.h"
#include "Macros.h"

#include <Binding/NidiumJS.h>
#include "external/linenoise.h"

#include <stdio.h>
#include <errno.h>
#include <pwd.h>

namespace Nidium {
namespace Server {

enum {
    MESSAGE_READLINE
};

static void *nidium_repl_thread(void *arg)
{
    REPL * repl = (REPL *)arg;
    char *line;
    const char *homedir;
    char historyPath[PATH_MAX];

    if ((homedir = getenv("HOME")) == NULL) {
        homedir = getpwuid(getuid())->pw_dir;
    }

    sprintf(historyPath, "%s/%s", homedir, ".nidium-repl-history");

    linenoiseHistoryLoad(historyPath);

repl:
    while ((line = linenoise(repl->isContinuing() ? "... " : "nidium> ")) != NULL) {
        repl->setExitCount(0);

        linenoiseHistoryAdd(line);
        linenoiseHistorySave(historyPath);

        repl->postMessage(line, MESSAGE_READLINE);

        sem_wait(repl->getReadLineLock());
    }

    int exitcount = repl->getExitCount();

    repl->setExitCount(exitcount+1);

    if (exitcount == 0) {
        printf("(To exit, press ^C again)\n");
        goto repl;
    }

    kill(getppid(), SIGINT);

    return NULL;
}

// {{{ REPL
REPL::REPL(Nidium::Binding::NidiumJS *js)
    : m_JS(js), m_Continue(false), m_ExitCount(0)
{
    m_Buffer = buffer_new(512);

    sem_init(&m_ReadLineLock, 0, 0);

    pthread_create(&m_ThreadHandle, NULL, nidium_repl_thread, this);
}


void REPL::onMessage(const Nidium::Core::SharedMessages::Message &msg)
{
    buffer_append_string(m_Buffer, (char *)msg.dataPtr());

    JS::RootedObject rgbl(m_JS->cx, JS::CurrentGlobalOrNull(m_JS->cx));

    if (JS_BufferIsCompilableUnit(m_JS->cx, rgbl,
        (char *)m_Buffer->data, m_Buffer->used)) {

        m_Continue = false;

        char *ret = m_JS->LoadScriptContentAndGetResult((char *)m_Buffer->data,
            m_Buffer->used, "commandline");

        if (ret) {
            printf("%s\n", ret);
            free(ret);
        }

        m_Buffer->used = 0;
    } else {
        m_Continue = true;
    }

    free(msg.dataPtr());

    sem_post(&m_ReadLineLock);
}

void REPL::onMessageLost(const Nidium::Core::SharedMessages::Message &msg)
{
    free(msg.dataPtr());
}

REPL::~REPL()
{
    /*
        TODO : stop thread and pthread_join()
    */
    buffer_destroy(m_Buffer);
}
// }}}

} // namespace Server
} // namespace Nidium

