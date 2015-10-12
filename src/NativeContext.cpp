#define _HAVE_SSL_SUPPORT 1

#include "NativeContext.h"
#include "NativeMacros.h"
#include <NativeJS.h>
#include "NativeJSConsole.h"
#include "NativeJSSystem.h"
#include <NativeMessages.h>
#include <NativeStreamInterface.h>
#include <NativeFileStream.h>
#include <NativeHTTPStream.h>
#include <NativePath.h>
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
    bool jsstrict) :
    m_Worker(worker)
{

    //NativePath::cd("/Users/paraboul/dev/");
    //NativePath::chroot("/Users/paraboul/dev/");

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

