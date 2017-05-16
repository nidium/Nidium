/*
   Copyright 2017 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <direct.h>
#include <process.h>
#include <sys/types.h>
#include <fcntl.h>
#include <io.h>

#include <Port/MSWindows.h>

#ifdef NIDIUM_ENABLE_CRASHREPORTER
#include <client/windows/handler/exception_handler.h>
#endif

#include "WinUIInterface.h"
#include "System.h"

extern "C" {
   unsigned long _ape_seed;
}

namespace Nidium {
namespace Interface {

class SystemInterface;
class UIInterface;

SystemInterface *SystemInterface::_interface = new System();
UIInterface *__NidiumUI;
}

namespace App {

//TODO: check driveletter usage
char _root[PATH_MAX]; // Using _root to store the location of nidium exec

#ifdef NIDIUM_ENABLE_CRASHREPORTER
static bool dumpCallback(const google_breakpad::MinidumpDescriptor &descriptor,
                         void *context,
                         bool succeeded)
{
    fprintf(stderr,
            "Nidium crash - Sending report - No personal information is "
            "transmited\n");
    char reporter[PATH_MAX];
    snprintf(reporter, PATH_MAX, "%s/nidium-crash-reporter",
             Nidium::App::_root);
    int ret = execl(reporter, "nidium-crash-reporter", descriptor.path(), NULL);
    printf("Crash reporter returned code %d\n", ret);
    return succeeded;
}
#endif

} // namespace App
} // namespace Nidium

// Attach output of application to parent console
// Source : https://www.tillett.info/2013/05/13/how-to-create-a-windows-program-that-works-as-both-as-a-gui-and-console-application/
static BOOL attachOutputToConsole(void) {
    HANDLE consoleHandleOut, consoleHandleError;

    if (AttachConsole(ATTACH_PARENT_PROCESS)) {
        // Redirect unbuffered STDOUT to the console
        consoleHandleOut = GetStdHandle(STD_OUTPUT_HANDLE);
        if (consoleHandleOut != INVALID_HANDLE_VALUE) {
            freopen("CONOUT$", "w", stdout);
            setvbuf(stdout, nullptr, _IONBF, 0);
        } else {
            return false;
        }

        // Redirect unbuffered STDERR to the console
        consoleHandleError = GetStdHandle(STD_ERROR_HANDLE);
        if (consoleHandleError != INVALID_HANDLE_VALUE) {
            freopen("CONOUT$", "w", stderr);
            setvbuf(stderr, nullptr, _IONBF, 0);
        } else {
            return false;
        }
        return true;
    }

    //Not a console application
    return false;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPTSTR pCmdLine, int nCmdShow)
{
    attachOutputToConsole();

    Nidium::Interface::UIWinInterface UI;

#ifdef NIDIUM_ENABLE_CRASHREPORTER
    google_breakpad::MinidumpDescriptor descriptor(UI.getCacheDirectory());
    google_breakpad::ExceptionHandler eh(descriptor, NULL, dumpCallback, NULL,
                                         true, -1);
#endif

    Nidium::Interface::__NidiumUI = &UI;
    _ape_seed = time(NULL) ^ (getpid() << 16);
    if (getcwd(Nidium::App::_root, PATH_MAX)) {
        int l                 = strlen(Nidium::App::_root);
        Nidium::App::_root[l] = '/';
        l += 1;
        strncpy(&Nidium::App::_root[l], __argv[0], PATH_MAX - l);
        _splitpath(&Nidium::App::_root[1], NULL, &Nidium::App::_root[0], NULL, NULL);
    }

    const char *nml = NULL;

    nml = __argc > 1 ? __argv[1] : "embed://default.nml";

    UI.setArguments(__argc, __argv, hInstance );
    if (!UI.runApplication(nml)) {
        return 0;
    }

    UI.runLoop();

    return 0;
}

