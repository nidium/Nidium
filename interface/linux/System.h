#ifndef interface_linux_system_h__
#define interface_linux_system_h__

#include "SystemInterface.h"

namespace Nidium {
namespace Interface {

class NativeSystem : public NativeSystemInterface
{
    public:
        NativeSystem();
        ~NativeSystem() {};
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

} // namespace Nidium
} // namespace Interface

#endif

