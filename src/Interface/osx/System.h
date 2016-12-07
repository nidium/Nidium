/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef interface_osx_system_h__
#define interface_osx_system_h__

#include "../SystemInterface.h"
#include <sys/param.h> /* for MAXPATHLEN */

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
    ~System(){};
    float backingStorePixelRatio();
    const char *getCacheDirectory();
    const char *getEmbedDirectory();
    const char *getUserDirectory();
    void openURLInBrowser(const char *url);
    const char *cwd();
    const char *getLanguage();
    void alert(const char *message, AlertType type = ALERT_INFO);
    void sendNotification(const char *title,
                          const char *content,
                          bool sound = false);
    const char *execute(const char *cmd);

private:
    char m_EmbedPath[MAXPATHLEN];
};

} // namespace Interface
} // namespace Nidium

#endif
