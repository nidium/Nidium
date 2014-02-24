#include "NativeContext.h"
#include "NativeMacros.h"
#include <NativeJS.h>
#include "NativeJSConsole.h"

NativeContext::NativeContext(ape_global *net)
{
    m_JS = new NativeJS(net);
    m_JS->setPrivate(this);
    
    m_JS->loadGlobalObjects();

    NativeJSconsole::registerObject(m_JS->cx);

}

NativeContext::~NativeContext()
{
    delete m_JS;
}
