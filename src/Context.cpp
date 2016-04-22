#define _HAVE_SSL_SUPPORT 1

#include <unistd.h>

#include <ape_netlib.h>

#include <Core/Messages.h>
#include <Core/Path.h>
#include <IO/FileStream.h>
#include <Net/HTTPStream.h>
#include <Binding/NidiumJS.h>

#include "JSConsole.h"
#include "JSSystem.h"
#include "Context.h"
#include "Macros.h"

namespace Nidium {
namespace Server {

// {{{ Static
static int Context_ping(void *arg)
{
    static uint64_t framecount = 0;
    Nidium::Binding::NidiumJS *js = (Nidium::Binding::NidiumJS *)arg;

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

        Nidium::Core::Path::cd(cwd);
        Nidium::Core::Path::chroot("/");
    } else {
        fprintf(stderr, "[Warn] Failed to get current working directory\n");
    }

    m_JS = new Nidium::Binding::NidiumJS(net);
    m_JS->setPrivate(this);
    m_JS->setStrictMode(jsstrict);

    Nidium::Core::Path::registerScheme(SCHEME_DEFINE("file://", Nidium::IO::FileStream, false), true);
    Nidium::Core::Path::registerScheme(SCHEME_DEFINE("http://", Nidium::Net::HTTPStream,    true));
    Nidium::Core::Path::registerScheme(SCHEME_DEFINE("https://", Nidium::Net::HTTPStream,    true));

    Nidium::Core::TaskManager::createManager();
    Nidium::Core::Messages::initReader(net);

    m_JS->loadGlobalObjects();

    JSconsole::registerObject(m_JS->cx);
    JSSystem::registerObject(m_JS->cx);

    m_JS->setPath(Nidium::Core::Path::getPwd());

    APE_timer_create(net, 1, Context_ping, (void *)m_JS);
}

Context::~Context()
{
    delete m_JS;
}
// }}}

} // namespace Server
} // namespace Nidium

