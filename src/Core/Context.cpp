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
#ifdef NIDIUM_PRODUCT_FRONTEND
#include "Interface/SystemInterface.h"
#endif

using namespace Nidium::Binding;
using namespace Nidium::Core;
using namespace Nidium::IO;
using namespace Nidium::Net;

class NidiumJS *g_nidiumjs = nullptr;

namespace Nidium {
namespace Core {

static void context_log(void *ctx, void *cb_args, ape_log_lvl_t lvl,
    const char *tag, const char *buff);

Context::Context(ape_global *ape) : m_APECtx(ape)
{
    m_JS = g_nidiumjs = new NidiumJS(ape, this);
    
    Path::RegisterScheme(SCHEME_DEFINE("file://", FileStream, false), true);
    Path::RegisterScheme(SCHEME_DEFINE("http://", HTTPStream, true));
    Path::RegisterScheme(SCHEME_DEFINE("https://", HTTPStream, true));

    TaskManager::CreateManager();
    Messages::InitReader(ape);

    m_JS->loadGlobalObjects();

    m_PingTimer = APE_timer_create(ape, 1, Ping, (void *)m_JS);
#ifdef DEBUG
    ape_log_lvl_t verblvl = APE_LOG_DEBUG;
#else
    ape_log_lvl_t verblvl = APE_LOG_ERROR;
#endif

    char *env_verbose = getenv("NIDIUM_VERBOSITY");
    if (env_verbose) {
        verblvl = (ape_log_lvl_t)nidium_min(nidium_max(0, atoi(env_verbose)), APE_LOG_COUNT-1);
    }

    APE_setlogger(verblvl, nullptr, context_log, nullptr, this);
}

int Context::Ping(void *arg)
{
    static uint64_t framecount = 0;
    NidiumJS *js               = static_cast<NidiumJS *>(arg);

    if (++framecount % 1000 == 0) {
        js->gc();
    }

    return 8;
}

void Context::logFlush()
{
    m_LogBuffering = false;

    if (m_Logbuffer.length()) {
        this->postMessageSync(strdup(m_Logbuffer.c_str()), kContextMessage_log);
        m_Logbuffer.clear();
        m_Logbuffer.shrink_to_fit();
    }
}

void Context::log(const char *str)
{
    if (m_LogBuffering) {
        m_Logbuffer += str;

        return;
    }
    this->postMessageSync(strdup(str), kContextMessage_log);
}

void Context::vlog(const char *format, ...)
{
    va_list args;

    va_start(args, format);
    this->vlog(format, args);
    va_end(args);
}

void Context::vlog(const char *format, va_list args)
{
    char *buff;

    vasprintf(&buff, format, args);

    this->log(buff);

    free(buff);
}

static void context_log(void *ctx, void *cb_args, ape_log_lvl_t lvl,
    const char *tag, const char *buff)
{
    Context *_this = (Context *)ctx;

    if (tag) {
        _this->vlog("[%s:%s] %s\n", APE_getloglabel(lvl), tag, buff);
    } else {
        _this->vlog("[%s] %s\n", APE_getloglabel(lvl), buff);
    }
}


Context::~Context()
{
    APE_timer_destroy(m_APECtx, m_PingTimer);
    destroyJS();

    /*
        We need to cleanup the message queue beforehand
        because the Messages base classe would try to call "delMessageForDest"
        while the object was already cleaned up by ~DestroyReader()
    */
    Messages::cleanupMessages();

    Messages::DestroyReader();
}

void Context::onMessage(const SharedMessages::Message &msg)
{
    switch (msg.event()) {
        case kContextMessage_log:
        {
            const char *str = (char *)msg.dataPtr();

#ifdef NIDIUM_PRODUCT_FRONTEND
            Interface::SystemInterface::GetInstance()->print(str);
#else
            fwrite(str, 1, strlen(str), stdout);
#endif

            free(msg.dataPtr());
        }
        default:
        break;
    }
}

void Context::onMessageLost(const SharedMessages::Message &msg)
{
    switch (msg.event()) {
        case kContextMessage_log:
        {
            free(msg.dataPtr());
        }
        default:
        break;
    }
}


} // namespace Core
} // namespace Nidium
