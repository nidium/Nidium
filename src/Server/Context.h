/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef server_context_h__
#define server_context_h__

#include <Binding/NidiumJS.h>
#include "Core/Context.h"

namespace Nidium {
namespace Server {

class Worker;

class Context : public Core::Context
{
public:
    Context(ape_global *ape,
            Worker *worker,
            bool jsstrict  = false,
            bool runInREPL = false);
    virtual ~Context();


    Worker *getWorker() const
    {
        return m_Worker;
    }

    bool isREPL() const
    {
        return m_RunInREPL;
    }

    void log(const char *str) override;

private:
    Worker *m_Worker;
    bool m_RunInREPL;
};

} // namespace Server
} // namespace Nidium

#endif
