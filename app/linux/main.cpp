#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <libgen.h>
#include <time.h>
#include <sys/types.h>

#include <client/linux/handler/exception_handler.h>

#include "NativeX11UIInterface.h"
#include "NativeSystem.h"

NativeSystemInterface *NativeSystemInterface::_interface = new NativeSystem();
NativeUIInterface *__NativeUI;


int _nativebuild = 1002;
unsigned long _ape_seed;

char _root[PATH_MAX]; // Using _root to store the location of nidium exec

#ifdef NATIVE_ENABLE_BREAKPAD
static bool dumpCallback(const google_breakpad::MinidumpDescriptor& descriptor,
        void* context,
        bool succeeded)
{
    fprintf(stderr, "Nidium crash - Sending report - No personal information is transmited\n");
    char reporter[PATH_MAX];
    snprintf(reporter, PATH_MAX, "%s/nidium-crash-reporter", _root);
    int ret = execl(reporter, "nidium-crash-reporter", descriptor.path(), NULL);
    fprintf(stdout, "Crash reporter returned %d\n", ret);
    return succeeded;
}
#endif

int main(int argc, char **argv)
{
    NativeX11UIInterface UI;

#ifdef NATIVE_ENABLE_BREAKPAD
    google_breakpad::MinidumpDescriptor descriptor(UI.getCacheDirectory());
    google_breakpad::ExceptionHandler eh(descriptor,
            NULL,
            dumpCallback,
            NULL,
            true,
            -1);
#endif

    __NativeUI = &UI;
    _ape_seed = time(NULL) ^ (getpid() << 16);
    if (getcwd(_root, PATH_MAX)) {
        int l = strlen(_root);
        _root[l] = '/';
        l += 1;
        strncpy(&_root[l], argv[0], PATH_MAX - l);
        dirname(_root);
    }

    const char *nml = NULL;

    nml = argc > 1 ? argv[1] : "private://default.nml";

    UI.setArguments(argc, argv);

    if (!UI.runApplication(nml)) {
        return 0;
    }

    UI.runLoop();

    return 0;
}

