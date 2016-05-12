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

    static Context *GetObject(struct JSContext *cx) {
        return static_cast<Context *>(Binding::NidiumJS::GetObject(cx)->getPrivate());
    }

    static Context *GetObject(Binding::NidiumJS *njs) {
        return static_cast<Context *>(njs->getPrivate());
    }

    Binding::NidiumJS *getNJS() const {
        return m_JS;
    }


protected:
    Binding::NidiumJS *m_JS;
    ape_global *m_APECtx;
    ape_timer_t *m_PingTimer;

    static int Ping(void *arg);
};

} // namespace Core
} // namespace Nidium

#endif