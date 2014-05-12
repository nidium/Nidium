#include "NativeContext.h"
#include "NativeMacros.h"
#include <NativeJS.h>
#include "NativeJSConsole.h"
#include <NativeMessages.h>
#include <NativeStreamInterface.h>
#include <NativeFileStream.h>

NativeContext::NativeContext(ape_global *net)
{

    m_JS = new NativeJS(net);
    m_JS->setPrivate(this);

    NativePath::registerScheme(SCHEME_DEFINE("file://", NativeFileStream, false), true);
    NativeTaskManager::createManager();
    NativeMessages::initReader(net);    
    m_JS->loadGlobalObjects();

    NativeJSconsole::registerObject(m_JS->cx);

}

NativeContext::~NativeContext()
{
    delete m_JS;
}
