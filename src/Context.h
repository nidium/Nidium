#ifndef server_context_h__
#define server_context_h__

#include <Binding/NidiumJS.h>

namespace Nidium {
namespace Server {

class Worker;

class Context
{
public:

    Context(ape_global *ape, Worker *worker, bool jsstrict = false, bool runInREPL = false);
    ~Context();

    static Context *GetObject(struct JSContext *cx) {
        return (Context *)Nidium::Binding::NidiumJS::GetObject(cx)->getPrivate();
    }

    static Context *GetObject(Nidium::Binding::NidiumJS *njs) {
        return (Context *)njs->getPrivate();
    }

    Nidium::Binding::NidiumJS *getNJS() const {
        return m_JS;
    }

    Worker *getWorker() const {
        return m_Worker;
    }

    bool isREPL() const {
        return m_RunInREPL;
    }
private:
    Nidium::Binding::NidiumJS *m_JS;
    Worker *m_Worker;
    bool m_RunInREPL;
};

} // namespace Server
} // namespace Nidium

#endif

