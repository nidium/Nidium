#define _HAVE_SSL_SUPPORT 1

#include <unistd.h>

#include <ape_netlib.h>

#include <Core/Messages.h>
#include <Core/Path.h>
#include <IO/FileStream.h>
#include <Net/HTTPStream.h>
#include <Binding/NidiumJS.h>

#include "Server/Context.h"
#include "Server/Macros.h"
#include "Binding/JSConsole.h"
#include "Binding/JSSystem.h"

namespace Nidium {
namespace Server {

// {{{ Static
static int Context_ping(void *arg)
{
    static uint64_t framecount = 0;
    Nidium::Binding::NidiumJS *js = static_cast<Nidium::Binding::NidiumJS *>(arg);

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

        Nidium::Core::Path::CD(cwd);
        Nidium::Core::Path::Chroot("/");
    } else {
        fprintf(stderr, "[Warn] Failed to get current working directory\n");
    }

    m_JS = new Nidium::Binding::NidiumJS(net);
    m_JS->setPrivate(this);
    m_JS->setStrictMode(jsstrict);

    Nidium::Core::Path::RegisterScheme(SCHEME_DEFINE("file://", Nidium::IO::FileStream, false), true);
    Nidium::Core::Path::RegisterScheme(SCHEME_DEFINE("http://", Nidium::Net::HTTPStream,    true));
    Nidium::Core::Path::RegisterScheme(SCHEME_DEFINE("https://", Nidium::Net::HTTPStream,    true));

    Nidium::Core::TaskManager::CreateManager();
    Nidium::Core::Messages::InitReader(net);

    m_JS->loadGlobalObjects();

    JSconsole::RegisterObject(m_JS->cx);
    JSSystem::RegisterObject(m_JS->cx);

    m_JS->setPath(Nidium::Core::Path::GetCwd());

    APE_timer_create(net, 1, Context_ping, (void *)m_JS);
}

Context::~Context()
{
    delete m_JS;
}
// }}}

} // namespace Server
} // namespace Nidium

