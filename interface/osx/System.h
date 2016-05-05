/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef interface_osx_system_h__
#define interface_osx_system_h__

#include "../SystemInterface.h"

namespace Nidium {
namespace Interface {

/*
    Enable Retina support
*/
#define NIDIUM_ENABLE_HIDPI 1

class System : public SystemInterface
{
    public:
        System();
        ~System() {};
        float backingStorePixelRatio();
        const char *getCacheDirectory();
        const char *getPrivateDirectory();
        const char *getUserDirectory();
        void openURLInBrowser(const char *url);
        const char *pwd();
        const char *getLanguage();
        void alert(const char *message, AlertType type = ALERT_INFO);
        void sendNotification(const char *title, const char *content, bool sound = false);
        const char *execute(const char *cmd);
};

} // namespace Interface
} // namespace Nidium

#endif

