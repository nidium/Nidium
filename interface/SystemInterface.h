#ifndef interface_systeminterface_h__
#define interface_systeminterface_h__

#include <stdlib.h>

namespace Nidium {
namespace Interface {

class NativeSystem;

class NativeSystemInterface
{
    public:

        enum AlertType {
            ALERT_WARNING = 0,
            ALERT_INFO = 1,
            ALERT_CRITIC = 2
        };

        virtual float backingStorePixelRatio()=0;
        virtual void openURLInBrowser(const char *url) {
            return;
        }
        virtual const char *execute(const char *cmd) {
            return NULL;
        }
        virtual const char *getCacheDirectory()=0;
        virtual const char *getPrivateDirectory()=0;
        virtual const char *getUserDirectory() {
            return "~/";
        };
        virtual void alert(const char *message, AlertType type = ALERT_INFO)=0;
        virtual const char *pwd()=0;
        virtual void sendNotification(const char *title, const char *content, bool sound = false) {};
        static NativeSystemInterface* getInstance()
        {
            return NativeSystemInterface::_interface;
        }
        static NativeSystemInterface *_interface;
        virtual ~NativeSystemInterface() {};
    private:
        void operator=(NativeSystem const&);
    protected:
        float m_fBackingStorePixelRatio;
};

} // namespace Nidium
} // namespace Interface

#endif

