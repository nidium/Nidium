#ifndef nativecontext_h__
#define nativecontext_h__

#include <Binding/NidiumJS.h>

class NativeWorker;

class NativeContext
{
public:

    NativeContext(ape_global *ape, NativeWorker *worker, bool jsstrict = false, bool runInREPL = false);
    ~NativeContext();

    static NativeContext *GetObject(struct JSContext *cx) {
        return (NativeContext *)Nidium::Binding::NidiumJS::GetObject(cx)->getPrivate();
    }

    static NativeContext *GetObject(Nidium::Binding::NidiumJS *njs) {
        return (NativeContext *)njs->getPrivate();
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

#endif
