#include "NativeContext.h"
#include "NativeMacros.h"
#include <NativeJS.h>

NativeContext::NativeContext(ape_global *net)
{
    m_JS = new NativeJS(net);
    m_JS->loadGlobalObjects();
    m_JS->setPrivate(this);
}

NativeContext::~NativeContext()
{
    delete m_JS;
}
