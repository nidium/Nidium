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
    m_JS = new NidiumJS(ape, this);

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

void Context::log(const char *str)
{
    fwrite(str, sizeof(char), strlen(str), stdout);
}

void Context::vlog(const char *format, ...)
{
    char *buff;
    va_list args;
    va_start(args, format);

    vasprintf(&buff, format, args);

    this->log(buff);

    va_end(args);
}

Context::~Context()
{
    APE_timer_destroy(m_APECtx, m_PingTimer);
    Messages::DestroyReader();
    destroyJS();
}


} // namespace Core
} // namespace Nidium

