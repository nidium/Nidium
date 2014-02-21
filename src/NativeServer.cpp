#include <native_netlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include "NativeServer.h"
#include "NativeContext.h"
#include "NativeMacros.h"

int ape_running = 1;
unsigned long _ape_seed;

static void signal_handler(int sign)
{
    ape_running = 0;
    NLOG("[Quit] Shutting down...");
}

static int inc_rlimit(int nofile)
{
    struct rlimit rl;

    rl.rlim_cur = nofile;
    rl.rlim_max = nofile;

    return setrlimit(RLIMIT_NOFILE, &rl);
}

int NativeServer::Start(int argc, char **argv)
{
    _ape_seed = time(NULL) ^ (getpid() << 16);
    ape_global *net = native_netlib_init();

    NLOG("[Native Server] Starting.");

    inc_rlimit(64000);

    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT, &signal_handler);
    signal(SIGTERM, &signal_handler);

    NativeContext *ctx = new NativeContext(net);

    events_loop(net);

    delete ctx;

    return 1;
}
