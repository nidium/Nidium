#include "NativeContext.h"
#include "NativeMacros.h"
#include <NativeJS.h>
#include "NativeJSConsole.h"
#include <NativeMessages.h>
#include <NativeStreamInterface.h>
#include <NativeFileStream.h>
#include <NativeHTTPStream.h>
#include <NativePath.h>

NativeContext::NativeContext(ape_global *net)
{

    //NativePath::cd("/Users/paraboul/dev/");
    //NativePath::chroot("/Users/paraboul/dev/");

    m_JS = new NativeJS(net);
    m_JS->setPrivate(this);

    NativePath::registerScheme(SCHEME_DEFINE("file://", NativeFileStream, false), true);
    NativePath::registerScheme(SCHEME_DEFINE("http://",    NativeHTTPStream,    true));
    NativePath::registerScheme(SCHEME_DEFINE("https://",   NativeHTTPStream,    true));

    NativeTaskManager::createManager();
    NativeMessages::initReader(net);    
    m_JS->loadGlobalObjects();

    NativeJSconsole::registerObject(m_JS->cx);

}

NativeContext::~NativeContext()
{
    delete m_JS;
}
