/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef core_context_h__
#define core_context_h__

#include "Binding/NidiumJS.h"
#include <string>
#include <assert.h>

typedef struct _ape_timer_t ape_timer_t;
typedef struct _ape_global ape_global;

namespace Nidium {

namespace Binding {
class NidiumJS;
}

namespace Core  {


class Context : public Nidium::Core::Messages
{
public:
    Context(ape_global *ape);
    virtual ~Context();

    template <typename T>
    static T *GetObject(struct JSContext *cx)
    {
        return static_cast<T *>(Binding::NidiumJS::GetObject(cx)->getContext());
    }

    template <typename T>
    static T *GetObject(Binding::NidiumJS *njs)
    {
        return static_cast<T *>(njs->getContext());
    }
    template <typename T>
    static T *GetObject()
    {
        return static_cast<T *>(Binding::NidiumJS::GetObject(NULL)->getContext());
    }

    Binding::NidiumJS *getNJS() const
    {
        return m_JS;
    }

    void logStartBuffering() {
        assert(m_LogBuffering == false);

        m_LogBuffering = true;
    }

    void logFlush();

    virtual void onMessage(const SharedMessages::Message &msg) override;
    virtual void onMessageLost(const SharedMessages::Message &msg) override;

    virtual void log(const char *str);
    virtual void vlog(const char *format, ...);
    virtual void vlog_valist(const char *format, va_list args);
    virtual void logClear(){};
    virtual void logShow(){};
    virtual void logHide(){};

protected:

    enum ContextMessage
    {
        kContextMessage_log
    };

    static int Ping(void *arg);

    void destroyJS()
    {
        if (m_JS) {
            delete m_JS;
            m_JS = NULL;
        }
    }
    Binding::NidiumJS *m_JS;
    ape_global *m_APECtx;
    ape_timer_t *m_PingTimer;

    bool m_LogBuffering = false;
private:

    std::string m_Logbuffer;
};

/*
    RAII Helper to enable/flush log buffering
*/
class AutoContextLogBuffering
{
public:
    AutoContextLogBuffering(Context *ctx) : m_Context(ctx)
    {
        m_Context->logStartBuffering();
    }

    ~AutoContextLogBuffering()
    {
        m_Context->logFlush();
    }
private:
    Context *m_Context;
};

} // namespace Core
} // namespace Nidium

#endif
