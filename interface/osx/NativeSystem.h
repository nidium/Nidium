#ifndef nativesystem_h__
#define nativesystem_h__

#include "NativeSystemInterface.h"

/*
    Enable Retina support
*/
#define NIDIUM_ENABLE_HIDPI 1

class NativeSystem : public NativeSystemInterface
{
    public:
        NativeSystem();
        ~NativeSystem(){};
        float backingStorePixelRatio();
        const char *getCacheDirectory();
        const char *getPrivateDirectory();
        void alert(const char *message, AlertType type = ALERT_INFO);
};

#endif
