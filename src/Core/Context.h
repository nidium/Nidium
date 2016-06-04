/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef core_context_h__
#define core_context_h__

#include "Binding/NidiumJS.h"

typedef struct _ape_timer_t ape_timer_t;
typedef struct _ape_global ape_global;

namespace Nidium {

namespace Binding
{
    class NidiumJS;
}

namespace Core {


class Context
{
public:
    Context(ape_global *ape);
    virtual ~Context();

    template <typename T>
    static T *GetObject(struct JSContext *cx) {
        return static_cast<T *>(Binding::NidiumJS::GetObject(cx)->getContext());
    }

    template <typename T>
    static T *GetObject(Binding::NidiumJS *njs) {
        return static_cast<T *>(njs->getContext());
    }

    Binding::NidiumJS *getNJS() const {
        return m_JS;
    }

    virtual void log(const char *str);
    virtual void vlog(const char *format, ...);
    virtual void vlog(const char *format, va_list args); 
    virtual void logClear() {};
    virtual void logShow() {};
    virtual void logHide() {};

protected:
    static int Ping(void *arg);

    void destroyJS() {
        if (m_JS) {
            delete m_JS;
            m_JS = NULL;
        }
    }
    Binding::NidiumJS *m_JS;
    ape_global *m_APECtx;
    ape_timer_t *m_PingTimer;
};

} // namespace Core
} // namespace Nidium

#endif
