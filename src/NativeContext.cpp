#define _HAVE_SSL_SUPPORT 1

#include <unistd.h>

#include <native_netlib.h>

#include <Core/Messages.h>
#include <Core/NativePath.h>
#include <IO/FileStream.h>
#include <Net/HTTPStream.h>
#include <Binding/NidiumJS.h>

#include "NativeJSConsole.h"
#include "NativeJSSystem.h"
#include "NativeContext.h"
#include "NativeMacros.h"


static int NativeContext_ping(void *arg)
{
    static uint64_t framecount = 0;
    Nidium::Binding::NidiumJS *js = (Nidium::Binding::NidiumJS *)arg;

    if (++framecount % 1000 == 0) {
        js->gc();
    }

    return 8;
}

NativeContext::NativeContext(ape_global *net, NativeWorker *worker,
    bool jsstrict, bool runInREPL) :
    m_Worker(worker), m_RunInREPL(runInREPL)
{
    char cwd[PATH_MAX];

    memset(&cwd[0], '\0', sizeof(cwd));
    if (getcwd(cwd, sizeof(cwd)-1) != NULL) {
        strcat(cwd, "/");

        NativePath::cd(cwd);
        NativePath::chroot("/");
    } else {
        fprintf(stderr, "[Warn] Failed to get current working directory\n");
    }

    m_JS = new Nidium::Binding::NidiumJS(net);
    m_JS->setPrivate(this);
    m_JS->setStrictMode(jsstrict);

    NativePath::registerScheme(SCHEME_DEFINE("file://", Nidium::IO::FileStream, false), true);
    NativePath::registerScheme(SCHEME_DEFINE("http://", Nidium::Net::HTTPStream,    true));
    NativePath::registerScheme(SCHEME_DEFINE("https://", Nidium::Net::HTTPStream,    true));

    Nidium::Core::TaskManager::createManager();
    Nidium::Core::Messages::initReader(net);
    m_JS->loadGlobalObjects();

    NativeJSconsole::registerObject(m_JS->cx);
    NativeJSSystem::registerObject(m_JS->cx);

    APE_timer_create(net, 1, NativeContext_ping, (void *)m_JS);
}

NativeContext::~NativeContext()
{
    delete m_JS;
}

