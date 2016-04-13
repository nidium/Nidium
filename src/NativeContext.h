#ifndef nativecontext_h__
#define nativecontext_h__

#include <Binding/NativeJS.h>

class NativeWorker;

class NativeContext
{
public:

    NativeContext(ape_global *ape, NativeWorker *worker, bool jsstrict = false, bool runInREPL = false);
    ~NativeContext();

    static NativeContext *getNativeClass(struct JSContext *cx) {
        return (NativeContext *)NativeJS::getNativeClass(cx)->getPrivate();
    }

    static NativeContext *getNativeClass(NativeJS *njs) {
        return (NativeContext *)njs->getPrivate();
    }

    NativeJS *getNJS() const {
        return m_JS;
    }

    NativeWorker *getWorker() const {
        return m_Worker;
    }

    bool isREPL() const {
        return m_RunInREPL;
    }
private:
    NativeJS *m_JS;
    NativeWorker *m_Worker;
    bool m_RunInREPL;
};

#endif
