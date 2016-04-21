#ifndef servercontext_h__
#define servercontext_h__

#include <Binding/NidiumJS.h>

class NativeWorker;

namespace Nidium {
namespace Server {


class Context
{
public:

    Context(ape_global *ape, NativeWorker *worker, bool jsstrict = false, bool runInREPL = false);
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

    NativeWorker *getWorker() const {
        return m_Worker;
    }

    bool isREPL() const {
        return m_RunInREPL;
    }
private:
    Nidium::Binding::NidiumJS *m_JS;
    NativeWorker *m_Worker;
    bool m_RunInREPL;
};

} // namespace Server
} // namespace Nidium

#endif
