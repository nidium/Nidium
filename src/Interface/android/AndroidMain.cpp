/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/

#include <jni.h>
#include <string>
#include "Core/Context.h"
#include "ape_netlib.h"

#include "AndroidUIInterface.h"
#include "System.h"
#include <SDL.h>

#define LOG_TAG    __FILE__ ":"
#define LOGI(...)  __android_log_print(ANDROID_LOG_INFO, LOG_TAG,__VA_ARGS__)
#define LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, LOG_TAG,__VA_ARGS__)
#define LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

unsigned long _ape_seed;

namespace Nidium {
namespace Interface {
    SystemInterface *SystemInterface::_interface = nullptr;
    UIInterface *__NidiumUI;
}
}

using namespace Nidium::Interface;

// Entry point called by SDL
int main(int argc, char **argv)
{
    AndroidUIInterface UI;
    __NidiumUI = &UI;

    _ape_seed = time(NULL) ^ (getpid() << 16);

    const char *nml = NULL;
    LOGD("argc=%d\n", argc);
    for (int i = 0; i < argc; i++) {
        LOGD("%s", argv[i]);
    }
    nml = argc > 1 ? argv[1] : "embed://default.nml";
    LOGD("Loading argument %s\n", nml);
    UI.setArguments(argc, argv);

    if (!UI.runApplication(nml)) {
        LOGE("Failed to run nidium application");
        return 0;
    }

    UI.runLoop();

    return 0;
}
