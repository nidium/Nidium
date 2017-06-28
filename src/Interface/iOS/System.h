/*
   Copyright 2017 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#ifndef interface_ios_system_h__
#define interface_ios_system_h__

#include "SystemInterface.h"

namespace Nidium {
namespace Interface {

class System : public SystemInterface
{
public:
    System();
    ~System() {}

    float backingStorePixelRatio()
    {
        return m_BackingStorePixelRatio;
    }
    const char *getCacheDirectory()
    {
        return "tmp";
    }
    const char *getEmbedDirectory()
    {
        return m_EmbedPath;
    }
    const char *getUserDirectory()
    {
        return "Documents/";
    }
    void alert(const char *message, AlertType type = ALERT_INFO)
    {
        fprintf(stderr, "Alert: %s\n", message);
    }
    void initSystemUI() {}
    const char *cwd()
    {
        return "/";
    }
    const char *getLanguage()
    {
        return "en";
    }
    void sendNotification(const char *title, const char *content, bool sound) {}
    const char *execute(const char *cmd) {}
private:
    const char *m_EmbedPath;
    float m_BackingStorePixelRatio = 1;
};

} // namespace Interface
} // namespace Nidium

#endif
