/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#define _HAVE_SSL_SUPPORT 1
#include "Server/Context.h"

#include <stdio.h>
#include <string.h>

#ifdef _MSC_VER
#include "Port/MSWindows.h"
#include <direct.h>
#else
#include <unistd.h>
#endif

#include <linenoise.h>

#include "IO/FileStream.h"
#include "Net/HTTPStream.h"

using Nidium::Core::Path;
using Nidium::Core::TaskManager;
using Nidium::Core::Messages;
using Nidium::Binding::NidiumJS;

namespace Nidium {
namespace Server {


// {{{ Context
Context::Context(ape_global *net, Worker *worker, bool jsstrict, bool runInREPL)
    : Core::Context(net), m_Worker(worker), m_RunInREPL(runInREPL)
{
    char cwd[PATH_MAX];

    memset(&cwd[0], '\0', sizeof(cwd));
    if (getcwd(cwd, sizeof(cwd) - 1) != NULL) {
        strcat(cwd, "/");

        Path::CD(cwd);
        Path::Chroot("/");
    } else {
        ndm_log(NDM_LOG_WARN, "Context", "Failed to get current working directory");
    }

    m_JS->setStrictMode(jsstrict);
}

void Context::onMessage(const Core::SharedMessages::Message &msg)
{
    switch (msg.event()) {
        case kContextMessage_log:
        {
            const char *str = (char *)msg.dataPtr();

            linenoisePause();
            fwrite(str, 1, strlen(str), stdout);
            linenoiseResume();

            free(msg.dataPtr());
        }
        default:
        break;
    }
}

Context::~Context()
{
}
// }}}

} // namespace Server
} // namespace Nidium
