/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/

#include <jni.h>
#include <android/log.h>
#include <string>
#include "Core/Context.h"
#include "ape_netlib.h"

#include "AndroidUIInterface.h"
#include "System.h"
#include <SDL_main.h>

#define LOG_TAG "Nidroid"
#define LOGI(...)  __android_log_print(ANDROID_LOG_INFO, LOG_TAG,__VA_ARGS__)
#define LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, LOG_TAG,__VA_ARGS__)
#define LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

unsigned long _ape_seed;

namespace Nidium {
namespace Interface {
    SystemInterface *SystemInterface::_interface = new System();
    UIInterface *__NidiumUI;
}
}

// Entry point called by SDL
int main(int argc, char **argv)
{
    LOGI("Hello android");
    Nidium::Interface::AndroidUIInterface UI;
    Nidium::Interface::__NidiumUI = &UI;

    _ape_seed = time(NULL) ^ (getpid() << 16);

    const char *nml = "http://p.nf/android.nml";

    UI.setArguments(argc, argv);

    LOGI("RUN");
    if (!UI.runApplication(nml)) {
    LOGI("Run failed");
        return 0;
    }

    LOGI("starting loop");
    UI.runLoop();

    return 0;
}
