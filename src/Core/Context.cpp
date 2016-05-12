/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/

#include "Core/Context.h"
#include "Core/Path.h"
#include "Core/TaskManager.h"
#include "Core/Messages.h"
#include "Net/HTTPStream.h"
#include "IO/FileStream.h"

using namespace Nidium::Binding;
using namespace Nidium::Core;
using namespace Nidium::IO;
using namespace Nidium::Net;


namespace Nidium {
namespace Core {


Context::Context(ape_global *ape) :
    m_APECtx(ape)
{
    m_JS = new NidiumJS(ape);
    m_JS->setPrivate(this);

    Path::RegisterScheme(SCHEME_DEFINE("file://", FileStream, false), true);
    Path::RegisterScheme(SCHEME_DEFINE("http://", HTTPStream,    true));
    Path::RegisterScheme(SCHEME_DEFINE("https://", HTTPStream,    true));

    TaskManager::CreateManager();
    Messages::InitReader(ape);

    m_JS->loadGlobalObjects();

    m_PingTimer = APE_timer_create(ape, 1, Ping, (void *)m_JS);
}

int Context::Ping(void *arg)
{
    static uint64_t framecount = 0;
    NidiumJS *js = static_cast<NidiumJS *>(arg);

    if (++framecount % 1000 == 0) {
        js->gc();
    }

    return 8;
}

Context::~Context()
{
    APE_timer_destroy(m_APECtx, m_PingTimer);
    delete m_JS;
}


} // namespace Core
} // namespace Nidium

