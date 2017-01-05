/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <pwd.h>
#include <locale.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>

#include "System.h"
#include "Macros.h"
#include <libgen.h>
#include <string>

namespace Nidium {
namespace Interface {

// {{{ System
System::System() : m_SystemUIReady(false)
{
    char procPath[PATH_MAX];
    char nidiumPath[PATH_MAX];
    pid_t pid = getpid();

    sprintf(procPath, "/proc/%d/exe", pid);

    if (readlink(procPath, nidiumPath, PATH_MAX) == -1)
        m_EmbedPath = nullptr;
    else {
        const char *embed = "src/Embed/";
        char *dir         = dirname(dirname(nidiumPath));
        size_t len        = strlen(dir) + strlen(embed) + 2;

        m_EmbedPath = static_cast<char *>(malloc(sizeof(char) * len));

        snprintf(m_EmbedPath, len, "%s/%s", dir, embed);
    }
}

System::~System()
{
}

float System::backingStorePixelRatio()
{
    return 1;
}

const char *System::getEmbedDirectory()
{
    return m_EmbedPath;
}

const char *System::getCacheDirectory()
{
    const char *homedir = getUserDirectory();
    char nHome[4096];

    snprintf(nHome, 4096, "%s.config/nidium/", homedir);

    if (mkdir(nHome, 0755) == -1 && errno != EEXIST) {
        fprintf(stderr, "Cant create cache directory %s\n", nHome);
        return NULL;
    }

    return strdup(nHome);
}

const char *System::getUserDirectory()
{
    static char retHome[4096];

    char *homedir = getenv("HOME");
    if (!homedir) {
        homedir = getpwuid(getuid())->pw_dir;
        LOGD("homdir set to %s", homedir);
    }

    snprintf(retHome, 4096, "%s/", homedir);

    return retHome;
}

void System::alert(const char *message, AlertType type)
{
    LOGI("%s", message);
#if 0
    switch (type) {
        case ALERT_WARNING:
            break;
        case ALERT_CRITIC:
            break;
        case ALERT_INFO:
            break;
        default:
            break;
    }
#endif
}


const char *System::cwd()
{
    static char dir[MAXPATHLEN];

    getcwd(dir, MAXPATHLEN);

    return dir;
}
const char *System::getLanguage()
{
    return nullptr;
}

void System::sendNotification(const char *title,
                              const char *content,
                              bool sound)
{
}

const char *System::execute(const char *cmd)
{
    return nullptr;
}
// }}}

} // namespace Interface
} // namespace Nidium
