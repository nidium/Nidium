#ifndef nativecontext_h__
#define nativecontext_h__

#include <JS/NativeJS.h>

class NativeWorker;

class NativeContext
{
public:

    NativeContext(ape_global *ape, NativeWorker *worker, bool jsstrict = false);
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
private:
    NativeJS *m_JS;
    NativeWorker *m_Worker;
};

#endif
