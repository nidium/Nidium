/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/

#ifndef interface_windows_system_h__
#define interface_windows_system_h__


#include "SystemInterface.h"
#include <Windows.h>
#include <Shlobj.h>

namespace Nidium{
namespace Interface{

class System: public SystemInterface
{
public:
    System() = default;
    ~System() = default;
    const char* getCacheDirectory();
    const char* getEmbedDirectory();
    const char* getUserDirectory();
    const char* getLanguage();
    const char* getCwd();
    void sendNotification(const char *title, const char *content, bool sound);
    const char *execute(const char *cmd);

private:
    bool m_SystemUIReady;
    char *m_Emebedpath;
}
} // namepspace Interface
} // namepspace Nidium
#endif