#ifndef nativesystem_h__
#define nativesystem_h__

#include "NativeSystemInterface.h"

class NativeSystem : public NativeSystemInterface
{
    public:
        NativeSystem();
        ~NativeSystem(){};
        float backingStorePixelRatio();     
        const char *getCacheDirectory();
        const char *getPrivateDirectory();
        const char *getUserDirectory();
        void alert(const char *message, AlertType type = ALERT_INFO);
        void initSystemUI();
        const char *pwd();
    private:
        bool m_SystemUIReady;
};

#endif
