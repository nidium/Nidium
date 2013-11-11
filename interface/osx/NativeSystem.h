#ifndef nativesystem_h__
#define nativesystem_h__

#include "NativeSystemInterface.h"

/*
    Enable Retina support
*/
#define NIDIUM_ENABLE_HIDPI 0

class NativeSystem : public NativeSystemInterface
{
    public:
        NativeSystem();
        ~NativeSystem(){};
        float backingStorePixelRatio();
        const char *getCacheDirectory();
};

#endif