/*
   Copyright 2017 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/

#include <unistd.h>

#include "System.h"
#include "IOSUIInterface.h"

unsigned long _ape_seed;

namespace Nidium {
namespace Interface {
class SystemInterface;
class UIInterface;

SystemInterface *SystemInterface::_interface = new System();
UIInterface *__NidiumUI;
}
}

int main(int argc, char *argv[])
{
    Nidium::Interface::IOSUIInterface UI;
    Nidium::Interface::__NidiumUI = &UI;

    _ape_seed = time(NULL) ^ (getpid() << 16);

    const char *nml = NULL;
    nml = argc > 1 ? argv[1] : "embed://default.nml";
    UI.setArguments(argc, argv);

    if (!UI.runApplication(nml)) {
        printf("Failed to run nidium application\n");
        return 0;
    }

    UI.runLoop();

    return 0;
}
