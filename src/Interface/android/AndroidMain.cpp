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
#include <SDL.h>

unsigned long _ape_seed;

namespace Nidium {
namespace Interface {
    SystemInterface *SystemInterface::_interface = nullptr;
    UIInterface *__NidiumUI;
}
}

using namespace Nidium::Interface;

// Called before SDL_main, used to setup System class for Nidium
extern "C" void Java_com_nidium_android_Nidroid_nidiumInit(JNIEnv *env, jobject thiz, jobject nidroid)
{
    SystemInterface::_interface = new System(env, nidroid);
}


// Entry point called by SDL
int main(int argc, char **argv)
{
    AndroidUIInterface UI;
    __NidiumUI = &UI;

    _ape_seed = time(NULL) ^ (getpid() << 16);

    
    const char *userDir = SystemInterface::GetInstance()->getUserDirectory();
    char nml[2048];
    snprintf(nml, 2048, "%s/%s", userDir, "nidium/foo.nml");

    UI.setArguments(argc, argv);

    if (!UI.runApplication(nml)) {
        LOGE("Failed to run nidium application");
        return 0;
    }

    UI.runLoop();

    return 0;
}
