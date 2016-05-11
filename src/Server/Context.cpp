/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#define _HAVE_SSL_SUPPORT 1
#include "Server/Context.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <IO/FileStream.h>
#include <Net/HTTPStream.h>


using Nidium::Core::Path;
using Nidium::Core::TaskManager;
using Nidium::Core::Messages;
using Nidium::Binding::NidiumJS;

namespace Nidium {
namespace Server {

// {{{ Static
static int Context_ping(void *arg)
{
    static uint64_t framecount = 0;
    NidiumJS *js = static_cast<NidiumJS *>(arg);

    if (++framecount % 1000 == 0) {
        js->gc();
    }

    return 8;
}
// }}}

// {{{ Context
Context::Context(ape_global *net, Worker *worker,
    bool jsstrict, bool runInREPL) :
    m_Worker(worker), m_RunInREPL(runInREPL)
{
    char cwd[PATH_MAX];

    memset(&cwd[0], '\0', sizeof(cwd));
    if (getcwd(cwd, sizeof(cwd)-1) != NULL) {
        strcat(cwd, "/");

        Path::CD(cwd);
        Path::Chroot("/");
    } else {
        fprintf(stderr, "[Warn] Failed to get current working directory\n");
    }

    m_JS = new NidiumJS(net);
    m_JS->setPrivate(this);
    m_JS->setStrictMode(jsstrict);

    Path::RegisterScheme(SCHEME_DEFINE("file://", Nidium::IO::FileStream, false), true);
    Path::RegisterScheme(SCHEME_DEFINE("http://", Nidium::Net::HTTPStream,    true));
    Path::RegisterScheme(SCHEME_DEFINE("https://", Nidium::Net::HTTPStream,    true));

    TaskManager::CreateManager();
    Messages::InitReader(net);

    m_JS->loadGlobalObjects();

    m_JS->setPath(Path::GetCwd());

    APE_timer_create(net, 1, Context_ping, (void *)m_JS);
}

Context::~Context()
{
    delete m_JS;
}
// }}}

} // namespace Server
} // namespace Nidium

