/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include "Server/REPL.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <signal.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/types.h>

#ifdef _MSC_VER
#include "Port/MSWindows.h"
#include <Shlobj.h>
#include <WinBase.h>
#else
#include <unistd.h>
#include <pwd.h>
#endif

#include <linenoise.h>

#include "Binding/NidiumJS.h"


using Nidium::Core::SharedMessages;

namespace Nidium {
namespace Server {

enum ReplMessage
{
    kReplMessage_Readline
};

static void *nidium_repl_thread(void *arg)
{
    REPL *repl = static_cast<REPL *>(arg);
    char *line;
    const char *homedir;
    char historyPath[PATH_MAX];

    if ((homedir = getenv("HOME")) == NULL) {
#ifdef _MSC_VER
        WCHAR wc[MAX_PATH];
        char path[MAX_PATH];
        if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_PROFILE, NULL, 0, wc))) {
           sprintf(path, "%ws", path);
           homedir = path;
        }
#else
        homedir = getpwuid(getuid())->pw_dir;
#endif
    }

    sprintf(historyPath, "%s/%s", homedir, ".nidium-repl-history");

    linenoiseInit();
    linenoiseHistoryLoad(historyPath);

repl:
    while ((line = linenoise(repl->isContinuing() ? "... " : "nidium> "))
           != NULL) {
        repl->setExitCount(0);

        linenoiseHistoryAdd(line);
        linenoiseHistorySave(historyPath);

        repl->postMessage(line, kReplMessage_Readline);

        sem_wait(repl->getReadLineLock());
    }

    int exitcount = repl->getExitCount();

    repl->setExitCount(exitcount + 1);

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


void REPL::onMessage(const SharedMessages::Message &msg)
{
    buffer_append_string(m_Buffer, static_cast<char *>(msg.dataPtr()));

    JS::RootedObject rgbl(m_JS->m_Cx, JS::CurrentGlobalOrNull(m_JS->m_Cx));

    if (JS_BufferIsCompilableUnit(m_JS->m_Cx, rgbl, (char *)m_Buffer->data,
                                  m_Buffer->used)) {

        m_Continue = false;

        char *ret = m_JS->LoadScriptContentAndGetResult(
            reinterpret_cast<char *>(m_Buffer->data), m_Buffer->used,
            "commandline");

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

void REPL::onMessageLost(const SharedMessages::Message &msg)
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
