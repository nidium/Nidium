#ifndef nativecontext_h__
#define nativecontext_h__

#include <NativeJS.h>

class NativeContext
{
public:

    NativeContext(ape_global *ape);
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
private:
    NativeJS *m_JS;
};

#endif