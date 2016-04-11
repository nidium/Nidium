#define _HAVE_SSL_SUPPORT 1

#include <unistd.h>

#include "NativeContext.h"
#include "NativeMacros.h"

#include <JS/NativeJS.h>
#include "NativeJSConsole.h"
#include "NativeJSSystem.h"
#include <Core/NativeMessages.h>
#include <IO/NativeStreamInterface.h>
#include <IO/NativeFileStream.h>
#include <Net/NativeHTTPStream.h>
#include <Core/NativePath.h>
#include <native_netlib.h>


static int NativeContext_ping(void *arg)
{
    static uint64_t framecount = 0;
    NativeJS *js = (NativeJS *)arg;

    if (++framecount % 1000 == 0) {
        js->gc();
    }

    return 8;
}

NativeContext::NativeContext(ape_global *net, NativeWorker *worker,
    bool jsstrict, bool runInREPL) :
    m_Worker(worker), m_RunInREPL(runInREPL)
{
    char cwd[1024];

    memset(&cwd[0], '\0', sizeof(cwd));
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
//        NativePath::cd(cwd);
//        NativePath::chroot(cwd);
    }

    m_JS = new NativeJS(net);
    m_JS->setPrivate(this);
    m_JS->setStrictMode(jsstrict);

    NativePath::registerScheme(SCHEME_DEFINE("file://", NativeFileStream, false), true);
    NativePath::registerScheme(SCHEME_DEFINE("http://",    NativeHTTPStream,    true));
    NativePath::registerScheme(SCHEME_DEFINE("https://",   NativeHTTPStream,    true));

    NativeTaskManager::createManager();
    NativeMessages::initReader(net);
    m_JS->loadGlobalObjects();

    NativeJSconsole::registerObject(m_JS->cx);
    NativeJSSystem::registerObject(m_JS->cx);

    add_timer(&net->timersng, 1, NativeContext_ping, (void *)m_JS);
}

NativeContext::~NativeContext()
{
    delete m_JS;
}

