#include "NativeSystem.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>

NativeSystem::NativeSystem()
{
    fbackingStorePixelRatio = 1.0;
}

float NativeSystem::backingStorePixelRatio()
{
    return fbackingStorePixelRatio;
}

const char *NativeSystem::getCacheDirectory()
{
    char *homedir = getenv("HOME");
    char nHome[1024];

    if (!homedir) {
        homedir = getpwuid(getuid())->pw_dir;
    }

    sprintf(nHome, "%s/.nidium/", homedir);

    if (mkdir(nHome, 0777) == -1 && errno != EEXIST) {
        printf("Cant create cache directory %s\n", nHome);
        return NULL;
    }

    return strdup(nHome);
}
