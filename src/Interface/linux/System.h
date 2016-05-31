/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef interface_linux_system_h__
#define interface_linux_system_h__

#include "SystemInterface.h"

namespace Nidium {
namespace Interface {

class System : public SystemInterface
{
    public:
        System();
        ~System() {};
        float backingStorePixelRatio();
        const char *getCacheDirectory();
        const char *getEmbedDirectory();
        const char *getUserDirectory();
        void alert(const char *message, AlertType type = ALERT_INFO);
        void initSystemUI();
        const char *pwd();
        const char *getLanguage();
    private:
        bool m_SystemUIReady;
};

} // namespace Interface
} // namespace Nidium

#endif

