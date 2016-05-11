/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef server_context_h__
#define server_context_h__

#include <Binding/Nidiumcore.h>

using Nidium::Binding::Nidiumcore;

namespace Nidium {
namespace Server {

class Worker;

class Context
{
public:
    Context(ape_global *ape, Worker *worker, bool jsstrict = false, bool runInREPL = false);
    ~Context();

    static Context *GetObject(struct JSContext *cx) {
        return static_cast<Context *>(Nidiumcore::GetObject(cx)->getPrivate());
    }

    static Context *GetObject(Nidiumcore *njs) {
        return static_cast<Context *>(njs->getPrivate());
    }

    Nidiumcore *getNJS() const {
        return m_JS;
    }

    Worker *getWorker() const {
        return m_Worker;
    }

    bool isREPL() const {
        return m_RunInREPL;
    }
private:
    Nidiumcore *m_JS;
    Worker *m_Worker;
    bool m_RunInREPL;
};

} // namespace Server
} // namespace Nidium

#endif

