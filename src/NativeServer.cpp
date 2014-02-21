#include "NativeServer.h"
#include <native_netlib.h>

#include <NativeJS.h>
//#include <NativeSystemInterface.h>
#include <osx/NativeSystem.h>

#include <string.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

int ape_running = 1;
unsigned long _ape_seed;

NativeSystemInterface *NativeSystemInterface::_interface = new NativeSystem();

int NativeServer::Start(int argc, char **argv)
{
    _ape_seed = time(NULL) ^ (getpid() << 16);
    ape_global *net = native_netlib_init();

    printf("[Native Server] Starting.\n");

    NativeJS *js = new NativeJS(net);

    events_loop(net);
    return 1;
}
