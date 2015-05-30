#ifndef nativecontext_h__
#define nativecontext_h__

#include <NativeJS.h>

class NativeWorker;

class NativeContext
{
public:

    NativeContext(ape_global *ape, NativeWorker *worker);
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

    void loadNativeObjects();

    NativeWorker *getWorker() const {
        return m_Worker;
    }
private:
    NativeJS *m_JS;
    NativeWorker *m_Worker;
};

#endif
