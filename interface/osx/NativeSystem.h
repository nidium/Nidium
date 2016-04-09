/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef nativesystem_h__
#define nativesystem_h__

#include "../NativeSystemInterface.h"

/*
    Enable Retina support
*/
#define NIDIUM_ENABLE_HIDPI 1

class NativeSystem : public NativeSystemInterface
{
    public:
        NativeSystem();
        ~NativeSystem() {};
        float backingStorePixelRatio();
        const char *getCacheDirectory();
        const char *getPrivateDirectory();
        const char *getUserDirectory();
        void openURLInBrowser(const char *url);
        const char *pwd();
        void alert(const char *message, AlertType type = ALERT_INFO);
        void sendNotification(const char *title, const char *content, bool sound = false);
        const char *execute(const char *cmd);
};

#endif

